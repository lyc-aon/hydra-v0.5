# UI Shell Component Map

Last updated: 2026-03-06

## Purpose

This note explains how the current Phase 1 and Phase 2 shell is composed after the maintainability audit pass.

## Root composition

`qml/Hydra/App.qml`

Owns:

- root window sizing and shell breakpoints
- rail collapse and resize state
- delayed hover-hint routing
- quick-help and detailed-help routing
- shell frame composition
- explicit wiring of `appState` into the rail and board

Delegates visual structure to:

- `components/ShellBackdrop.qml`
- `components/FrameCorners.qml`
- `components/DividerHandle.qml`
- `components/HoverHintBubble.qml`
- `components/QuickHelpBubble.qml`
- `components/DetailHelpPanel.qml`
- `components/LaunchSidebar.qml`
- `components/SessionBoard.qml`

## Left rail

`qml/Hydra/components/LaunchSidebar.qml`

Owns:

- the scrollable rail container
- width-mode derivation for dense and tight states
- assembly of the rail panels

Contains:

- `ConsoleHeader.qml`
- `LaunchBusPanel.qml`
- `TargetMapPanel.qml`
- `ExecutePanel.qml`

### ConsoleHeader

Shows:

- product masthead
- `MUX` readiness chip
- repository count chip
- worktree count chip
- section help entry point

### LaunchBusPanel

Shows:

- generic-shell readiness state
- tmux readiness state
- launch-preflight copy

### TargetMapPanel

Shows:

- repository list
- worktree list
- worktree creation field and action
- target-map help entry point

Uses:

- `RepoCard.qml`
- `WorktreeCard.qml`
- `StatusChip.qml`
- `SectionHeader.qml`
- `SurfacePanel.qml`

### ExecutePanel

Shows:

- current selected target label
- launch path
- main launch action
- execute help entry point

Uses:

- `StatusChip.qml`
- `SectionHeader.qml`
- `SurfacePanel.qml`
- `FrameCorners.qml`

## Session board

`qml/Hydra/components/SessionBoard.qml`

Owns:

- board title row
- refresh action
- summary metrics
- transient status banner
- assembly of the live-session surface

Contains:

- `MetricTile.qml`
- `SessionsSurface.qml`

### SessionsSurface

Shows:

- live mux and target chips
- standby or live header row
- empty-state guidance
- active session list

Uses:

- `StatusChip.qml`
- `FrameCorners.qml`
- `SessionCard.qml`

## Help system

The current help system is intentionally local to the Phase 1 and 2 shell.

`qml/Hydra/HelpCatalog.js`

Owns:

- section help topics for the current shell only
- quick-help and detailed-help content payloads

The flow is:

1. a hoverable shell element calls `helpHost.queueHoverHint(...)`
2. `App.qml` delays and positions the hover hint
3. `HoverHintBubble.qml` renders the short hover explanation
4. a panel info button triggers `helpHost.openQuickHelp(...)`
5. `App.qml` resolves the topic from `HelpCatalog.js`
6. `QuickHelpBubble.qml` renders the short click-open explanation
7. `DetailHelpPanel.qml` renders the longer explanation
8. `HelpSectionCard.qml` renders the repeated detail sections

## Shared shell primitives

These components now carry the repeated surface logic that had been inlined across the shell:

- `StatusChip.qml`
- `SectionHeader.qml`
- `SurfacePanel.qml`
- `FrameCorners.qml`

## Data flow

The current QML shell no longer depends on context-property globals.

The flow is now explicit:

1. `src/main.cpp` passes `appState` and startup options through `QQmlApplicationEngine::setInitialProperties()`
2. `App.qml` receives them as root properties
3. `App.qml` passes `appState` into `LaunchSidebar.qml` and `SessionBoard.qml`
4. rail and board subcomponents receive `appState` through explicit required properties

This keeps the dependency graph visible to both readers and QML tooling.
