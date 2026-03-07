# Steam Palette Correction Pass

Last updated: 2026-03-06

## Status

- Phase: `Complete`
- Scope: correct the default shell palette so it matches the original green Steam reference more exactly

## Problem statement

The current shell moved closer to the Hermes / Steam / NGE reference pack overall, but the default token set still drifted away from the specific olive-green Steam palette the product should use by default.

The corrective goal for this pass is narrower than the previous visual work:

1. treat `Image Inspiration/Oldschool Steam.jpg` as the primary palette authority for the default shell
2. use deterministic image sampling instead of broad aesthetic blending
3. document the exact extraction method and sampled values
4. retune only the relevant shell tokens and dependent component surfaces
5. rebuild, recapture, and revalidate the GUI after the retune

## Planned method

- audit available local image-analysis tools first
- extract exact dominant and supporting colors from `Image Inspiration/Oldschool Steam.jpg`
- prefer deterministic tools already present on the machine over adding new dependencies
- only install an additional tool if the current stack cannot produce exact, inspectable palette output
- map sampled Steam colors to explicit Hydra roles: shell background, panel surfaces, borders, primary text, muted text, primary highlight, and warning/alert separation

## Completed results

- kept the existing local image stack because it was already sufficient and inspectable:
  - `ImageMagick 7.1.2-3`
  - `Pillow 12.1.1`
- added reusable exact-extraction tooling in `scripts/extract_steam_palette.py`
- extracted Steam pane colors from `Image Inspiration/Oldschool Steam.jpg` specifically
- generated:
  - `docs/validation/steam-palette-extraction-2026-03-06.md`
  - `.runtime/steam-palette-extraction-2026-03-06.json`
  - `.runtime/steam-palette-swatch-2026-03-06.png`
- retuned `qml/Hydra/styles/HydraTheme.qml` from those measured Steam values
- rebuilt, recaptured screenshots, and reran both GUI validators successfully

## Key sampled Steam anchors

- pane body olive: `#4C5844`
- dark olive frame: `#404838`
- raised olive: `#505848`
- muted line olive: `#586050`
- light border text: `#9EA28C`
- bright label text: `#D2D3C0`

## Validation gate

- `cmake --build build/debug --target hydra -j2`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/steam-palette-correction`
- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`
