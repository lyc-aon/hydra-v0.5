# Notification Polish And Audit Pass

Date: 2026-03-06
Status: Complete

## Issues being addressed

1. The launch sweep in `ExecutePanel.qml` visually escapes the launch button instead of staying clipped to the button surface.
2. Worktree-creation validation errors currently use the shared status channel in a persistent way and do not auto-clear.
3. The transient status banner should feel more intentional when it appears and disappears.
4. The pass should also recheck nearby code for low-risk refactor or size-reduction opportunities without reducing capability or visual quality.

## Research anchors

- Qt Quick clipping should be used intentionally at the component that owns the visual boundary.
- Qt Quick animation guidance still favors simple property animation on the specific properties that communicate state change.
- Shared notification state should be explicit rather than inferred from string contents.

## Planned implementation

1. clip the launch sweep to the button boundary
2. make action feedback transient by default where it is acting as a notification rather than a persistent state surface
3. add an explicit status tone to app state so QML does not have to guess how to style a message
4. animate the status banner entrance and exit more clearly
5. look for any nearby duplication or avoidable size growth in the touched files

## Validation

1. `cmake --build build/debug --target hydra -j2`
2. `cmake --build build/debug-make -j2`
3. `./build/debug-make/hydra_core_smoke`
4. `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-notification-pass.png --quit-after-screenshot`
5. `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/notification-pass`
6. `python3 scripts/validate_wayland_ui.py`
7. `python3 scripts/validate_divider_drag_x11.py`
8. `python3 scripts/capture_guidance_states_x11.py --output-dir .runtime/ui-captures/guidance-notification-pass`

## Outcome

Completed:

- the launch sweep is clipped to the launch button instead of painting outside it
- worktree-creation validation errors now clear automatically
- the status banner now animates in and out instead of jumping on visibility
- notification tone is explicit in app state instead of being guessed from message text
- the semantic validator now proves the empty-worktree warning appears and clears
- the real-window capture flow now includes the visible warning state

Low-risk refactor wins:

- failed worktree creation no longer forces an unnecessary workspace reload
- failed launch and failed terminate no longer force an unnecessary full reload
- the touched files stayed within the current size targets
