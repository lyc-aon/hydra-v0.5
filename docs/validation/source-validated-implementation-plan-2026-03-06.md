# Source-Validated Implementation Plan

Validation date: March 6, 2026.

This note cross-checks the Hydra V2 direction against current primary sources and separates confirmed facts from design inference.

## Overall judgment

The core thesis is strong:

- Qt 6 plus Qt Quick/QML remains the right desktop shell bet for a polished macOS and Ubuntu-class app.
- `tmux` remains the strongest cross-platform session backend for detach, reattach, pane metadata, subscriptions, and long-lived process management.
- External terminals should stay integration targets, not Hydra's core abstraction.
- Provider-native config, resume, auth, and event surfaces are real enough today to justify first-class adapters.

The one place to tighten the execution order is embedded terminal rendering. Hydra should keep the `TerminalSurface` abstraction from day one, but the first real value slice should prove repo/session orchestration and persistence before betting the whole schedule on a custom Qt Quick terminal surface.

## Confirmed by current docs

### Qt and the native shell

Qt 6 still officially targets modern desktop platforms, including macOS and Linux. Qt Quick Controls continues to provide the right desktop primitives for Hydra's shell, and Qt's control customization guidance still pushes teams toward customizing built-in controls instead of rebuilding them wholesale. Qt's performance guidance continues to frame 60 FPS as the practical target, which supports the user's emphasis on a smooth workstation feel rather than a decorative but choppy UI.

Assessment:

- Keep Qt 6 + Qt Quick/QML.
- Build a Hydra design system on top of stock controls instead of building a control set from scratch.
- Treat frame budget and animation discipline as product requirements, not polish work.

### tmux as the session engine

The `tmux` control-mode documentation still exposes the machinery Hydra needs: structured notifications, pane and session metadata, and a subscription model. That is still a much saner base than inventing custom resumable PTY/session management across macOS and Linux.

Assessment:

- Keep `TmuxAdapter` as the primary `MuxAdapter`.
- Model session restoration in three layers: hard resume, soft resume, cold restore.
- Normalize `tmux` events into Hydra state, but retain raw `tmux` metadata for diagnostics and auditability.

### External terminals are integration targets

Current terminal docs still support the original framing:

- Kitty exposes remote control and session features.
- WezTerm exposes both CLI control and its own multiplexer.
- iTerm2 remains scriptable through AppleScript and Python.
- Ghostty is interesting, but `libghostty` is still not a mature foundation to bet Hydra's core on.

Assessment:

- Keep external terminal handoff as a supported feature.
- Do not model Hydra around a particular terminal's split/tab/session API.
- Keep any `libghostty` backend explicitly experimental until that API becomes stable.

### Provider-native config and resume are real

Claude Code:

- Anthropic's current docs still describe project-local settings, hooks, model configuration, and conversation continue/resume flows.
- Hook events remain explicit enough to justify a structured event adapter.

Codex CLI:

- OpenAI's current docs confirm user config in `~/.codex/config.toml` and project overrides in `.codex/config.toml`.
- OpenAI also documents `approval_policy`, `sandbox_mode`, custom `model_providers`, `otel.*` settings, `codex resume`, and `codex fork`.

Gemini CLI:

- Google's current docs confirm configuration layering with user and project settings, multiple auth lanes, and restore/checkpointing commands.
- The public Gemini CLI docs support the idea that project-native settings and session restoration matter.

Assessment:

- Keep one `ProviderAdapter` per CLI family.
- Treat provider-native config overlays and resume flows as first-class features, not optional extras.
- Prefer structured signals over terminal-text scraping wherever the provider exposes them.

### Repo-local storage and worktrees

Git's current docs still support local ignore state via `.git/info/exclude`, and per-worktree configuration through `extensions.worktreeConfig` / `config.worktree` behavior. The user's repo-local `.hydra/` plan still fits cleanly with Git's model.

Assessment:

- Create `.hydra/` on first repo use.
- Add `.hydra/` to `.git/info/exclude` by default.
- Store worktree-specific hints in Git's per-worktree config only when the distinction matters.

## Refinements I recommend

### 1. Split "session core" from "terminal rendering" in the schedule

This is the biggest practical refinement.

The user's architecture is right to keep a `TerminalSurface` abstraction. The schedule should still assume that an embedded, high-quality terminal surface is a separate risk bucket from session orchestration. `tmux` launch, resume, repo binding, persistence, worktree handling, and status normalization are all independently valuable before Hydra has a production-grade embedded terminal.

Recommendation:

- Phase 1 vertical slice: repo registry, launch presets, `tmux` launch, session persistence, session board, status chips.
- Phase 2: minimal embedded terminal surface or an interim external-terminal handoff for developer use.
- Phase 3: production terminal backend after the app shell and session model are stable.

This is an inference from the implementation risk, not a direct source claim.

### 2. Model discovery must stay auth-mode aware

The layered model strategy is correct, but it needs one explicit rule: discovery behavior must differ by auth lane.

