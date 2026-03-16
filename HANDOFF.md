# Hydra v0.5 Handoff

Last updated: 2026-03-15

## Release state

Hydra v0.5 is the public prototype cut for the Nous Research 2026 Hackathon.

Hydra is in launch-ready state. The active desktop shell boots cleanly, closes cleanly, keeps sound enabled, restores the full side rail, exits the core smoke cleanly, and the verified desktop/runtime path is free of repeating QML binding errors.

Current shipped surface:

- `Workbench` session manager with embedded terminal binding
- `Master` view with router-backed orchestration
- router helper flow is asynchronous: master requests go to router immediately, workers report completion back to router later, and router forwards reviewed summaries to master
- smart-router control plane only; no generic shell provider and no pseudo-TUI overlay path
- repo and worktree target management
- resumable provider sessions for supported providers
- audit export and transcript archiving on managed shutdown
- app-exit shutdown keeps unresolved pending resume tokens out of Resume until provider metadata resolves, instead of blocking the UI path or leaving dead Resume entries behind
- the Wayland UI audit now presses the live `End session` button on a Codex worker session and verifies Hydra survives, exits the session cleanly, and can relaunch it
- verified provider matrix on the current build:
  - `Codex` master/router: `idle`
  - `Gemini` master/router: `idle`
  - `Claude Code` master: `awaiting_approval`, router: `idle`
  - `Hermes` master/router: `idle`
- `OpenCode` is wired as a fifth provider with explicit session-id resume, model override, and isolated XDG state for master/router control workspaces; the launch/resume contract is covered in `hydra_provider_launch_overrides_smoke`
- embedded terminal handling is now normalized across providers: tmux mouse stays off, Hydra handles wheel scrollback through tmux copy-mode, and the terminal context menu owns Copy/Paste/Select All for Codex, Gemini, Hermes, and OpenCode; OpenCode keeps one explicit native-terminal exception because its alt-screen UI otherwise breaks drag selection and wheel scrolling in the embedded pane
- tmux-backed multiline paste now preserves literal linefeeds (`paste-buffer -r`) so Hermes and the other chat-style providers no longer receive carriage-return mangled clipboard input
- deleting stored Resume entries now drops them from the active UI snapshot immediately instead of leaving stale cards behind until a later refresh
- OpenCode launch has now been rechecked on the real app path with a live Hydra worker session, not only through adapter-plan smoke coverage

## Hardening notes

- provider launch config now persists the fields needed for accurate resume parity: model override, injected env, and Hermes profile settings
- OpenCode does not expose a clean top-level sandbox flag, so Hydra maps its safety modes through documented inline config: `workspace-safe` forces ask-mode on bash/edit, `bypass` forces allow-mode
- provider embedding policy is now explicit: Hydra keeps provider TUI surfaces native by default and only overrides embed-critical behavior such as alternate-screen handling, control-session state isolation, and observability hints
- Codex and Gemini no longer suppress extra native TUI chrome just for aesthetics; Codex still keeps `--no-alt-screen`, and Gemini still keeps `useAlternateBuffer=false`, because those are the embed-critical pieces
- master/router control launches no longer silently fall back to `$HOME` if workspace staging fails
- idle ambient animation churn was removed from the live shell chrome; a real desktop audit instance now idles at about `0.8%` CPU with zero stderr lines over a 5-second sample
- `validate_wayland_ui.py` now writes Hydra stdout/stderr to temp log files instead of piping them unsafely
- `hydra_shutdown_resume_smoke` now exercises real Codex, Gemini, Claude Code, and Hermes sessions through provider-specific trust/ready flow, multiple inputs, observed output, and managed close/resume on an isolated tmux socket; Gemini interactive prompt verification is best-effort when the isolated prompt does not settle into a deterministic trust/ready footer
- shutdown/terminate no longer block on polling sleeps while waiting for provider resume metadata; pending resume tokens stay hidden until refresh resolves them
- App shutdown is no longer doing a second synchronous owned-session teardown from `App.qml`'s close path; the app-level shutdown hooks own that cleanup so window close does not block the UI thread
- structural boundaries are cleaner now:
  - `SessionSupervisor` lifecycle and shutdown behavior lives outside the launch/resume unit
  - `SessionSupervisor` transcript/audit export and resume-token resolution helpers live outside the live launch/resume unit
  - router/provider runtime helpers, router preset text, and router preset/workspace state no longer live in one file
  - `App.qml` coordinates state while `AppShellSurface.qml`, `StartupOverlayStack.qml`, `AppShortcutHub.qml`, and `CloseConfirmDialog.qml` own the shell frame and peripheral UI surfaces

## Constraints

- Hydra is Linux-first and tmux-first.
- `qmltermwidget` desktop support depends on the prepared runtime import. `tools/prepare_qmltermwidget.sh` will fetch upstream source on demand if that runtime import is missing.
- Sound is desktop-only and uses detached `pw-play` or `aplay`. Set `HYDRA_DISABLE_SOUND=1` to force silence for automation.
- `hydra_shutdown_resume_smoke` is safe to run in the default validation set now because it uses an isolated tmux socket namespace.
- `Hydra.Backend` QML tooling metadata is generated during the build; the source tree no longer carries a checked-in backend `qmldir/plugins.qmltypes` shim.
- Gemini and Codex isolated provider homes are rooted under the generic data directory so provider runtimes do not fail on app-data paths that contain spaces.
- OpenCode control sessions use the same generic-data root strategy, but via XDG data/state/cache overrides instead of a provider-specific home variable.

## Current repo shape

- the checked-in third-party surface is the prepared `qmltermwidget` runtime import only
- source-of-truth docs are `README.md`, `HANDOFF.md`, `docs/README.md`, `docs/how-to-use.md`, `docs/architecture/system-architecture.md`, `docs/provider-embedding-plan.md`, and `docs/validation/gui-automation-script-inventory.md`
- the active helper-script surface is `validate_wayland_ui.py`, `capture_ui_screenshots.py`, `ui_fixture.py`, and `hydra_router_control.py`

## Remaining constraints

- desktop screenshot and boot-control flags remain compiled into the binary because the screenshot and boot-probe harnesses use the real desktop entrypoint.

## Recommended verification

```bash
cmake --preset debug
cmake --build build/debug --target hydra hydra_boot_probe hydra_core_smoke hydra_provider_launch_overrides_smoke hydra_session_order_smoke hydra_terminal_clipboard_routing_smoke hydra_shutdown_resume_smoke hydra_qmllint -j8
ctest --test-dir build/debug --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_core_smoke
./build/debug/hydra_provider_launch_overrides_smoke
./build/debug/hydra_session_order_smoke
./build/debug/hydra_terminal_clipboard_routing_smoke
./build/debug/hydra_shutdown_resume_smoke
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_boot_probe --wait-ms 4000
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_boot_probe --skip-boot --wait-ms 4000
./build/debug/hydra --skip-boot --screenshot /tmp/hydra-launch.png --screenshot-delay-ms 2500 --quit-after-screenshot
```

Use `python3 scripts/validate_wayland_ui.py` as the longer GNOME Wayland audit, not the default fast smoke set.

## Source of truth

- [README.md](README.md)
- [docs/README.md](docs/README.md)
- [docs/how-to-use.md](docs/how-to-use.md)
- [docs/architecture/system-architecture.md](docs/architecture/system-architecture.md)
- [docs/provider-embedding-plan.md](docs/provider-embedding-plan.md)
- [docs/validation/gui-automation-script-inventory.md](docs/validation/gui-automation-script-inventory.md)
