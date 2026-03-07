# Maintainability Audit Pass

Date: 2026-03-06
Status: Complete

## Why this pass exists

Hydra's Phase 1 and Phase 2 product behavior is real and validated, but the codebase now has clear maintainability debt from the rapid shell iteration passes.

Primary problems observed before refactoring:

- `qml/Hydra/App.qml` is over 1000 lines.
- `qml/Hydra/components/LaunchSidebar.qml` is over 900 lines.
- `qml/Hydra/components/SessionBoard.qml` is over 500 lines.
- QML depends on `QQmlContext`-injected globals such as `appState` and startup flags.
- Small visual primitives such as chips, framed panels, and help surfaces are duplicated inline.
- `src/infrastructure/git/git_repo_workspace.cpp` mixes several separate responsibilities in one file.
- `src/main.cpp` mixes app bootstrap and screenshot/test harness boot options.

## Audit goals

1. Keep current product behavior intact.
2. Reduce hidden coupling and make QML dependencies explicit.
3. Break oversized files into reusable, named components.
4. Remove fallback behavior that only exists to mask programmer errors.
5. Keep the current shell visuals and validation tooling working.
6. Improve code readability before Phase 3 adds more product complexity.

## Research anchors

- Qt warns against injecting values through context properties when regular properties or singletons can be used.
- Qt recommends required properties for explicit data dependencies in QML delegates and components.
- Qt Quick performance guidance recommends simple delegates, minimizing bindings, and careful use of clipping.
- C++ Core Guidelines favor explicit interfaces, smaller named functions, and abstractions that reflect the actual domain rather than speculative generalization.

## Planned refactor scope

### QML shell structure

- replace context-property dependency with explicit root properties and `setInitialProperties()`
- pass `appState` explicitly into shell components
- extract reusable components from `App.qml`
- extract reusable components and major panel slices from `LaunchSidebar.qml`
- extract reusable components and major surface slices from `SessionBoard.qml`

### C++ bootstrap and infrastructure

- separate runtime startup options from `main.cpp`
- break `git_repo_workspace.cpp` into smaller named helper functions
- remove generic fallback help-topic behavior that should not appear in normal usage

### Documentation

- add a code-relationship note for the shell component tree
- update handoff and implementation docs after the refactor and rebuild

## Validation plan

After large changes:

1. `cmake --build build/debug --target hydra -j2`
2. `./build/debug/hydra_core_smoke`
3. `timeout 6 ./build/debug/hydra -platform offscreen`
4. `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/maintainability-audit`
5. `python3 scripts/validate_wayland_ui.py`
6. `python3 scripts/validate_divider_drag_x11.py`

## Expected outcome

The result of this pass should be a lighter, more explicit, more modular Phase 1 and 2 baseline that is easier to extend in Phase 3 without losing the current visuals or validation coverage.

## Outcome

Completed:

- `QQmlContext` globals removed in favor of explicit root properties and `setInitialProperties()`
- `App.qml`, `LaunchSidebar.qml`, and `SessionBoard.qml` split into smaller named shell components
- shared shell primitives added for chips, panel headers, panel surfaces, frame corners, divider, and help surfaces
- `main.cpp` reduced to a small startup entry point
- shared process runner added for Git and tmux subprocess work
- duplicated slug logic moved into a shared domain helper
- generic help-topic fallback removed
- shell component relationship note added in `docs/architecture/ui-shell-component-map.md`

Validation completed:

1. `cmake --build build/debug --target hydra -j2`
2. `cmake --build build/debug-make -j2`
3. `./build/debug-make/hydra_core_smoke`
4. `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-audit-test.png --quit-after-screenshot`
5. `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/maintainability-audit`
6. `python3 scripts/validate_wayland_ui.py`
7. `python3 scripts/validate_divider_drag_x11.py`
