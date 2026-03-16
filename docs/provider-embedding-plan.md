# Provider Embedding Plan

Last updated: 2026-03-15

## Goal

Keep provider sessions as native as possible inside Hydra while preserving the embed-critical invariants:

- wheel scrolling works in embedded terminals
- left/right mouse behavior works
- copy, paste, and selection work
- session preview state tracks real provider activity well enough to trust the rail
- session data remains resumable after session end or app exit
- launch, refresh, and resume-delete actions stay responsive under the normal provider set

## Deterministic policy

Hydra should only override provider UI when the override is necessary to preserve terminal embedding.

Allowed normalization:

- disable alternate-screen behavior when the provider TUI breaks embedded scroll, selection, or mouse routing
- disable provider autoupdate checks that add noise or mutate state during launch
- isolate provider home / XDG runtime state for master/router control sessions
- stage provider-native prompt files required for control sessions
- add provider-specific observability hints for idle / thinking / approval / input-required state

Disallowed normalization unless a regression proves it necessary:

- suppressing animations purely for aesthetics
- suppressing tooltips, status lines, spinners, or footer chrome purely for aesthetics
- rewriting provider UI into a Hydra-specific pseudo-TUI
- forcing worker sessions into isolated homes when shared state is the expected native workflow

## Provider posture

- Codex:
  - keep `--no-alt-screen`
  - keep control-session isolated `CODEX_HOME`
  - keep update suppression
  - do not suppress native TUI tooltips / status / notifications unless they break embedding
- Gemini:
  - keep `useAlternateBuffer=false`
  - keep control-session isolated `GEMINI_CLI_HOME`
  - do not suppress spinner / loading copy unless they break embedding
- Claude Code:
  - stay close to native
  - keep prompt-file staging and status-hint tuning
- Hermes:
  - keep isolated `HERMES_HOME` because that is how Hydra gets deterministic control-session state
  - avoid visible bootstrap noise
  - keep shared embedded terminal behavior for scroll, selection, and paste
- OpenCode:
  - stay close to native
  - keep XDG-isolated control-session state
  - improve observability hints rather than flattening the UI
  - allow one explicit terminal exception: local mouse-selection stays enabled and tmux mouse stays enabled when the native OpenCode alt-screen UI would otherwise break drag selection and wheel scrolling in the embedded pane
  - treat any other provider-specific terminal behavior as a regression that must be justified before it stays

## Verification checklist

Automated:

- `cmake --build build/debug --target hydra hydra_qmllint hydra_provider_launch_overrides_smoke hydra_terminal_clipboard_routing_smoke hydra_terminal_interactive_attach_smoke hydra_shutdown_resume_smoke hydra_boot_probe -j8`
- `./build/debug/hydra_provider_launch_overrides_smoke`
- `./build/debug/hydra_terminal_clipboard_routing_smoke`
- `./build/debug/hydra_terminal_interactive_attach_smoke`
- `./build/debug/hydra_shutdown_resume_smoke`
- `QT_QPA_PLATFORM=offscreen ./build/debug/hydra_boot_probe --skip-boot --wait-ms 4000`
- `python3 scripts/validate_wayland_ui.py`
- verify that repeated refreshes and resume-delete actions no longer feel blocked by repeated provider `--version` probes

Human-emulated / desktop:

- launch a fresh Hydra window separate from any productive session
- open one worker session for each provider that is locally available
- verify wheel scroll, drag selection, right-click menu, Copy, Paste, and Select All in the embedded terminal
- verify multiline paste lands as real linefeeds in provider chats and is not rewritten into `^M` / carriage-return noise
- verify left-click focus handoff between workbench, master, and router terminals
- verify terminal background / foreground colors still track the active Hydra theme
- verify the rail/session preview changes from active to idle without obvious false flips while the provider is working
- verify ending a session moves it to Resume instead of relaunching it automatically on next app boot
- verify deleting stored Resume entries does not visibly stall the shell
- verify deleting stored Resume entries removes the card immediately rather than only showing a toast
- verify OpenCode actually renders a live session in the workbench terminal, not just a created session row in SQLite
- verify OpenCode launches `idle`, flips active only while actually working, and returns to `idle` on the native footer (`tab agents` / `ctrl+p commands`)

## Current straight path

1. Remove only cosmetic provider-TUI suppression first.
2. Rebuild and rerun the focused terminal/provider smokes.
3. Recheck real UI behavior on a separate Hydra instance.
4. Only add back a suppression if a specific regression is reproduced and documented.
5. Treat provider-specific interaction smokes as best-effort only when the upstream CLI prompt itself is nondeterministic in isolated control homes, and write that exception down explicitly.

## Launch latency checklist

Deterministic rollout for launch/preview responsiveness:

1. Show provider entries immediately from static adapter metadata.
2. Move expensive provider `--version` probes off the routine refresh path.
3. Refresh provider probes asynchronously on lifecycle start and only occasionally afterward.
4. After launch, resume, master launch, router launch, or terminate:
   - update the in-memory session snapshot immediately from the supervisor result
   - let the session card and preview surface appear from that local state first
   - schedule the full refresh behind it instead of blocking the visible result on the refresh bundle
5. Keep the heavy reconciliation path intact in the background:
   - provider probe refresh
   - workspace reload
   - session-state refresh
   - resume-token discovery
   - timeline reload
6. Do not touch terminal interaction behavior while hardening launch latency.

Manual verification after a rebuild:

- provider selector appears immediately on app boot, even before real availability probes settle
- launching a worker adds the session card immediately instead of waiting for the next full refresh
- launching master/router adds the control session immediately instead of waiting for the next full refresh
- resuming a session shows the resumed card immediately
- terminating a live session removes it from the live surface immediately and moves it to Resume
- provider availability badges/status still settle correctly after the async probe pass
- no terminal interaction regressions were introduced while improving launch latency
