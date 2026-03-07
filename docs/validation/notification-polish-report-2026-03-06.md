# Notification Polish Report

Date: 2026-03-06
Status: Complete

## Scope

This pass corrected three concrete Phase 1 / 2 shell issues:

1. the launch sweep in `ExecutePanel.qml` was rendering outside the button boundary
2. the empty-worktree warning stayed on screen indefinitely
3. the status banner appeared and disappeared as a hard visibility jump instead of a more intentional notification

The pass also rechecked the touched files for low-risk maintainability improvements.

## Research note

Current guidance used for the pass:

- Qt Quick clipping guidance supports clipping at the surface that actually owns the visual boundary
- Qt Quick animation guidance still favors simple property animation on the elements that communicate state
- shared notification state should be explicit in the view-model rather than inferred in QML from string contents

## Implementation

Changed:

- `qml/Hydra/components/ExecutePanel.qml`
  - the launch sweep is now clipped to the button surface
- `src/ui/viewmodels/app_state.hpp`
- `src/ui/viewmodels/app_state.cpp`
  - action feedback now uses explicit transient notification behavior
  - warning tone is now explicit through `statusIsWarning`
  - failed worktree creation no longer triggers an unnecessary workspace reload
  - launch and terminate only reload the full app state when the operation actually succeeds
- `qml/Hydra/components/SessionBoard.qml`
  - status banner now has tone-aware styling and animated enter/exit behavior
- `scripts/validate_wayland_ui.py`
  - now proves the empty-worktree warning appears and clears automatically
- `scripts/capture_guidance_states_x11.py`
  - now captures the visible worktree-warning state for visual inspection

## Validation

Passed:

1. `cmake --build build/debug --target hydra -j2`
2. `cmake --build build/debug-make -j2`
3. `./build/debug-make/hydra_core_smoke`
4. `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-notification-pass.png --quit-after-screenshot`
5. `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/notification-pass`
6. `python3 scripts/validate_wayland_ui.py`
7. `python3 scripts/validate_divider_drag_x11.py`
8. `python3 scripts/capture_guidance_states_x11.py --output-dir .runtime/ui-captures/guidance-notification-pass`
9. `python3 scripts/review_ui_with_gemini.py .runtime/ui-captures/guidance-notification-pass/contact-sheet.png .runtime/ui-captures/notification-pass/contact-sheet.png --output docs/validation/gemini-notification-polish-review-2026-03-06.md`

Artifacts:

- `.runtime/ui-captures/notification-pass/contact-sheet.png`
- `.runtime/ui-captures/guidance-notification-pass/contact-sheet.png`
- `docs/validation/gemini-notification-polish-review-2026-03-06.md`

## Current size / hotspot snapshot

Largest implementation files after this pass:

- `src/infrastructure/git/git_repo_workspace.cpp` — 486 lines
- `qml/Hydra/App.qml` — 424 lines
- `src/ui/viewmodels/app_state.cpp` — 378 lines
- `qml/Hydra/components/TargetMapPanel.qml` — 332 lines
- `qml/Hydra/components/SessionsSurface.qml` — 281 lines

Readout:

- none of the current hotspots are over the stated soft ceiling
- `git_repo_workspace.cpp` is long because it still owns repository-root detection, `.hydra` provisioning, exclude wiring, worktree listing, and worktree creation
- `App.qml` is long because it is the root interaction host for shell sizing, rail motion, hover help, quick help, and detailed help
- `app_state.cpp` is long because it still owns selection state, repo/worktree/session reload orchestration, and transient notification state
- the QML panel files remain long mainly because they are layout assemblers, not because of obvious duplication regressions

Current recommendation:

- keep the current split for now
- only modularize `App.qml` or `app_state.cpp` further if Phase 3 adds enough new interaction state to push them meaningfully past the current range
- split `git_repo_workspace.cpp` only when provider-era repo features add new responsibilities to it
