# Steam Palette Correction Report

Last updated: 2026-03-06

## Objective

Correct the default Hydra shell palette so it aligns with `Image Inspiration/Oldschool Steam.jpg` specifically, rather than the broader mixed Hermes / Steam / NGE reference pack.

## Tooling used

No new downloads were needed.

Existing local tooling was sufficient and preferable:

- `ImageMagick 7.1.2-3`
- `Pillow 12.1.1`
- built-in Hydra screenshot export
- existing Wayland and X11 GUI validators

Reason for keeping the existing stack:

- `ImageMagick` provides deterministic histogram and quantization output
- `Pillow` makes it easy to segment the collage into real Steam panes and quantify recurring colors programmatically
- both are well-known, mature tools already installed on the machine

## Exact extraction method

Implemented in:

- `scripts/extract_steam_palette.py`

Method:

1. Load `Image Inspiration/Oldschool Steam.jpg` directly.
2. Segment the collage into non-black connected components so the real Steam window panes are isolated from the black background.
3. Quantize each pane independently with deterministic median-cut extraction.
4. Merge recurring pane colors to identify the actual Steam surface palette.
5. Extract a separate bright palette for text, borders, and highlights.

Artifacts:

- `docs/validation/steam-palette-extraction-2026-03-06.md`
- `.runtime/steam-palette-extraction-2026-03-06.json`
- `.runtime/steam-palette-swatch-2026-03-06.png`

## Sampled Steam anchors

Primary recurring pane colors:

- `#4C5844`
- `#404838`
- `#505848`
- `#505844`
- `#444C3C`
- `#4C5440`
- `#586050`
- `#707868`

Bright text / border colors:

- `#9EA28C`
- `#818A78`
- `#939277`
- `#D2D3C0`

## Code changes

Primary token retune:

- `qml/Hydra/styles/HydraTheme.qml`

Supporting reusable extraction path:

- `scripts/extract_steam_palette.py`

## Validation

Passed:

- `python3 scripts/extract_steam_palette.py`
- `cmake --build build/debug --target hydra -j2`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/steam-palette-correction`
- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`

Primary visual artifact:

- `.runtime/ui-captures/steam-palette-correction/contact-sheet.png`

## Comparison note

The Steam-specific Gemini comparison is preserved in:

- `docs/validation/gemini-steam-palette-correction-review-2026-03-06.md`

Important caveat:

- the model critique was partially useful on material treatment, but it drifted away from the actual image data by reinterpreting Steam as charcoal-plus-blue
- for this correction pass, the measured palette extracted from the reference image itself is treated as the source of truth over the model's color interpretation
