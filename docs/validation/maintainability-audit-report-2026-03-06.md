# Maintainability Audit Report

Date: 2026-03-06
Status: Complete

## Scope

This pass audited and refactored the current implemented Hydra surface through Phase 2.

The goal was to improve maintainability, reduce hidden coupling, shrink oversized files, and keep the validated Phase 1 and 2 behavior intact.

## Research used

Primary sources:

- Qt context-property guidance: https://doc.qt.io/qt-6/qtqml-cppintegration-contextproperties.html
- Qt `QQmlApplicationEngine::setInitialProperties`: https://doc.qt.io/qt-6/qqmlapplicationengine.html
- Qt QML best practices: https://doc.qt.io/qt-6/qtquick-bestpractices.html
- Qt Quick performance: https://doc.qt.io/qt-6/qtquick-performance.html
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

## Main findings before refactor

1. `qml/Hydra/App.qml` was carrying backdrop, shell layout, divider logic, and both help surfaces in one file.
2. `qml/Hydra/components/LaunchSidebar.qml` and `SessionBoard.qml` were each carrying multiple distinct panels inline.
3. QML depended on hidden `QQmlContext` globals for `appState` and startup flags.
4. Shell primitives such as status chips, panel headers, and corner-frame accents were duplicated inline.
5. `src/main.cpp` mixed desktop bootstrap and screenshot-capture orchestration.
6. `git_repo_workspace.cpp` and `tmux_adapter.cpp` duplicated synchronous process-execution logic.
7. slug-generation logic was duplicated in more than one domain service.
8. `HelpCatalog.js` still had a generic fallback path that should never appear in normal usage.

## Changes made

### Explicit QML data flow

Replaced context-property wiring with explicit root properties and `setInitialProperties()`.

Updated:

- `src/main.cpp`
- `src/app/desktop_launch_options.hpp`
- `src/app/desktop_launch_options.cpp`
- `qml/Hydra/App.qml`
- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/components/SessionBoard.qml`

### QML decomposition

Added reusable shell components:

- `qml/Hydra/components/ShellBackdrop.qml`
- `qml/Hydra/components/FrameCorners.qml`
- `qml/Hydra/components/DividerHandle.qml`
- `qml/Hydra/components/QuickHelpBubble.qml`
- `qml/Hydra/components/DetailHelpPanel.qml`
- `qml/Hydra/components/HelpSectionCard.qml`
- `qml/Hydra/components/StatusChip.qml`
- `qml/Hydra/components/SectionHeader.qml`
- `qml/Hydra/components/SurfacePanel.qml`

Split major rail and board sections into components:

- `qml/Hydra/components/ConsoleHeader.qml`
- `qml/Hydra/components/LaunchBusPanel.qml`
- `qml/Hydra/components/TargetMapPanel.qml`
- `qml/Hydra/components/ExecutePanel.qml`
- `qml/Hydra/components/SessionsSurface.qml`

### Backend cleanup

Added shared process execution helper:

- `src/infrastructure/process/process_runner.hpp`
- `src/infrastructure/process/process_runner.cpp`

Refactored to use it:

- `src/infrastructure/mux/tmux_adapter.cpp`
- `src/infrastructure/git/git_repo_workspace.cpp`

Added shared slug helper:

- `src/domain/support/slug.hpp`
- `src/domain/support/slug.cpp`

Refactored to use it:

- `src/domain/services/session_supervisor.cpp`
- `src/domain/services/worktree_manager.cpp`

### Help cleanup

Removed the runtime generic fallback topic from `qml/Hydra/HelpCatalog.js`.

## File-size results

Largest QML files after the pass:

- `qml/Hydra/App.qml`: 349 lines
- `qml/Hydra/components/TargetMapPanel.qml`: 311 lines
- `qml/Hydra/components/SessionsSurface.qml`: 279 lines
- `qml/Hydra/components/SessionBoard.qml`: 218 lines
- `qml/Hydra/components/ExecutePanel.qml`: 208 lines
- `qml/Hydra/components/LaunchSidebar.qml`: 109 lines

Notable C++ sizes after the pass:

- `src/infrastructure/git/git_repo_workspace.cpp`: 486 lines
- `src/ui/viewmodels/app_state.cpp`: 347 lines
- `src/infrastructure/mux/tmux_adapter.cpp`: 126 lines
- `src/app/desktop_launch_options.cpp`: 125 lines
- `src/main.cpp`: 46 lines

## Remaining hotspots

1. `src/infrastructure/git/git_repo_workspace.cpp` is still the largest C++ implementation file and the main remaining backend hotspot.
2. `src/ui/viewmodels/app_state.cpp` still mixes repo reload, workspace reload, and transient status lifecycle in one object.
3. `scripts/validate_wayland_ui.py` is now the largest actively maintained script.

These are still maintainable, but they are the next candidates if Phase 3 increases complexity.

## Validation

Passed:

- `cmake --build build/debug --target hydra -j2`
- `cmake --build build/debug-make -j2`
- `./build/debug-make/hydra_core_smoke`
- `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-audit-test.png --quit-after-screenshot`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/maintainability-audit`
- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`

Visual review artifact:

- `docs/validation/gemini-maintainability-audit-review-2026-03-06.md`

## Audit conclusion

The implemented Phase 1 and Phase 2 shell is now materially more maintainable than it was before this pass.

The most important improvements are:

- explicit QML dependencies instead of hidden globals
- smaller shell files with named responsibilities
- reusable shell primitives instead of repeated inline styling blocks
- separated desktop bootstrap logic
- reduced duplicated backend subprocess and slug logic

This is a stronger baseline for Phase 3 than the earlier monolithic shell files.
