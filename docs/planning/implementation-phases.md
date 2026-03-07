# Implementation Phases

Status legend:

- `Complete`: implemented and verified in the current codebase.
- `In Progress`: some core pieces are real, but the phase exit criteria are not all met.
- `Pending`: planned but not yet implemented.

## Phase 0: Foundation

Status: `In Progress`

Implemented:

- CMake workspace
- Qt 6 application bootstrap
- SQLite schema and migration runner
- ADR set
- desktop app launches locally in the current Ubuntu environment

Deferred:

- typed event bus
- logging and diagnostics baseline
- secrets-store abstraction
- CI for macOS and Ubuntu
- warnings-as-errors discipline across the full product pipeline

Exit criteria:

- app launches on macOS and Ubuntu
- warnings are treated as errors
- CI builds both platforms

Current readout:

The local Ubuntu launch path is real. The phase remains open because CI, the event bus, logging baseline, and secrets abstractions are still missing.

## Phase 1: Vertical Slice

Status: `Complete`

Implemented:

- launch sidebar for repo selection and launch actions
- repo registry
- `tmux` session launch for a generic shell
- persisted session metadata
- session board with normalized state labels and refresh loop
- restart reload against persisted sessions and live `tmux` state

Deferred inside the phase boundary:

- `LaunchProfile` exists only as a model stub; there is no preset selection flow yet
- runtime state normalization is still basic and currently driven by `tmux` liveness rather than structured provider signals

Exit criteria:

- user can launch a real repo-bound shell session
- session survives Hydra restart when `tmux` session is still alive
- metadata persists in SQLite

Current readout:

The Phase 1 slice is real and usable. The remaining gaps are quality-of-life and future-state depth, not blockers for the vertical slice itself.
The launch, persistence, refresh, restart reload, and end-session path were revalidated on 2026-03-06 against isolated app data through `scripts/validate_wayland_ui.py`.
The final refinement pass on 2026-03-06 also validated that the navigation rail can collapse and reopen under live GUI control without breaking the launch/end path.

## Phase 1.5: Shell Tightening

Status: `Complete`

Implemented:

- shared QML theme/token source
- denser shell frame and layered background
- tighter left rail hierarchy
- stronger repo and session cards
- restrained hover and transition behavior
- explicit model-role binding in QML delegates to avoid runtime blank-card failures
- initial accessibility labels on key shell controls for AT-SPI testing
- no fake provider, risk, timeline, or terminal surfaces

Current readout:

Phase 1.5 is complete as a validated shell pass, but it should not be treated as the final visual direction.
Local Ubuntu/Wayland UI validation can now be driven through AT-SPI, and Hydra also now has deterministic screenshot export plus headless Gemini review for visual critique.
A separate Xvfb/X11 harness is still unverified for Hydra itself.
The final refinement pass also removed roadmap/meta copy from the shell itself and added a validated full-pane board mode through the collapsible rail.
The follow-up refinement pass moved the toggle onto the pane divider, made the rail width adjustable, added animated collapse/expand behavior, added press feedback on shell controls, and fixed the session-row action clipping issue.
The Hermes / Steam follow-up refinement pass then retuned the Phase 1 / 2 default theme toward old-Steam / Hermes control-room cues without changing the current product boundary.
The GUI validation harnesses were also hardened to scope AT-SPI lookups to the launched Hydra PID so local automation is not confused by unrelated open Hydra windows.
The deep visual follow-up pass then closed the sticky refresh-banner bug, retuned the default tokens from sampled inspiration colors, and pushed the rail/board/cards farther toward a measured instrument-panel aesthetic rather than a generic dark dashboard.
The Steam palette correction pass then narrowed the palette authority to `Image Inspiration/Oldschool Steam.jpg` specifically and replaced the mixed-reference token drift with exact pane-derived Steam olives, khakis, and border tones.
The micro-alignment refinement pass then removed the remaining strip-collision and split-band label problems from the smaller card/readout surfaces.
The UX-assistance refinement pass then added section-level help buttons, contextual hover descriptions, a quick-help bubble, a detailed in-app help panel, and validated real-GUI capture coverage for those surfaces.
The maintainability audit pass then removed hidden context-property coupling, split the largest shell files into smaller named components, introduced reusable shell primitives, and added a shell component map for the current Phase 1 and 2 implementation.
A follow-on Phase 1 / 2 refactor roadmap is now active to finish the remaining help-surface cleanup, keep the shell modular, and checkpoint the repo with a first local commit before Phase 3.
The latest follow-up refactor pass then removed the remaining native Qt attached-tooltips from the shell and replaced them with one delayed themed hover-hint system so the live UI no longer mixes two incompatible hover surfaces.
The notification polish pass then clipped the launch sweep to its button, made action feedback transient by default, added tone-aware status-banner animation, and validated that empty-worktree warnings now appear and clear automatically.

