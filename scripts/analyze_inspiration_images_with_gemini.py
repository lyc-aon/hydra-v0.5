#!/usr/bin/env python3

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


class GeminiInspirationError(RuntimeError):
    pass


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def relative_workspace_path(path: Path, root: Path) -> str:
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(root))
    except ValueError as exc:
        raise GeminiInspirationError(f"Image must be inside the workspace: {path}") from exc


def build_prompt(image_path: str) -> str:
    return (
        "You are analyzing a single local inspiration image for the Hydra V2 desktop application.\n\n"
        "Context:\n"
        "- Hydra V2 is a Qt/QML desktop orchestration tool.\n"
        "- The product is currently only at Phase 1/2.\n"
        "- Do not invent future product surfaces that do not exist.\n"
        "- This task is about visual language only.\n\n"
        "Deliver a precise breakdown with these headings:\n"
        "1. Filename\n"
        "2. Primary Mood\n"
        "3. Dominant Palette\n"
        "4. Contrast Model\n"
        "5. Typography Cues\n"
        "6. Geometry And Shape Language\n"
        "7. Material And Texture Cues\n"
        "8. UI Composition Cues\n"
        "9. Motion Implications For Software\n"
        "10. Details Worth Borrowing\n"
        "11. Details That Would Become Kitsch If Copied Too Literally\n\n"
        "Be concrete, visually grounded, and specific to the supplied image.\n\n"
        f"Image: @{image_path}\n"
    )


def parse_args() -> argparse.Namespace:
    root = repo_root()
    inspiration_dir = root / "Image Inspiration"
    parser = argparse.ArgumentParser(description="Run Gemini analysis over local inspiration images one at a time.")
    parser.add_argument(
        "images",
        nargs="*",
        type=Path,
        default=sorted(inspiration_dir.glob("*")),
        help="Workspace-local inspiration images to analyze.",
    )
    parser.add_argument(
        "--model",
        default="gemini-2.5-pro",
        help="Gemini model to use.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=root / "docs" / "validation" / "gemini-inspiration-image-analysis-2026-03-06.md",
        help="Markdown file to write with the raw analysis output.",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=240,
        help="Per-image Gemini timeout in seconds.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = repo_root()

    images = [path.resolve() for path in args.images if path.is_file()]
    missing = [str(path) for path in images if not path.exists()]
    if missing:
        raise GeminiInspirationError(f"Missing image(s): {', '.join(missing)}")
    if not images:
        raise GeminiInspirationError("No inspiration images found")

    workspace_paths = [relative_workspace_path(path, root) for path in images]
    sections: list[str] = []

    for workspace_path in workspace_paths:
        prompt = build_prompt(workspace_path)
        result = subprocess.run(
            [
                "gemini",
                "-p",
                prompt,
                "-m",
                args.model,
                "--approval-mode",
                "default",
                "--output-format",
                "text",
                "--sandbox",
                "false",
            ],
            cwd=root,
            text=True,
            capture_output=True,
            timeout=args.timeout,
        )
        if result.returncode != 0:
            raise GeminiInspirationError(
                result.stderr.strip() or result.stdout.strip() or f"Gemini failed for {workspace_path}"
            )

        output = result.stdout.strip()
        if not output:
            raise GeminiInspirationError(f"Gemini returned empty output for {workspace_path}")

        sections.append(f"## `{workspace_path}`\n\n{output}\n")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    body = (
        "# Gemini Inspiration Image Analysis\n\n"
        f"Model: `{args.model}`\n\n"
        "Images:\n"
        + "\n".join(f"- `{path}`" for path in workspace_paths)
        + "\n\n"
        + "\n".join(sections)
        + "\n"
    )
    args.output.write_text(body, encoding="utf-8")
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
