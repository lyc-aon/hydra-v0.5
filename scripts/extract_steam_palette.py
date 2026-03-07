#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
from collections import Counter, deque
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw


@dataclass(frozen=True)
class PaneBox:
    x0: int
    y0: int
    x1: int
    y1: int

    @property
    def width(self) -> int:
        return self.x1 - self.x0 + 1

    @property
    def height(self) -> int:
        return self.y1 - self.y0 + 1

    @property
    def area(self) -> int:
        return self.width * self.height

    def as_tuple(self) -> tuple[int, int, int, int]:
        return (self.x0, self.y0, self.x1, self.y1)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def hex_color(color: tuple[int, int, int]) -> str:
    return f"#{color[0]:02X}{color[1]:02X}{color[2]:02X}"


def find_panes(image: Image.Image, threshold: int = 24, min_area: int = 15000) -> list[PaneBox]:
    width, height = image.size
    pixels = image.load()
    mask = [[max(pixels[x, y]) > threshold for x in range(width)] for y in range(height)]
    seen = [[False] * width for _ in range(height)]
    panes: list[PaneBox] = []

    for y in range(height):
        for x in range(width):
            if not mask[y][x] or seen[y][x]:
                continue
            queue = deque([(x, y)])
            seen[y][x] = True
            min_x = max_x = x
            min_y = max_y = y

            while queue:
                cx, cy = queue.popleft()
                min_x = min(min_x, cx)
                max_x = max(max_x, cx)
                min_y = min(min_y, cy)
                max_y = max(max_y, cy)
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if 0 <= nx < width and 0 <= ny < height and mask[ny][nx] and not seen[ny][nx]:
                        seen[ny][nx] = True
                        queue.append((nx, ny))

            pane = PaneBox(min_x, min_y, max_x, max_y)
            if pane.area >= min_area:
                panes.append(pane)

    panes.sort(key=lambda box: (box.y0, box.x0))
    return panes


def quantized_counts(image: Image.Image, colors: int) -> Counter[tuple[int, int, int]]:
    quantized = image.quantize(colors=colors, method=Image.Quantize.MEDIANCUT).convert("RGB")
    pixels = quantized.load()
    return Counter(pixels[x, y] for y in range(quantized.height) for x in range(quantized.width))


def round_color(color: tuple[int, int, int], step: int = 4) -> tuple[int, int, int]:
    return tuple(int(round(channel / step) * step) for channel in color)


def write_swatches(path: Path, palettes: list[tuple[str, list[tuple[str, int]]]]) -> None:
    width = 1200
    row_height = 84
    header_height = 44
    image = Image.new("RGB", (width, header_height + (len(palettes) * row_height)), (8, 9, 8))
    draw = ImageDraw.Draw(image)

    draw.text((18, 12), "Steam Palette Swatches", fill=(220, 220, 210))

    for row_index, (label, colors) in enumerate(palettes):
        y = header_height + (row_index * row_height)
        draw.text((18, y + 8), label, fill=(210, 210, 198))
        for color_index, (hex_value, count) in enumerate(colors):
            x = 240 + (color_index * 140)
            rgb = tuple(int(hex_value[index:index + 2], 16) for index in (1, 3, 5))
            draw.rectangle((x, y + 6, x + 96, y + 54), fill=rgb, outline=(230, 230, 220), width=1)
            draw.text((x, y + 58), hex_value, fill=(220, 220, 210))
            draw.text((x + 82, y + 58), str(count), fill=(150, 156, 144), anchor="ra")

    image.save(path)


