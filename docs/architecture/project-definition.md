# Project Definition

## What Hydra is

Hydra is a native desktop orchestration workbench for running, supervising, restoring, and comparing many parallel agent-cli sessions against real repositories and worktrees.

Hydra is not:

- a web app
- an Electron or Tauri shell
- a terminal emulator with a thin sidebar
- a wrapper around one terminal brand's split API

## Product goals

- Native-feeling macOS and Linux desktop UX.
- Fast, stable session orchestration for long-running agent workflows.
- Strong restore semantics across live sessions, dead processes, and full app restarts.
- First-class provider support for Claude Code, Codex CLI, Gemini CLI, and generic commands.
- Clear risk boundaries around filesystem, network, and approval policies.
- Repo-aware workflows with worktrees, docs packs, shared context, and session templates.

## Non-goals

- Replacing provider CLIs with a single fake abstraction that hides provider differences.
- Reimplementing Git.
- Reimplementing a cloud scheduler.
- Shipping Windows as a first-class target in the first release.

## Primary user flows

### Launch

The user selects a repo, optional worktree, provider, model lane, auth lane, risk profile, layout target, and heartbeat template, then launches one or more sessions.

### Supervise

The user watches session state, output, approval prompts, attention signals, idle status, and worktree bindings from a single board.

### Restore

The user closes Hydra, reopens it, and expects Hydra to reattach live sessions or invoke provider-native resume where possible.

### Compare

The user runs multiple providers or multiple model variants against the same repo and compares progress, cost/risk posture, and outcomes.

## Success criteria

- Hydra can launch and restore real multi-session workflows across multiple repos without losing session identity.
- Status is driven primarily by structured signals, not by terminal text scraping.
- Dangerous permissions are explicit and visibly risky.
- Repo-local `.hydra/` data stays local by default and does not leak into version control.
- The UI feels responsive under realistic parallel-load conditions.
