# Phase 1 / 1.5 / 2 Validation Report

Last updated: 2026-03-06

## Scope

This report records the validation pass for the currently claimed completed phases:

- Phase 1
- Phase 1.5
- Phase 2

The goal was to verify real behavior rather than trust the implementation plan by inspection alone.

## Methods used

### Build and launch checks

Validated:

- `cmake --build build/debug --target hydra -j2`
- `timeout 6 ./build/debug/hydra -platform offscreen`

### UI validation path

Validated with:

- `python3-pyatspi`
- real GNOME Wayland session
- temporary `XDG_DATA_HOME`
- isolated temporary Git repo
- isolated temporary SQLite database

### Backend-only supplemental validation

Validated separately with a direct `WorktreeManager` probe linked against `libhydra_core.a`.

This supplemental step was used only for the worktree-creation path because semantic text entry into the branch field is still not reliable through the supported Wayland automation route.

## Results

### Phase 1

Validated as working:

- Hydra launches and creates its temp SQLite database
- a repository row can be surfaced in the UI and selected
- launching creates a persisted session row
- launching creates a live `tmux` session
- refresh keeps a single visible end-session action
- closing and reopening Hydra reloads the live session
- ending the session removes the live `tmux` session and persists `exited`

### Phase 1.5

Validated as working:

- current key controls render with non-zero geometry
- the previous blank-card runtime failure is not present in the tested flow
- restart and refresh do not produce duplicate visible session actions

Important correction made before this report:

- blank ghost entries were caused by runtime QML delegate failures from `model.*` access, not by SQLite corruption

### Phase 2

Validated as working:

- selecting the repo causes `.hydra/` bootstrap
- `.hydra/docs/` is created
- `.hydra/local.json` is created
- `.git/info/exclude` contains `.hydra/`
- a linked worktree is visible and selectable in the UI
- launching from that selected worktree persists the worktree path as the session working directory
- backend worktree creation via `WorktreeManager` succeeds against an isolated repo

## Evidence summary

UI validation produced these confirmed control geometries in the isolated test run:

- `Repository Validation Repo`: `300x92`
- `Worktree ui-precreated`: `300x68`
- `Worktree branch name`: `190x36`
- `Create worktree`: `78x36`
- `Launch tmux shell`: `276x42`
- `Refresh Hydra state`: `108x38`
- `End session Validation Repo shell`: `52x28`

Isolated session validation confirmed:

- session name: `Validation Repo shell`
- initial state after launch: `idle`
- persisted working directory: isolated linked worktree path
- final state after end: `exited`

## Limits discovered during validation

- Direct AT-SPI button driving works.
- Semantic text entry into the branch field does not currently work through the validated Wayland automation path.
- Screenshot-based visual capture was denied in the tested non-interactive GNOME context.
- `Xvfb` remains unverified for Hydra itself and should not be described as a working validation path.

## Readout against the roadmap

### Confirmed accurate

- Phase 1 marked `Complete`
- Phase 1.5 marked `Complete`
- Phase 2 marked `Complete`

### Confirmed still out of scope

- embedded terminal rendering
- provider adapters
- structured provider telemetry
- command palette
- keyboard-first hardening beyond current accessible naming

## Conclusion

The roadmap is materially accurate for Phase 1, Phase 1.5, and Phase 2, with one important qualification:

- UI-driven creation of a brand-new worktree still needs either manual typing or a future stronger text-input automation path for full semantic automation coverage

That is a validation-tooling boundary, not evidence that the underlying Phase 2 worktree creation capability is absent.