def main() -> int:
    root = repo_root()
    parser = argparse.ArgumentParser(description="Extract an exact palette report from the Oldschool Steam inspiration image.")
    parser.add_argument(
        "--image",
        type=Path,
        default=root / "Image Inspiration" / "Oldschool Steam.jpg",
    )
    parser.add_argument(
        "--output-md",
        type=Path,
        default=root / "docs" / "validation" / "steam-palette-extraction-2026-03-06.md",
    )
    parser.add_argument(
        "--output-json",
        type=Path,
        default=root / ".runtime" / "steam-palette-extraction-2026-03-06.json",
    )
    parser.add_argument(
        "--output-swatch",
        type=Path,
        default=root / ".runtime" / "steam-palette-swatch-2026-03-06.png",
    )
    args = parser.parse_args()

    image = Image.open(args.image).convert("RGB")
    panes = find_panes(image)

    global_palette = quantized_counts(image, 24)
    pane_palettes: list[dict[str, object]] = []
    aggregate = Counter()

    for index, pane in enumerate(panes, start=1):
        crop = image.crop(pane.as_tuple())
        counts = quantized_counts(crop, 8)
        top_colors = counts.most_common(6)
        pane_palettes.append(
            {
                "pane": index,
                "box": list(pane.as_tuple()),
                "colors": [{"hex": hex_color(color), "count": count} for color, count in top_colors],
            }
        )
        for color, count in top_colors:
            aggregate[round_color(color)] += count

    source_pixels = image.load()
    bright_pixels = [
        source_pixels[x, y]
        for y in range(image.height)
        for x in range(image.width)
        if max(source_pixels[x, y]) >= 120
    ]
    bright_palette_counter = Counter()
    if bright_pixels:
        bright_image = Image.new("RGB", (len(bright_pixels), 1))
        bright_image.putdata(bright_pixels)
        bright_palette_counter = quantized_counts(bright_image, 10)

    recurring_palette = [(hex_color(color), count) for color, count in aggregate.most_common(12)]
    bright_palette = [(hex_color(color), count) for color, count in bright_palette_counter.most_common(10)]
    global_palette_list = [(hex_color(color), count) for color, count in global_palette.most_common(18)]

    report = {
        "image": str(args.image),
        "size": list(image.size),
        "pane_boxes": [list(pane.as_tuple()) for pane in panes],
        "global_palette": [{"hex": hex_value, "count": count} for hex_value, count in global_palette_list],
        "recurring_pane_palette": [{"hex": hex_value, "count": count} for hex_value, count in recurring_palette],
        "bright_palette": [{"hex": hex_value, "count": count} for hex_value, count in bright_palette],
        "pane_palettes": pane_palettes,
    }

    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(report, indent=2), encoding="utf-8")

    palettes_for_swatches = [
        ("Global Quantized Palette", global_palette_list[:8]),
        ("Recurring Pane Palette", recurring_palette[:8]),
        ("Bright Text / Border Palette", bright_palette[:8]),
    ]
    args.output_swatch.parent.mkdir(parents=True, exist_ok=True)
    write_swatches(args.output_swatch, palettes_for_swatches)

    pane_sections = []
    for pane in pane_palettes:
        lines = [f"## Pane {pane['pane']} {tuple(pane['box'])}", ""]
        for entry in pane["colors"]:
            lines.append(f"- `{entry['hex']}` x {entry['count']}")
        pane_sections.append("\n".join(lines))

    md = (
        "# Steam Palette Extraction\n\n"
        "Last updated: 2026-03-06\n\n"
        "Primary source image: `Image Inspiration/Oldschool Steam.jpg`\n\n"
        "## Method\n\n"
        "1. Load the Steam collage directly with Pillow.\n"
        "2. Segment non-black connected components to isolate the actual Steam window panes.\n"
        "3. Quantize each pane independently with deterministic median-cut palette extraction.\n"
        "4. Merge recurring pane colors to identify the real Steam surface palette rather than the black collage background.\n"
        "5. Extract a separate bright palette for text, borders, and highlights.\n\n"
        f"Swatch image: `{args.output_swatch.relative_to(root)}`\n\n"
        "## Global quantized palette\n\n"
        + "\n".join(f"- `{hex_value}` x {count}" for hex_value, count in global_palette_list)
        + "\n\n## Recurring pane palette\n\n"
        + "\n".join(f"- `{hex_value}` x {count}" for hex_value, count in recurring_palette)
        + "\n\n## Bright text / border palette\n\n"
        + "\n".join(f"- `{hex_value}` x {count}" for hex_value, count in bright_palette)
        + "\n\n## Pane boxes\n\n"
        + "\n".join(f"- Pane {index + 1}: `{tuple(box)}`" for index, box in enumerate(report["pane_boxes"]))
        + "\n\n"
        + "\n\n".join(pane_sections)
        + "\n"
    )

    args.output_md.parent.mkdir(parents=True, exist_ok=True)
    args.output_md.write_text(md, encoding="utf-8")
    print(args.output_md)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