Recommendation:

- API-key lanes: enumerate official model catalogs where the provider exposes them.
- OAuth or provider-managed lanes: show documented aliases/defaults first, then enrich with exact IDs only where the provider makes them available.
- Cache alias catalogs and exact-ID catalogs separately.

This is an inference from how the provider docs differ by auth mode.

### 3. Normalize state, but preserve raw provenance

The status model is good, but Hydra should store both:

- a normalized state for the UI
- the raw event source that produced it

That matters for debugging cross-provider mismatches and for trust when Hydra says a session is `AwaitingApproval` versus `Idle`.

Recommendation:

- Store every normalized state change with `source_kind`, `source_session_id`, `source_payload_ref`, and timestamp.

This is a design inference.

### 4. Treat dangerous permission profiles as named policies

The user's footgun warning is correct, and the current Claude/Codex docs support it. Hydra should not expose raw toggles first.

Recommendation:

- Ship named policies such as `Read Only`, `Workspace Safe`, `Workspace + Network`, `Unrestricted`.
- Require duplication or explicit confirmation before editing the dangerous profiles.
- Display scope and consequences inline.

This is a design recommendation supported by the provider security docs.

## Cautions and open questions

### Embedded terminal complexity is still real

The `libvterm` direction is sensible, but "sensible" is not the same as "cheap." A Qt Quick terminal surface with proper rendering, selection, IME handling, scrollback, link detection, keyboard fidelity, and accessibility is a real subsystem.

Recommendation:

- Keep the abstraction.
- Do not let terminal rendering block the session-core vertical slice.

### Ghostty remains interesting but not stable enough for a dependency bet

The official Ghostty project materials still present `libghostty` as promising rather than hardened. That is good enough for R&D, not for the only production backend.

Recommendation:

- Track it.
- Do not anchor Hydra's first shipping terminal path on it.

### Gemini hook/event surface needs hands-on verification

Google's public Gemini CLI docs clearly support config layering, auth modes, and restoration concepts. The hook/event story is less crisp in the current public docs than Anthropic's or OpenAI's. That does not break the adapter plan, but it means the Gemini adapter should start conservative:

- launch
- auth probe
- config overlay
- model selection
- restore path

Then add richer event ingestion after direct CLI verification.

This caution is based on weaker public documentation, not on a negative finding.

## Final recommendation

Keep the strategic bets from the original document:

- Qt 6 + Qt Quick/QML shell
- `tmux` core backend
- provider-native adapters
- repo-local `.hydra/` state
- explicit risk profiles
- normalized status model fed by structured signals

Tighten the execution order:

1. Deliver the session core and persistent orchestration shell first.
2. Keep embedded terminal rendering behind `TerminalSurface`.
3. Ship external-terminal handoff early if it accelerates validation.
4. Add the production embedded terminal path only after the rest of the architecture proves itself.

## Source register

Primary sources reviewed on March 6, 2026:

- Qt supported platforms: https://doc.qt.io/qt-6/supported-platforms.html
- Qt Quick Controls customization: https://doc.qt.io/qt-6/qtquickcontrols-customize.html
- Qt Quick performance guidance: https://doc.qt.io/qt-6/qtquick-performance.html
- tmux control mode and subscriptions: https://man7.org/linux/man-pages/man1/tmux.1.html
- Kitty remote control: https://sw.kovidgoyal.net/kitty/remote-control/
- Kitty sessions: https://sw.kovidgoyal.net/kitty/sessions/
- WezTerm CLI: https://wezterm.org/cli/cli/index.html
- WezTerm multiplexing: https://wezterm.org/multiplexing.html
- iTerm2 scripting: https://iterm2.com/documentation-scripting.html
- Ghostty project and `libghostty`: https://github.com/ghostty-org/ghostty
- Claude Code overview: https://docs.anthropic.com/en/docs/claude-code/overview
- Claude Code settings: https://docs.anthropic.com/en/docs/claude-code/settings
- Claude Code hooks: https://docs.anthropic.com/en/docs/claude-code/hooks
- Claude Code continue/resume: https://docs.anthropic.com/en/docs/claude-code/common-workflows
- Codex config reference: https://developers.openai.com/codex/config-reference/
- Codex CLI command reference: https://developers.openai.com/codex/cli/reference/
- Codex security and sandboxing: https://developers.openai.com/codex/security#sandbox-and-approvals
- Gemini CLI configuration: https://google-gemini.github.io/gemini-cli/docs/cli/configuration/
- Gemini CLI authentication: https://google-gemini.github.io/gemini-cli/docs/cli/authentication/
- Gemini CLI commands and restore flows: https://google-gemini.github.io/gemini-cli/docs/cli/commands/
- Git ignore behavior: https://git-scm.com/docs/gitignore
- Git worktree configuration: https://git-scm.com/docs/git-worktree

Note:

The recommendation sections above include some inference about execution order and risk. Those inferences are marked as recommendations rather than source facts.
