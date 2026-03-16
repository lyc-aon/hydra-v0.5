# HYDRA v0.5

Hydra v0.5 is a prototype Linux Qt/QML desktop workbench for launching, supervising, restoring, and comparing parallel agent CLI sessions over `tmux`.

Prototype for the Nous Research 2026 Hackathon.

- website: https://hydragui.app
- X/Twitter: [@lyc_aon](https://x.com/lyc_aon)

## Shipped surface

- providers: `Codex CLI`, `Gemini CLI`, `Claude Code`, `Hermes CLI`, `OpenCode`
- views: `Workbench` for session management and `Master` for router-driven orchestration
- router flow: master sends natural-language requests to the router, router dispatches workers asynchronously, workers report completion back to router, and router forwards reviewed summaries to master
- persistence: SQLite for repos, sessions, shell state, resumable provider metadata, and audit paths
- terminal: embedded `qmltermwidget` bound to the selected tmux session
- sound: bundled WAV cues played through detached Linux audio tools so startup stays non-blocking
- provider embedding policy: preserve native provider UI by default and only override embed-critical behavior such as alternate-screen handling, control-session isolation, and state-observability hints

Hermes support is intentionally limited but active. Launch, resume metadata, and profile-mode selection are part of the supported product surface.
OpenCode support is launch-ready with explicit session-id resume, CLI model override, Hydra-managed safety mapping via inline config, and XDG-isolated control-session state for master/router workspaces.

## Repo shape

- first-party runtime and tooling code lives in `src/` and `qml/Hydra/`, with only a small docs/scripts surface around it
- the only checked-in vendor surface is `third_party/qmltermwidget_runtime/`
- full `qmltermwidget` upstream source is fetched only when that runtime import needs to be regenerated
- structural hot spots were split by responsibility:
  - `SessionSupervisor` runtime observation, launch/resume, and lifecycle/shutdown code no longer live in one translation unit
  - transcript/audit export and provider resume-token resolution now live outside the live launch/resume unit
  - router/provider runtime helpers and router preset/workspace code no longer live in one `AppState` file
  - `App.qml` now delegates shell chrome/layout to `AppShellSurface`, startup overlays to `StartupOverlayStack`, and shell shortcuts / shutdown confirmation UI to dedicated components
- the active docs set is intentionally small and listed below

## Build

```bash
tools/prepare_qmltermwidget.sh
cmake --preset debug
cmake --build build/debug --target hydra hydra_boot_probe hydra_core_smoke hydra_provider_launch_overrides_smoke hydra_session_order_smoke hydra_terminal_clipboard_routing_smoke hydra_shutdown_resume_smoke hydra_qmllint -j8
ctest --test-dir build/debug --output-on-failure
```

`cmake --preset debug` now enables the safe CTest surface by default. `tools/prepare_qmltermwidget.sh` fetches `qmltermwidget` source on demand when the prepared runtime import is missing. The checked-in desktop dependency surface is the generated runtime import, not the full upstream source tree.

## Launch

```bash
./build/debug/hydra
```

Skip the boot flow:

```bash
./build/debug/hydra --skip-boot
```

## How To Use

Workbench flow:

1. Select or add a repo/worktree in the left rail.
2. Pick a provider.
3. Launch a worker session.
4. Use the embedded terminal normally: scroll, select, copy, paste, and resume.

Master/router flow:

1. Switch to the `Master` view.
2. Launch `Master` and `Router`.
3. Send natural-language requests to the master.
4. Let the router dispatch worker tasks asynchronously.
5. Review worker reports coming back through the router into the master.

More detail lives in [docs/how-to-use.md](docs/how-to-use.md).

## Repo surface

- active operator docs are `README.md`, `HANDOFF.md`, `docs/how-to-use.md`, `docs/architecture/system-architecture.md`, `docs/provider-embedding-plan.md`, and `docs/validation/gui-automation-script-inventory.md`
- active helper scripts are `scripts/validate_wayland_ui.py`, `scripts/capture_ui_screenshots.py`, `scripts/ui_fixture.py`, and `scripts/hydra_router_control.py`
- `third_party/` carries only the prepared `qmltermwidget` runtime import needed by the desktop shell
- `Hydra.Backend` tooling metadata is generated during the build and is not checked in as hand-maintained source
- `hydra_shutdown_resume_smoke` now uses its own isolated tmux socket namespace and does not touch the default tmux server

## Safe verification

```bash
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_core_smoke
./build/debug/hydra_provider_launch_overrides_smoke
./build/debug/hydra_session_order_smoke
./build/debug/hydra_terminal_clipboard_routing_smoke
./build/debug/hydra_shutdown_resume_smoke
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_boot_probe --wait-ms 4000
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_boot_probe --skip-boot --wait-ms 4000
./build/debug/hydra --skip-boot --screenshot /tmp/hydra-launch.png --screenshot-delay-ms 2500 --quit-after-screenshot
```

`scripts/validate_wayland_ui.py` is still kept as the extended GNOME Wayland audit, but it is no longer treated as a fast default smoke gate.
It now presses the live `End session` action on a Codex worker session, verifies Hydra stays up, confirms the session exits cleanly, and relaunches the same session before the restart checks.

## Verified runtime

- desktop boot reaches `startupShellReady=true` with and without the boot overlay
- the verified launch path is free of repeating QML binding/runtime errors and idle stderr spam
- a real desktop audit instance now idles at about `0.8%` CPU over a 5-second `pidstat` sample
- the embedded terminal uses a non-FBO rendering path to keep the live runtime lighter under provider TUI load
- provider matrix verified on the current build:
  - `Codex`: master and router launch cleanly and settle `idle`
  - `Gemini`: master and router launch cleanly and settle `idle`
  - `Claude Code`: master settles `awaiting_approval`, router settles `idle`
  - `Hermes`: master and router launch cleanly and settle `idle`
- `OpenCode` launch/resume wiring is covered by `hydra_provider_launch_overrides_smoke`: launch uses the real TUI, resume uses `--session`, worker launches keep shared state, and non-worker control sessions use isolated XDG data/state/cache roots
- provider terminal integration now follows one shared embedded policy across Codex, Gemini, Hermes, and OpenCode: tmux mouse stays disabled, Hydra owns wheel scrollback through tmux copy-mode, and Copy/Paste/Select All use the same context-menu path; OpenCode keeps one explicit native-terminal exception because its alt-screen UI otherwise breaks drag selection and wheel scrolling in the embedded pane
- tmux text paste now preserves literal linefeeds with `paste-buffer -r`, which keeps multiline provider chat paste intact instead of rewriting it into carriage-return noise
- `hydra_shutdown_resume_smoke` now drives real Codex, Gemini, Claude Code, and Hermes sessions through launch, provider-specific trust/ready transitions, multiple inputs, provider output, and managed close/resume paths on an isolated tmux socket
- `hydra_shutdown_resume_smoke` treats Gemini interactive prompt validation as best-effort because the isolated Gemini control home does not reach a deterministic trust/ready footer on every machine run; the shutdown/resume assertions still remain hard-gated
- app-exit shutdown now keeps unresolved pending resume tokens hidden from Resume until provider metadata resolves, instead of blocking the UI path or leaving dead Resume entries behind
- deleting stored Resume entries now removes them from the live UI snapshot immediately instead of waiting on a later refresh cycle
- OpenCode launch is verified on the real app path, not just via provider-plan smoke coverage

## Current latency profile

- isolated `./build/debug/hydra --skip-boot` reaches a screenshot/quit path in about `3.0s` with `0` stdout lines and `0` stderr lines
- the same isolated real-app instance averages about `0.4%` CPU over a 5-second `pidstat` sample after boot
- provider readiness still depends on external CLI probes in the async refresh path; on this machine the slowest current probe is `gemini --version` at about `0.88s`, followed by `opencode --version` at about `0.43s`
- OpenCode is currently the heaviest session-launch path: native OpenCode TUI startup to prompt/footer is about `1.76s`, and Hydra's OpenCode resolver shells out to `opencode session list --format json -n 200`, which costs about `0.49s` per call
- launch/resume/terminate/master/router actions still end by running a full async `refresh()` snapshot, so the visible delay before session cards and preview details settle is often the refresh bundle rather than tmux attach itself

## Docs

- [HANDOFF.md](HANDOFF.md)
- [docs/README.md](docs/README.md)
- [docs/how-to-use.md](docs/how-to-use.md)
- [docs/architecture/system-architecture.md](docs/architecture/system-architecture.md)
- [docs/provider-embedding-plan.md](docs/provider-embedding-plan.md)
- [docs/validation/gui-automation-script-inventory.md](docs/validation/gui-automation-script-inventory.md)

## Known constraints

- Hydra is Linux-first and tmux-first.
- desktop builds still depend on the prepared `qmltermwidget` runtime import under `third_party/qmltermwidget_runtime/`.
- `tools/prepare_qmltermwidget.sh` fetches upstream `qmltermwidget` source only when that runtime import needs to be regenerated.
