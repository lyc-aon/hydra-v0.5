# Visual Review Pipeline

Last updated: 2026-03-06

## Purpose

This note records the current end-to-end visual validation path for Hydra V2.

The earlier AT-SPI-only path was sufficient for semantic interaction testing, but not for judging layout, density, spacing, or visual polish. Hydra now has a deterministic screenshot export path inside the app itself, and those screenshots can be reviewed headlessly by the local Gemini CLI with its existing OAuth-backed login.

## What now works

The current repeatable review loop has four parts:

1. `scripts/capture_ui_screenshots.py`
   - creates an isolated Git fixture and isolated `XDG_DATA_HOME`
   - boots Hydra against that fixture
   - captures deterministic offscreen PNGs from the real QML window via `--screenshot`
   - writes them into the repo under `.runtime/ui-captures/current/`
   - also assembles a contact sheet automatically when `montage` is available

2. `scripts/review_ui_with_gemini.py`
   - sends workspace-local screenshots to `gemini-2.5-pro`
   - uses headless mode with `@path` attachment syntax
   - writes the critique to `docs/validation/gemini-ui-review-2026-03-06.md`

3. `scripts/validate_wayland_ui.py`
   - drives the live app over AT-SPI on the real GNOME Wayland session
   - validates actual repo/worktree/session behavior
   - now also validates a compact `960x700` desktop window for critical control visibility
   - catches functional regressions that screenshots alone would miss


4. `scripts/capture_guidance_states_x11.py`
   - launches Hydra under X11 on the live desktop session
   - captures actual hover and help states that offscreen startup capture does not represent reliably
   - writes those images into the repo under `.runtime/ui-captures/guidance-x11/`

## Why this replaced the earlier screenshot attempts

The earlier compositor-level screenshot paths were not reliable here:

- `gnome-screenshot` / GNOME Shell DBus screenshot access was denied in the tested automation context
- `Xvfb + xdotool + scrot` did not yield a dependable Hydra window path
- `ydotool` was unstable and removed

The app-owned capture path avoids that entire class of failure.

Hydra now exports screenshots from its own `QQuickWindow` in [`src/main.cpp`](../../src/main.cpp) using:

- `--screenshot <path>`
- `--window-width <px>`
- `--window-height <px>`
- `--quit-after-screenshot`

The capture is taken from a rendered frame and backed by a watchdog timer, which makes it usable for deterministic review output rather than only manual experimentation.

## Current verified procedure

### 1. Build the desktop app

```bash
cmake --preset debug
cmake --build build/debug --target hydra -j2
```

### 2. Capture representative screenshots

```bash
python3 scripts/capture_ui_screenshots.py
```

Current outputs:

- `.runtime/ui-captures/current/phase2-baseline-wide.png`
- `.runtime/ui-captures/current/phase2-baseline-resized-wide.png`
- `.runtime/ui-captures/current/phase2-baseline-collapsed-wide.png`
- `.runtime/ui-captures/current/phase2-baseline-tight.png`
- `.runtime/ui-captures/current/phase2-baseline-narrow.png`
- `.runtime/ui-captures/current/phase2-baseline-compact.png`
- `.runtime/ui-captures/current/phase2-live-wide.png`
- `.runtime/ui-captures/current/phase2-live-resized-wide.png`
- `.runtime/ui-captures/current/phase2-live-collapsed-wide.png`
- `.runtime/ui-captures/current/phase2-live-tight.png`
- `.runtime/ui-captures/current/phase2-live-narrow.png`
- `.runtime/ui-captures/current/phase2-live-compact.png`
- `.runtime/ui-captures/current/contact-sheet.png`
- `.runtime/ui-captures/current/manifest.json`

### 3. Capture actual hover/help states when the pass touches contextual guidance

```bash
python3 scripts/capture_guidance_states_x11.py
```

Current outputs:

- `.runtime/ui-captures/guidance-x11/launch-hover.png`
- `.runtime/ui-captures/guidance-x11/target-help-hover.png`
- `.runtime/ui-captures/guidance-x11/target-help-quick.png`
- `.runtime/ui-captures/guidance-x11/target-help-detail.png`
- `.runtime/ui-captures/guidance-x11/contact-sheet.png`
- `.runtime/ui-captures/guidance-x11/manifest.json`

Important detail:

- offscreen startup capture is trustworthy for deterministic shell-state frames
- hover/help surfaces should be reviewed from real X11 window captures instead

### 4. Run the Gemini visual critique

```bash
python3 scripts/review_ui_with_gemini.py
```

Current output:

- `docs/validation/gemini-ui-review-2026-03-06.md`

Important detail:

- the screenshots must live inside the workspace for clean headless review
- headless Gemini review works when the prompt includes `@relative/path/to/image.png`
- images outside the workspace trigger an approval prompt in interactive mode, which is not appropriate for unattended regression review