## Phase 2: Repo Intelligence

Status: `Complete`

Implemented:

- `.hydra/` bootstrapping at repo root
- `.git/info/exclude` wiring for `.hydra/`
- repo-local docs/context pack directory provisioning under `.hydra/docs/`
- Git worktree listing for the selected repository
- Git worktree creation from the launch rail
- worktree selection flow in the shell
- worktree-bound launches by targeting the selected worktree path
- persisted session working directory now serves as the worktree identity for launched sessions

Current limitations:

- no worktree remove/prune flow yet
- no docs/context pack browser or editor yet; the repo-local area is provisioned and surfaced by path only
- no worktree-specific Git config hints yet

Exit criteria:

- repo-local Hydra state is created automatically
- worktree-bound launches behave correctly

Current readout:

The current code meets the Phase 2 exit criteria.
Repo-local bootstrap, Git exclude wiring, worktree visibility/selection, and worktree-bound launch persistence were revalidated on 2026-03-06 against an isolated repo. Backend worktree creation was also rechecked directly through `WorktreeManager`.

Inter-phase note:

The planned Shell V2 rewrite is now complete and documented in `docs/planning/shell-v2-rewrite-plan-2026-03-06.md`.
That rewrite did not change the Phase 2 status; it was a composition/visual-system overhaul on top of already validated Phase 1 / 2 behavior.
Phase 3 can now proceed against the stronger shell baseline rather than the earlier clunky prototype shell.
The remaining visual limitation is primarily missing future product surfaces, not an unresolved Phase 1 / 2 shell defect.
Pointer-specific divider drag validation now runs through `scripts/validate_divider_drag_x11.py`, while `scripts/validate_wayland_ui.py` remains the semantic product-flow validator for the real GNOME Wayland session.
The same Wayland validator now also proves that the refresh confirmation appears and clears as a transient message instead of sticking indefinitely.
Collapsed-rail startup states remain covered by deterministic app-owned capture, while the X11 divider harness is now intentionally scoped to pointer drag resizing and the Wayland harness remains intentionally scoped to semantic product flow.

## Phase 3: Provider Adapters

Status: `Pending`

Deliverables:

- Claude adapter
- Codex adapter
- Gemini adapter
- generic command adapter

Initial scope per adapter:

- detection
- auth probe
- overlay writing
- launch plan
- resume plan
- model selection

Exit criteria:

- each provider can launch from Hydra with repo context
- at least one provider-native resume path works end-to-end

## Phase 4: Status and Observability

Status: `Pending`

Deliverables:

- `tmux` control-mode subscriptions
- provider hook or telemetry ingestion
- prompt-marker support
- event timeline
- activity and approval badges

Exit criteria:

- session board states are driven primarily by structured signals
- status provenance is inspectable

## Phase 5: Terminal Surface

Status: `Pending`

Deliverables:

- `TerminalSurface` abstraction
- initial embedded backend
- focus, input, selection, and scrollback

Exit criteria:

- embedded terminal is good enough for daily use in the vertical slice
- the app still supports external-terminal handoff as a fallback

## Phase 6: UX and Hardening

Status: `Pending`

Deliverables:

- theme system
- keyboard-first navigation
- command palette
- crash recovery
- audit export
- feature flags
- telemetry opt-in
- permission-profile governance

Exit criteria:

- product feels stable under sustained multi-session use
- dangerous profiles are explicit and auditable
