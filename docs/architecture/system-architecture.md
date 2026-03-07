# System Architecture

## Architecture shape

Hydra should be organized into five layers:

1. `app`
2. `ui`
3. `domain`
4. `infrastructure`
5. `platform`

## Layer responsibilities

### app

Owns startup, dependency wiring, settings bootstrap, database migration, and application lifecycle.

### ui

Owns QML views, Qt Quick state machines, design tokens, keyboard routing, session board composition, and command palette interactions. UI components do not talk directly to providers, `tmux`, or secrets APIs.

### domain

Owns the typed session model, repo model, worktree model, launch presets, restore policies, heartbeat rules, and normalized status transitions.

Core services:

- `SessionSupervisor`
- `StatusAggregator`
- `RepoRegistry`
- `WorktreeManager`
- `ModelCatalogService`
- `ProfileRegistry`
- `LayoutPersistenceService`

### infrastructure

Owns concrete adapters for:

- `TmuxAdapter`
- provider CLIs
- SQLite persistence
- terminal backends
- Git operations
- log ingestion
- notification routing

### platform

Owns thin OS-specific shims:

- secrets storage
- file opener integration
- notifications
- packaging hooks
- macOS notarization helpers

## Core component boundaries

### Hydra UI Shell

The app shell is a Qt Quick surface that hosts:

- launch sidebar
- repo list or repo cards
- session board
- timeline and diagnostics panes
- command palette
- settings surface

It binds to immutable view models emitted by the domain layer.

### SessionSupervisor

The `SessionSupervisor` is the orchestrator for launch, stop, pause, restore, and rebinding. It does not render output and does not parse provider-specific config formats directly.

It coordinates:

- `MuxAdapter`
- `ProviderAdapter`
- `SessionStore`
- `LayoutPersistenceService`
- `StatusAggregator`

### MuxAdapter

`MuxAdapter` is the session transport boundary.

Initial implementation:

- `TmuxAdapter`

Responsibilities:

- create and destroy mux sessions
- create panes, tabs, and groups
- expose pane/session metadata
- subscribe to control-mode events
- send input
- capture output references when needed

### TerminalSurface

`TerminalSurface` is only about rendering and local interaction.

Responsibilities:

- render terminal buffer state
- accept keyboard and clipboard input
- manage selection and scrollback
- expose focus and attention state

Non-responsibilities:

- launch providers
- decide status
- own session restore logic

### ProviderAdapter

Each provider adapter is responsible for translating Hydra launch and restore intents into provider-native commands and config overlays.

Initial adapters:

- `ClaudeCodeAdapter`
- `CodexCliAdapter`
- `GeminiCliAdapter`
- `GenericCommandAdapter`

### StatusAggregator

`StatusAggregator` converts raw events into a normalized Hydra session state:

- `Starting`
- `Idle`
- `Thinking`
- `RunningTool`
- `AwaitingApproval`
- `WaitingForInput`
- `Backgrounded`
- `Exited`
- `Error`

Every normalized state update should also retain provenance:

- source kind
- source session id
- source payload reference
- timestamp

## Restore model

### Hard resume

`tmux` session still exists. Hydra reattaches to the existing session/panes and restores the UI layout.

### Soft resume

The original process is gone, but the provider exposes a native session resume path. Hydra relaunches in the correct repo/worktree and invokes provider-native resume.

### Cold restore

Hydra restores repo selection, worktree binding, layout, profile, provider identity, and last-known session ids after app restart. It then attempts hard or soft resume in order.

## Eventing model

Hydra should use a typed internal event bus.

Event families:

- repo events
- worktree events
- session lifecycle events
- provider events
- mux events
- status transitions
- heartbeat events
- audit events

Blocking work must stay off the UI thread. UI receives already-shaped view models and lightweight notifications.
