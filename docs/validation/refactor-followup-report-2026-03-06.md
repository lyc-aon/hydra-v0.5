# Refactor Follow-up Report

Date: 2026-03-06
Status: Complete

## Scope

This pass continued the Phase 1 / 2 maintainability and shell-consistency audit.

Immediate problem addressed:

- the shell still mixed the product-themed help surfaces with the default Qt attached `ToolTip` surface
- the info-dot hover text was the most obvious mismatch, but the same inconsistency existed on multiple controls

## Research notes

Current-source guidance used for the pass:

- Qt Quick Controls `ToolTip` documentation confirms that the attached tooltip is a shared control-level surface, which is a poor fit when the product needs a bespoke themed hover-help layer
- Qt Quick best-practices and performance docs still favor explicit component boundaries, simple bindings, and avoiding unnecessary special-case UI paths
- GNOME tooltip guidance remains consistent with using short, unobtrusive hover hints instead of redundant or aggressive hover UI
- C++ Core Guidelines still support keeping the shell/data boundaries explicit and avoiding convenience fallback behavior that hides real dependencies

Brave Search note:

- the local Full Plate Harness Brave helper exists in sibling tooling, but `BRAVE_API_KEY` was not configured in the current environment during this pass
- web research therefore used current official documentation sources and local code inspection instead

## Implementation

Added:

- `qml/Hydra/components/HoverHintBubble.qml`

Changed:

- `qml/Hydra/App.qml`
  - now hosts one delayed themed hover-hint surface for the shell
- `qml/Hydra/components/InfoDotButton.qml`
  - no longer uses native attached `ToolTip`
- `qml/Hydra/components/DividerHandle.qml`
- `qml/Hydra/components/StatusChip.qml`
- `qml/Hydra/components/ConsoleHeader.qml`
- `qml/Hydra/components/ExecutePanel.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionsSurface.qml`
- `qml/Hydra/components/SessionCard.qml`
- `qml/Hydra/components/TargetMapPanel.qml`
- `qml/Hydra/components/RepoCard.qml`
- `qml/Hydra/components/WorktreeCard.qml`
- `scripts/capture_guidance_states_x11.py`
- `.gitignore`

Result:

- there are no remaining attached `ToolTip` usages under `qml/Hydra/`
- hover guidance now uses one consistent visual language across the shell
- the hover hint is delayed rather than appearing immediately
- the repo now has a local git baseline with generated Python cache artifacts ignored

## Validation

Passed:

1. `cmake --build build/debug --target hydra -j2`
2. `cmake --build build/debug-make -j2`
3. `./build/debug-make/hydra_core_smoke`
4. `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-followup-pass-2.png --quit-after-screenshot`
5. `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/refactor-followup-2`
6. `python3 scripts/validate_wayland_ui.py`
7. `python3 scripts/validate_divider_drag_x11.py`
8. `python3 scripts/capture_guidance_states_x11.py --output-dir .runtime/ui-captures/guidance-followup-2`
9. `python3 scripts/review_ui_with_gemini.py .runtime/ui-captures/guidance-followup-2/contact-sheet.png .runtime/ui-captures/refactor-followup-2/contact-sheet.png --output docs/validation/gemini-refactor-followup-review-2026-03-06.md`

Artifacts:

- `.runtime/ui-captures/refactor-followup-2/contact-sheet.png`
- `.runtime/ui-captures/guidance-followup-2/contact-sheet.png`
- `docs/validation/gemini-refactor-followup-review-2026-03-06.md`

## Current readout

This pass did not change product scope.

It improved the Phase 1 / 2 baseline by:

- removing the last mixed-style hover path from the shell
- keeping hover/help behavior local, explicit, and modular
- preserving the existing real-GUI validation model

Remaining refactor targets stay the same:

- keep `AppState` from accumulating unrelated Phase 3 concerns
- split `git_repo_workspace.cpp` only if future scope expands it materially
- keep validation responsibilities split by interaction type instead of forcing one harness to do everything
