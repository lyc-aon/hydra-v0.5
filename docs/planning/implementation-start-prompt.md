# Implementation Start Prompt

Use this when you want another coding agent to start building Hydra from the architecture in this repo.

```text
You are the principal engineer for a new desktop application called Hydra.

Hydra is a cross-platform desktop orchestration workbench for heavy parallel agentic coding sessions. It is NOT a web app, NOT Electron, NOT Tauri, and NOT a thin wrapper around one terminal emulator’s split-pane API.

Primary goals:
- Native-feeling desktop app
- Compiles in C++ on macOS and Ubuntu
- Modern, premium, animated UI
- Strong session persistence and resume
- First-class support for AI provider CLIs
- Clean architecture with future optionality

Technology constraints:
- C++20 or C++23
- Qt 6
- Qt Quick / QML for the UI
- CMake + Ninja build
- SQLite for local persistence
- tmux as the primary session/multiplexer backend
- No GPL-encumbered core dependencies
- Keep platform-specific code isolated behind interfaces

Product requirements:
1. Collapsible left sidebar with:
   - provider selection
   - model selection
   - auth lane selection
   - reasoning/think level if supported
   - sandbox/approval/access profile
   - worktree toggle
   - heartbeat selection
   - repo selection

2. Repositories:
   - add/remove/edit repo cards
   - each repo has name, path, short description, icon/color
   - each repo gets a local .hydra directory on first use
   - .hydra should be excluded from git by default via local ignore mechanisms
   - each repo has a docs/context area for model-usable files

3. Sessions:
   - launch one or many sessions into a repo or worktree
   - support multiple panes/tabs/groups
   - session pause/resume
   - if tmux session is alive, reattach
   - if process is gone, use provider-native resume when available
   - restore last layout and metadata after app restart

4. Providers:
   - initial adapters for Claude Code, Codex CLI, Gemini CLI, plus GenericCommandAdapter
   - each adapter must support:
     - executable detection
     - version probe
     - auth probe
     - model discovery strategy
     - launch command builder
     - resume strategy
     - config overlay writer
     - structured event/status ingestion strategy

5. Model discovery:
   - use official model-list APIs when available through current auth mode
   - support aliases and exact model IDs as separate concepts
   - cache discovered models locally with TTL
   - allow manual refresh
   - never rely on scraping CLI help output as the authoritative catalog

6. Status area:
   - show all active sessions
   - normalized state model:
     Starting, Idle, Thinking, RunningTool, AwaitingApproval, WaitingForInput, Backgrounded, Exited, Error
   - derive status from:
     - tmux metadata/control mode
     - provider-native hooks/events/telemetry
     - shell prompt markers where available
   - do not use fragile terminal-text scraping as the primary signal

7. Heartbeat:
   - optional per-repo or per-session prompt heartbeat
   - configurable interval
   - only send when session is truly idle
   - support templated variables
   - include quiet hours, max consecutive sends, and emergency kill switch

8. Settings:
   - multiple themes
   - keyboard shortcuts
   - external terminal handoff preferences
   - auth/secrets management
   - logging/telemetry controls

Architecture requirements:
- Separate interfaces from implementations
- No god objects
- RAII everywhere
- Move blocking work off the UI thread
- Use a typed event bus or equivalent clear eventing mechanism
- Make the terminal rendering surface abstract so we can swap implementations later
- Keep provider-specific logic out of the generic session core
- Keep platform-specific secret storage, packaging, and integrations behind thin adapters

Deliverables in this order:
1. A concise architecture decision record set
2. A proposed repository/file tree
3. The CMake workspace skeleton
4. Core interface definitions
5. SQLite schema proposal
6. Minimal Qt Quick shell with:
   - ApplicationWindow
   - collapsible sidebar
   - repo list
   - session board
7. A working vertical slice that:
   - launches a tmux-backed generic shell session into a selected repo
   - shows session state in the UI
   - persists metadata to SQLite
8. Stub provider adapters for Claude/Codex/Gemini
9. Build and run instructions for macOS and Ubuntu
10. Tests for core launch/session logic

Code quality rules:
- warnings as errors
- consistent formatting
- no placeholder architecture diagrams without concrete types
- no fake implementations that silently ignore errors
- every subsystem must have explicit ownership boundaries
- return concrete file contents, not only prose

Start by producing:
A) the architecture overview,
B) the repo tree,
C) the main interfaces/classes,
D) the first-pass CMakeLists files,
E) the first vertical-slice implementation plan.

Do not try to generate the entire finished application in one pass. Make concrete decisions, state assumptions, and build a real scaffold.
```
