# UX Assistance Refinement Pass

Last updated: 2026-03-06

## Status

- Phase: `Complete`
- Scope: add stronger contextual guidance, hover feedback, and unobtrusive documentation surfaces to the current Phase 1 / 2 shell

## Goals

1. make it clearer what each shell section is for
2. make hoverable and clickable elements feel more obviously interactive
3. keep help unobtrusive and directly anchored to the relevant section
4. provide a quick hint on hover and a deeper explanation on click
5. ensure the help text matches the real current architecture and workflow
6. verify the help surfaces through the real GUI and screenshot path

## Research Basis

- Qt Quick Controls `ToolTip` is appropriate for short, non-essential hover descriptions.
- Qt Quick Controls `Popup` is suitable for richer contextual surfaces, but the current Hydra shell uses a plain `Window`, so a custom in-scene overlay is the lower-risk implementation for this pass.
- GNOME tooltip guidance reinforces that tooltips should not carry essential information and should stay brief.

## Implementation Shape

- add one shared quick-help bubble at the app-shell level
- add one shared detailed help screen at the app-shell level
- add section-level info buttons for:
  - console overview
  - launch bus
  - target map
  - execute
  - sessions
- add short hover descriptions to the real interactive controls and cards
- strengthen hover feedback on cards and action buttons without inventing new workflow
- replace ambiguous top-level count shorthand where possible, and explain any remaining shorthand directly in the help copy
- add deterministic startup hooks so screenshot capture can render quick-help and detailed-help states
- extend the live GUI validator to click an info button, verify quick help, open detailed help, and dismiss it

## Planned work

- wire a centralized help-topic catalog into the app shell
- implement reusable help UI primitives in QML
- apply hover and tooltip coverage to existing interactive elements
- capture baseline, live, quick-help, and detailed-help visual states
- run targeted Gemini review on the new help and hover surfaces
- update handoff, readme, planning, and validation docs after the pass

## Validation gate

- `cmake --build build/debug --target hydra -j2`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/ux-assistance-refine`
- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`
- targeted Gemini review of the help/hover surfaces


## Outcome

- shared quick-help and detailed-help surfaces are now implemented
- hover/help coverage was validated through the real GUI
- offscreen capture remains the deterministic shell-state path
- actual X11 window capture is now the trusted hover/help evidence path
- Gemini review completed successfully against the real guidance captures
