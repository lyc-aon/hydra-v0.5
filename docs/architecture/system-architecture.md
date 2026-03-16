# System Architecture

Hydra has four real layers plus a thin desktop shim.

## App

`src/app/`

- boots the desktop process and dependency graph
- owns SQLite bootstrap and runtime-scoped session supervision
- exposes desktop-only helpers such as folder browsing and sound playback

Key classes:

- `HydraApplication`
- `DesktopDialogBridge`
- `DesktopSoundController`

## UI

`src/ui/` and `qml/Hydra/`

- `App.qml` coordinates window state, focus flow, and high-level view switching
- `AppShellSurface.qml` owns the live shell chrome, rail, divider, and view loaders
- `StartupOverlayStack.qml` owns the startup blackout, input gate, boot screen, and splash flow
- `AppShortcutHub.qml` owns the application shortcut bindings
- `CloseConfirmDialog.qml` owns the shutdown confirmation flow
- `AppState` is the main QML-facing viewmodel
- master and router session state are passed into QML as explicit facet objects instead of reaching through `AppState` dynamically
- `WorkbenchPane` is the repo/session workbench
- `MasterTerminalPane` is the control-plane surface
- `TerminalSurfaceController` binds the selected tmux session into the native terminal widget
- the router surface is a real peer terminal, not a fake inline TUI mode

## Domain

`src/domain/`

- `SessionSupervisor` owns launch, refresh, shutdown, and resume rules
- observation, launch/resume, lifecycle/shutdown, transcript/audit export, and resume-token resolution now live in separate translation units instead of one monolith
- `RepoRegistry` and `WorktreeManager` own repo/worktree state
- `StatusAggregator` normalizes session status and provenance
- provider catalog types support the master/router path

## Infrastructure

`src/infrastructure/`

- `tmux` transport
- provider CLI adapters and resolvers
- SQLite persistence
- Git/worktree operations
- native terminal backend glue

## State boundaries

- provider runtime helpers, Hermes profile handling, and API-key env injection live in `app_state_provider_runtime.cpp`
- router preset text and built-in prompt content live in `router_preset_library.cpp`
- router preset persistence, manifest staging, and router workspace helpers live in `app_state_router_presets.cpp`
- live router-session launch/relaunch/accessor behavior lives in `app_state_router.cpp`

## Runtime model

- session transport is `tmux`
- persistence is SQLite under the Hydra app-data directory
- isolated provider runtime state lives under the generic data directory in per-provider roots such as `.../hydra/providers/codex/`, `.../hydra/providers/gemini/`, and `.../hydra/providers/opencode/`
- provider embedding policy is capability-based rather than uniform-by-force: Hydra preserves native provider UI by default and only keeps overrides that are necessary for scroll, mouse, copy/paste, embedding, or resumable control-session state
- embedded terminal interaction is now normalized across providers: tmux mouse is disabled at attach time, Hydra wheel input drives tmux copy-mode scrollback, and the context menu owns Copy/Paste/Select All; OpenCode keeps one explicit native-terminal exception because its alt-screen UI otherwise breaks drag selection and wheel scrolling in the embedded pane
- OpenCode launch policy is mapped through its documented inline config env rather than undocumented storage mutation: Hydra forces ask-mode permissions for `workspace-safe`, allow-mode for `bypass`, and uses XDG overrides to isolate control-session state
- routed worker prompts carry a literal report path and a literal `worker-complete` command so completion reporting does not depend on shell environment inheritance inside provider tools
- app exit shuts down this window's owned tmux sessions and moves resumable provider conversations onto the Resume rail; unresolved pending resume tokens stay hidden until refresh resolves them into a real provider token
- the desktop shell uses a prepared `qmltermwidget` runtime import; the upstream source is fetched only when that runtime needs to be regenerated, and the runtime import is the only checked-in third-party code still on the active path
- the embedded terminal runs without FBO rendering or text antialiasing to keep idle terminal runtime lower under provider TUI load
- sound effects are played out-of-process through desktop audio tools so startup is not blocked by Qt Multimedia
- provider executable probes are cached briefly so refreshes, launches, and stored-session deletion do not repeatedly pay full CLI startup cost
- backend QML tooling metadata is generated in-build for `Hydra.Backend`
- screenshot and boot-probe launch flags live in `src/app/desktop_launch_options.*` as verification hooks for the desktop shell
- the live shutdown/terminate path no longer blocks on sleep-based polling while waiting for provider resume metadata
- `hydra_shutdown_resume_smoke` uses an isolated tmux socket namespace rather than the default server
- `hydra_shutdown_resume_smoke` now also covers real provider interaction loops for Codex, Gemini, Claude Code, and Hermes before the shutdown/resume assertions, with Gemini prompt validation treated as best-effort when its isolated control home never reaches a deterministic trust/ready footer
- the debug CMake preset enables the safe CTest smoke surface by default

## Current constraints

- Linux desktop only
- no provider auth management
- provider validation still depends on the locally installed CLIs and auth state for Codex, Gemini, Claude Code, Hermes, and OpenCode
- desktop builds still depend on the prepared `qmltermwidget` runtime import
