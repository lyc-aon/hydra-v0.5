Hydra should be designed as a desktop orchestration workbench for agent CLIs, not as a thin wrapper around whichever terminal is fashionable this quarter. Kitty exposes substantial remote-control and session machinery, WezTerm has a CLI plus its own multiplexer, and iTerm2 is scriptable through AppleScript and Python, but Ghostty’s published docs lean more on native windows/tabs/splits and shell integration than on a comparable automation surface. That makes external terminals good integration targets, not a sane core abstraction.

My recommendation is blunt: build Hydra with C++20/23 + Qt 6 + Qt Quick/QML + CMake/Ninja, package it with CPack, and treat performance, polish, and native feel as first-class concerns. Qt 6 officially targets modern macOS and Ubuntu-class Linux environments, Qt Quick Controls already gives you the right desktop primitives like drawers, split views, and tab bars, and Qt’s own guidance is to customize built-in controls rather than rebuild the universe one widget at a time. Qt also explicitly frames smooth UI around keeping frame time in range for 60 FPS, which matters if you want “hot and modern” instead of “glossy until it stutters.”

The core backend should be tmux. Not because tmux is sexy. It is not. It is a stubborn cockroach, which is exactly what you want in session infrastructure. It already solves detach/reattach, multiple panes, multiple sessions, background persistence, and it has a proper control mode with structured notifications, pane/session metadata, and subscription mechanisms. That gives Hydra a stable cross-platform session engine on both macOS and Ubuntu instead of trying to invent resumable terminal process management from scratch.

For terminal rendering inside Hydra, I would not make QTermWidget the foundation if commercial flexibility matters. Its licensing situation is awkward enough to be a needless future argument. A cleaner path is a TerminalSurface abstraction with a production backend built around a permissive terminal emulation core such as libvterm and a future experimental backend for libghostty once that API hardens. libvterm is MIT-licensed and is used by Qt Creator’s integrated terminal; libghostty is interesting, but Ghostty’s own architecture docs describe that embedding-facing library as not yet a stable standalone API.

So the architectural split should be:

Hydra UI shell
QML/Qt Quick app shell, animations, themes, sidebar, repo cards, session board, command palette.

SessionSupervisor
Owns launch, pause, resume, stop, restore, and layout persistence.

MuxAdapter
Primary implementation: TmuxAdapter. Later: optional alternate backends if you find a reason, which you probably won’t.

TerminalSurface
Embedded terminal view abstraction. Never let provider logic talk directly to rendering.

ProviderAdapter
One adapter per CLI family: Claude Code, Codex CLI, Gemini CLI, plus a generic command adapter.

ModelCatalogService
Discovers models, caches them, normalizes aliases and capabilities.

RepoRegistry / WorktreeManager
Handles repo cards, descriptions, default launch profiles, worktree spawning.

StatusAggregator
Merges tmux state, provider hooks/events, prompt markers, and heartbeat state into one normalized session model.

SecretsStore
macOS Keychain on Mac, Secret Service/libsecret on Linux, through a single abstraction such as QtKeychain or native wrappers. Apple explicitly recommends the keychain for small secrets, and libsecret is the standard Secret Service client side on Linux.

The repo-local storage model should be simple and hard to mess up. On first repo add or first launch, Hydra should create a .hydra/ directory at repo root and add it to .git/info/exclude by default so you don’t spray local machine/session metadata into version control. For worktrees, Hydra should use git worktree normally and optionally store worktree-local hints in Git’s per-worktree config mechanisms where useful. Git already gives you both .git/info/exclude for local ignores and config.worktree support for per-worktree configuration.

I would make the repo-local layout something like:

.hydra/providers/ for provider-specific generated overlays

.hydra/docs/ for attachable shared repo docs and model context packs

.hydra/heartbeats/ for heartbeat templates

.hydra/session-templates/ for saved launch layouts

.hydra/local.json for repo-local UI prefs and defaults

Do not make Hydra’s own config format the only source of truth. The right move is to keep a Hydra-level normalized profile, then generate or merge provider-native project config overlays where the provider already supports them.

That matters because the providers already expose real project-level config and session machinery:

Claude Code has user/project/local settings files, model aliases and availability controls, a hook system with structured events like SessionStart, PreToolUse, PermissionRequest, SubagentStart, TaskCompleted, and built-in resume/continue flows scoped to the project or repo.

Codex CLI has user and project config.toml, supports custom model providers, has structured OpenTelemetry event emission, resume/fork flows, and documents its sandbox model on macOS and Linux.

Gemini CLI has user and project settings, configurable hooks, OAuth/API key/Vertex auth modes, and session resume/checkpointing.

That leads to the right adapter contract. Each ProviderAdapter should define:

executable detection

version probe

auth probe

config overlay writer

launch command builder

resume strategy

model discovery strategy

structured event strategy

permission/sandbox options map

worktree compatibility hints

For model discovery, do not hardcode a giant static list unless you enjoy shipping weekly patch releases because vendors renamed something again. Use a layered strategy:

When the provider is authenticated with an API method that supports enumeration, call the official model list endpoint.