### 5. Run the functional UI validator

```bash
python3 scripts/validate_wayland_ui.py
```

This confirms the current visual sample still corresponds to real working Phase 1 / 1.5 / 2 behavior.

## Current state of the pipeline

Verified on 2026-03-06:

- offscreen screenshot export works and produces valid PNGs at `1500x920`, `1180x760`, `1040x720`, and `960x700`
- the capture fixture also exports collapsed-wide board states through `--start-sidebar-collapsed`
- the capture fixture now also exports widened-rail states through `--start-sidebar-width`
- workspace-local screenshots are accepted by `gemini-2.5-pro` in headless mode
- Gemini review output was written successfully for the shell rewrite checkpoints, including:
  - `docs/validation/gemini-ui-review-phase-d-2026-03-06.md`
  - `docs/validation/gemini-ui-review-phase-e-2026-03-06.md`
- Gemini review output was also written successfully for the final Phase 1 / 2 refinement pass:
  - `docs/validation/gemini-ui-review-phase-1-2-refine-2026-03-06.md`
- Gemini review output was also written successfully for the divider/resize refinement pass:
  - `docs/validation/gemini-ui-review-divider-refine-2026-03-06.md`
- Gemini per-image inspiration analysis was also written successfully for the Hermes / Steam visual-direction pass:
  - `docs/validation/gemini-inspiration-image-analysis-2026-03-06.md`
- Gemini post-pass direction review was also written successfully for the Hermes / Steam shell pass:
  - `docs/validation/gemini-hermes-steam-pass-review-2026-03-06.md`
- the Wayland AT-SPI validator still passes after the screenshot tooling refactor, including the compact-window visibility checks
- divider drag and divider-click pointer validation now runs separately through `scripts/validate_divider_drag_x11.py`
- the divider validator now waits for geometry transitions after drag/toggle input instead of sampling immediately, which makes it reliable against the current animated rail timing
- the Wayland and X11 validators now scope AT-SPI lookups to the launched Hydra PID so they do not attach to unrelated Hydra windows already open on the desktop
- actual hover/help evidence is now captured through `scripts/capture_guidance_states_x11.py` because the offscreen startup-help path is not treated as a trustworthy visual source
- Gemini guidance review was also written successfully from the real GUI captures:
  - `docs/validation/gemini-ux-assistance-review-2026-03-06.md`

This means Hydra now has:

- semantic UI operation testing
- deterministic visual capture
- external image-model critique
- a way to run those together between UI iterations

## Current limitations

- This path captures static frames, not motion or timing quality.
- There is still no video or animation regression harness.
- AT-SPI text entry into the branch field is still not a reliable semantic automation path on this setup.
- Gemini critique is useful as a design reviewer, but not a substitute for local engineering judgment or architecture constraints.
- The current capture fixture is intentionally representative, not exhaustive; it shows Phase 2 shell states, not every possible runtime combination.

Motion-specific local engineering note:

- when related shell geometry is animated, prefer a single animated source-of-truth property and derive dependent positions/opacities from it
- avoid separate `Behavior` blocks on interdependent geometry like rail width and divider position, because they can visibly drift
- avoid letting responsive breakpoints flip repeatedly during the animation itself; use stable target layout widths where possible

## Recommended use between rewrite iterations

For each major shell/UI pass:

1. rebuild `hydra`
2. run `python3 scripts/capture_ui_screenshots.py`
3. inspect the resulting contact sheet or individual PNGs
4. when the pass changes hover/help, run `python3 scripts/capture_guidance_states_x11.py`
5. run `python3 scripts/review_ui_with_gemini.py` or a targeted Gemini review over the relevant images
6. run `python3 scripts/validate_wayland_ui.py`
7. run `python3 scripts/validate_divider_drag_x11.py` when the change touches pointer-only divider behavior
8. only continue if both the visual and functional gates still hold

## Related docs

- `docs/validation/ui-observation-tooling-2026-03-06.md`
- `docs/validation/wayland-ui-operator-guide-2026-03-06.md`
- `docs/validation/phase-1-1.5-2-validation-report-2026-03-06.md`
- `docs/validation/gemini-ui-review-2026-03-06.md`
- `docs/validation/gemini-ui-review-phase-d-2026-03-06.md`
- `docs/validation/gemini-ui-review-phase-e-2026-03-06.md`
- `docs/validation/gemini-ui-review-phase-1-2-refine-2026-03-06.md`
- `docs/validation/gemini-ui-review-divider-refine-2026-03-06.md`
- `docs/validation/gemini-inspiration-image-analysis-2026-03-06.md`
- `docs/validation/gemini-hermes-steam-pass-review-2026-03-06.md`
- `docs/validation/inspiration-palette-extraction-2026-03-06.md`
