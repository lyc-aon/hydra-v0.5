# Deep Visual Pass Report

Last updated: 2026-03-06

## Build and runtime gates

Passed:

- `cmake --build build/debug --target hydra -j2`
- `./build/debug/hydra_core_smoke`
- `timeout 6 ./build/debug/hydra -platform offscreen`

## Screenshot capture

Passed:

- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/deep-visual-pass`

Artifacts:

- `.runtime/ui-captures/deep-visual-pass/manifest.json`
- `.runtime/ui-captures/deep-visual-pass/contact-sheet.png`

Capture set still covers:

- baseline wide
- baseline resized-wide
- baseline collapsed-wide
- live wide
- live resized-wide
- live collapsed-wide
- baseline/live tight
- baseline/live narrow
- baseline/live compact

## Live GUI validation

Passed:

- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`

Notable verified behaviors:

- repo selection
- `.hydra/` bootstrap
- `.git/info/exclude` wiring
- worktree selection
- worktree-bound launch persistence
- live `tmux` session creation
- restart reload
- in-app end session
- compact-window visibility
- divider drag resize
- collapse/restore behavior
- refresh success message becomes visible and then clears automatically

## Gemini review artifacts

- `docs/validation/gemini-phase1-2-delta-review-2026-03-06.md`
- `docs/validation/gemini-deep-visual-pass-review-2026-03-06.md`

## Validation boundary

Still true:

- AT-SPI is the reliable semantic GUI driver on the local GNOME Wayland session
- divider drag still needs the Xwayland pointer harness because GNOME Wayland blocks synthetic pointer input for that interaction
- text entry into the worktree branch field is still not a reliable semantic automation path through AT-SPI on this setup
