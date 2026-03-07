# UX Assistance Refinement Report

Last updated: 2026-03-06

## Result

The Phase 1 / 2 guidance pass is complete.

Hydra now has:

- section-level info buttons for the current shell areas
- brief hover guidance on core interactive controls
- a quick-help bubble anchored to the clicked section
- a detailed in-app help panel for the same topic
- stronger hover states on repo/worktree cards and the main shell actions
- real GUI capture coverage for the new hover/help surfaces

## Implemented

Code changes:

- centralized help content catalog:
  - `qml/Hydra/HelpCatalog.js`
- reusable section help button:
  - `qml/Hydra/components/InfoDotButton.qml`
- shared help surfaces and startup hooks:
  - `qml/Hydra/App.qml`
  - `src/main.cpp`
- rail wiring and hover/help additions:
  - `qml/Hydra/components/LaunchSidebar.qml`
- board wiring and refresh hover guidance:
  - `qml/Hydra/components/SessionBoard.qml`
- stronger hover states and per-item hover help:
  - `qml/Hydra/components/RepoCard.qml`
  - `qml/Hydra/components/WorktreeCard.qml`
  - `qml/Hydra/components/SessionCard.qml`
- module/resource wiring:
  - `CMakeLists.txt`
  - `qml/Hydra/components/qmldir`

Validation tooling additions:

- actual-X11 help/hover capture path:
  - `scripts/capture_guidance_states_x11.py`
- Wayland validator extended to verify:
  - section info button geometry
  - quick-help open path
  - detailed-help open path
  - detailed-help close path
- standard shell capture path kept for deterministic baseline/live screenshots:
  - `scripts/capture_ui_screenshots.py`

## Validation

Passed:

- `cmake --build build/debug --target hydra -j2`
- `./build/debug/hydra_core_smoke`
- `timeout 6 ./build/debug/hydra -platform offscreen`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/ux-assistance-refine`
- `python3 scripts/capture_guidance_states_x11.py --output-dir .runtime/ui-captures/guidance-x11`
- `python3 scripts/validate_wayland_ui.py`
- earlier divider regression gate for this pass remained valid because the post-help changes did not touch divider logic; a later rerun was interrupted manually while the script was still in progress

Artifacts:

- deterministic shell states:
  - `.runtime/ui-captures/ux-assistance-refine/contact-sheet.png`
- actual hover/help states:
  - `.runtime/ui-captures/guidance-x11/contact-sheet.png`
- Gemini guidance review:
  - `docs/validation/gemini-ux-assistance-review-2026-03-06.md`

## Findings

What worked well:

- the small info buttons are discoverable without becoming visually noisy
- the quick-help bubble is well placed and easy to dismiss
- the detailed-help panel is now readable enough and visually coherent with the shell
- action-oriented hover text works best on direct controls like launch and refresh

What needed correction during the pass:

- the first offscreen-only help-state captures were not trustworthy
- real GUI X11 window capture was added as the correct evidence path for hover/help surfaces
- the detailed-help panel needed stronger contrast and structured content blocks before it read cleanly in screenshots

## Remaining limits

- AT-SPI text entry into the branch field is still not reliable on this GNOME Wayland setup
- offscreen startup capture remains the correct path for deterministic shell-state frames, but not the trusted path for hover/help presentation
- the guidance layer only documents the current Phase 1 / 2 shell; later provider/terminal surfaces will need their own help topics
