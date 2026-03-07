# UI Observation Tooling

Last updated: 2026-03-06

## Purpose

This note records which local tools are actually usable for observing and driving the Hydra desktop UI from automation, and which paths are still incomplete.

For the detailed operator workflow, see `docs/validation/wayland-ui-operator-guide-2026-03-06.md`.
For the validated results against the roadmap, see `docs/validation/phase-1-1.5-2-validation-report-2026-03-06.md`.
For the current screenshot + Gemini review path, see `docs/validation/visual-review-pipeline-2026-03-06.md`.

## Current finding

The most reliable local UI-testing path on this machine is now split into two complementary lanes:

1. direct `pyatspi` on the real GNOME Wayland session for semantic interaction and feature validation
2. app-owned screenshot export plus headless Gemini review for visual inspection
3. Hydra under `QT_QPA_PLATFORM=xcb` plus `xdotool` for divider pointer interactions that cannot be semantically driven on GNOME Wayland

The semantic lane uses:

- `python3-pyatspi`
- `accerciser`

The visual lane uses:

- Hydra's built-in `--screenshot` export path
- `python3 scripts/capture_ui_screenshots.py`
- `python3 scripts/review_ui_with_gemini.py`
- `imagemagick` for optional contact sheets

The pointer lane uses:

- `python3 scripts/validate_divider_drag_x11.py`
- `xdotool`
- `import` from ImageMagick for stage captures

Both paths now work with Hydra.

The virtual-X path is not yet reliable for Hydra:

- `xvfb-run`
- `xdotool`
- `scrot`
- `import` from ImageMagick

These tools are installed, but Hydra did not create a visible X11 window inside the tested `Xvfb` session. That path needs separate xcb/platform investigation before it can be treated as a working harness.
This does not affect the real-session Xwayland pointer harness used by `scripts/validate_divider_drag_x11.py`.

## Packages installed locally

Installed in this session:

- `python3-pyatspi`
- `python3-dogtail`
- `accerciser`
- `scrot`
- `imagemagick`
- `gnome-screenshot`

Already present before this session:

- `xdotool`
- `xvfb-run`
- `Xvfb`

## Why the AT-SPI path is the current recommendation

This machine is running GNOME on Wayland, where classic X11 automation is not the primary control surface.

Direct `pyatspi` works against the real desktop session and can:

- inspect the Hydra accessibility tree
- find named controls
- invoke button actions without screen-coordinate clicking
- support deterministic feature checks against a temporary `XDG_DATA_HOME`

Hydra now exposes accessible names for the current main controls:

- `Repository Hydra V2`
- `Worktree branch name`
- `Create worktree`
- `Worktree main`
- `Launch tmux shell`
- `Refresh Hydra state`
- `End session Hydra V2 shell`
- `Toggle navigation rail`

## Verified local automation result

Verified on 2026-03-06:

1. launched Hydra on the real Wayland session with `QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1`
2. pointed Hydra at a temporary `XDG_DATA_HOME`
3. used `pyatspi` to invoke `Launch tmux shell`
4. confirmed a new SQLite session row in the temporary Hydra database
5. used `pyatspi` to invoke `End session Hydra V2 shell`
6. confirmed the persisted row moved to `exited`

This gives Hydra a real, scriptable UI-validation path before embedded-terminal work exists.

Hydra now also has a real visual-review path:

1. `scripts/capture_ui_screenshots.py` builds a temporary fixture and exports deterministic PNGs from the app window itself
2. screenshots land inside the repo under `.runtime/ui-captures/current/`, including narrow and compact desktop states plus a generated contact sheet
3. `scripts/review_ui_with_gemini.py` sends those workspace-local PNGs to `gemini-2.5-pro` in headless mode using `@path`
4. the resulting review is written to a validation markdown file under `docs/validation/`

The Wayland feature validator now also checks a compact `960x700` desktop launch so responsive regressions can be caught semantically as well as visually.
Divider click/drag validation is now intentionally separate because GNOME Wayland blocks the synthetic pointer stream needed for that pointer-only interaction.

## Current limitations

- The AT-SPI path depends on the real desktop session and a running session bus.
- The branch field is visible and measurable, but text entry is still not a validated semantic automation path on this setup.
- Hydra still needs broader accessibility labeling if we want deep semantic coverage across every future surface.
- `dogtail` is installed, but the input path we attempted hit GNOME access-denied behavior and is not the recommended driver today.
- `Xvfb` plus `xdotool` is not yet a working Hydra harness and should not be treated as such in docs or planning.
- screenshot capture through GNOME Shell was denied in the tested non-interactive automation context, which is why Hydra now captures its own window instead
- `ydotool` was attempted as a fallback keyboard injector, crashed on `uinput`, and was removed

## Recommended next use

Short term:

- use direct `pyatspi` for launch-rail and session-board feature checks
- use the built-in screenshot capture and Gemini review loop for visual iteration checks
- use `scripts/validate_divider_drag_x11.py` when testing divider click/drag behavior
- keep tests isolated with a temporary `XDG_DATA_HOME`

Later:

- investigate the X11/xcb path if a fully headless framebuffer harness is still desired
- expand accessible names and roles as more controls are added