When the provider is running primarily through provider-managed OAuth or alias-based UX, expose the provider’s documented aliases/defaults first, then optionally enrich with discovered exact IDs when available.

Cache catalogs locally with a TTL and a manual refresh button.

Display aliases and exact model IDs as separate concepts.

Track stability metadata where the provider publishes it, especially for systems like Gemini where “latest” and preview-style names are intentionally mutable.

For the launch sidebar, I’d structure presets like this:

Provider: Claude / Codex / Gemini / Generic

Model mode: alias or exact ID

Auth lane: OAuth / API key / provider-specific

Reasoning or think level

Filesystem access profile

Network profile

Approval mode

Worktree toggle

Heartbeat template

Repo target

Layout target: single pane stack / tab set / grid

One thing you should not do is make “bypass sandbox / skip permissions” look like a casual convenience toggle. Claude’s docs are explicit that dangerous permission skipping should only be used in a properly sandboxed environment, and Codex also documents default sandbox behavior and OS-level enforcement. Hydra should represent these as named risk profiles, visually loud, with explicit descriptions and one-click duplication from a safe baseline. Humans adore footguns when they are hidden behind soft gray checkboxes.

For session pausing and resuming, use a three-layer restore model:

Hard resume: tmux session still alive, Hydra reattaches instantly.

Soft resume: process is gone, but provider supports session resume, so Hydra relaunches in the same repo/worktree and invokes provider-native resume.

Cold restore: after app restart or reboot, Hydra restores layout, repo/worktree target, profile, and last-known provider session identity, then attempts provider-native resume where available. Claude, Codex, and Gemini all document resume flows, while tmux handles the live detach/reattach part.

For the status area, do not scrape terminal text and pretend that is observability. Build a normalized state machine driven by structured signals:

tmux control-mode events and pane/session metadata

provider-native hooks/events

provider-native telemetry where exposed

shell prompt markers such as OSC 133 when available

desktop notification escape sequences where useful for tab attention and idle alerts

That gives you real states like Starting, Idle, Thinking, RunningTool, AwaitingApproval, WaitingForInput, Backgrounded, Exited, Error. Claude hooks, Codex telemetry, Gemini hooks, and shell integration conventions already give you the raw material.

Your heartbeat feature is viable, but it needs guardrails or it turns into an automated nuisance cannon. I would only send a heartbeat when all of these are true:

session is idle or waiting at prompt

no tool is currently executing

no approval prompt is pending

minimum quiet period has elapsed since last output

heartbeat cap for the interval window has not been exceeded

Make the heartbeat templated, per-repo or per-session, with variables like branch, dirty state, elapsed idle time, and last task summary. Add quiet hours and a kill switch. Technically, dispatch can be done through tmux send-keys or direct PTY writes, but the scheduling decision should live above the transport layer.

For the UI itself: use Qt Quick Controls, but skin them into your own design system instead of shipping another stock Material clone with purple gradients and startup trauma. Use Drawer for the collapsible left rail, SplitView for session layouts, TabBar for grouped sessions, and QML states/transitions for focus, activity, and launch animations. Keep the motion purposeful, keep focus states obvious, and keep the frame budget honest. The goal is “premium workstation,” not “cyberpunk login screen for a crypto exchange.”

Build and release discipline should be boring in the good way: CMakePresets for consistent local and CI configuration, Qt Test for backend and UI-adjacent tests, CPack for DEB and DMG packaging, and Apple’s notarization workflow for macOS distribution. That gets you from “works on my laptop” to “somebody else can install it without ritual.”

One genuinely useful innovative reference is cmux. It is worth studying for interaction ideas around terminal grouping, notifications, and restored layouts, but not as your architectural base. It is macOS-only, built around Ghostty internals, and its current restore behavior is metadata/layout oriented rather than true live process resurrection. Good inspiration, wrong foundation.

Implementation order

Foundation
Create the CMake workspace, Qt app shell, dependency policy, CI matrix, packaging, secrets abstraction, SQLite schema, and ADRs.

Vertical slice
Sidebar + repo registry + launch preset model + tmux-backed session launch + one embedded terminal surface + session list.

Repo intelligence
.hydra/ creation, .git/info/exclude wiring, repo docs area, worktree creation/selection flow.

Provider adapters
Claude, Codex, Gemini, then generic command. Start with launch, resume, config overlay writing, and model selection.

Status and observability
Hook ingestion, tmux subscriptions, prompt markers, event timeline, activity badges.

UX pass
Theme system, keyboard-first navigation, animation polish, session board, command palette, external-terminal handoff.

Hardening
Crash recovery, audit/export, feature flags, telemetry opt-in, permission-profile governance.

Starting prompt

Paste this into Claude/Codex/Gemini as the first serious build prompt:

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

That prompt bakes in the right bets: Qt/QML for the native shell, tmux for cross-platform session semantics, provider-native config and resume instead of fantasy abstractions, official model discovery where available, and a clean path to future terminal backend swaps without betting Hydra on unstable APIs or terminal-specific gimmicks.
