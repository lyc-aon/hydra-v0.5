# Hydra V2 Design Review And Architecture-Aligned Plan

Date: 2026-03-06
Status: Adapted from Gemini output, normalized for Hydra architecture
Editors: Gemini CLI direction, refined by Codex

## Executive Assessment

Hydra V2 is ready for a serious shell-level and system-level design pass.
It is not ready for a full end-to-end product UX lock.

Reason:

- the repository contains a real native vertical slice
- the shell composition is real enough to design against
- the architectural boundaries are clear enough to constrain the work
- the application surface is still too narrow to freeze the whole future product

The design effort should therefore target:

- the app shell
- the design token system
- the current repo/session board flows
- state semantics and motion language
- responsive rules for the shell
- a forward blueprint for later phases

It should not assume that provider flows, worktree flows, timelines, or embedded terminal behavior already exist in the code.

## What Exists Now vs What Is Future Blueprint

### What Exists Now

Current code and handoff establish a real Phase 1 slice:

- C++23 + CMake workspace
- Qt 6 core + SQL build path
- conditional Qt Quick/QML desktop target
- SQLite persistence for `repos` and `sessions`
- `HydraApplication` bootstrap and app-state wiring
- `TmuxAdapter` launch and liveness probing
- `SessionSupervisor` generic-shell launch flow
- `AppState`, `RepoListModel`, and `SessionListModel`
- a two-column QML shell with launch/repo rail and session board
- a smoke binary that boots the orchestration spine without QML

Current validated behavior is narrow:

- bootstrap one repository
- select a repository
- launch a generic shell into detached `tmux`
- persist session metadata
- refresh live/dead session state
- show repo/session counts and simple session cards

### What Is Future Blueprint

These are architectural commitments, not present-day product facts:

- provider adapters for Claude, Codex, Gemini, and generic commands
- worktree management and `.hydra/` repo intelligence
- typed event bus
- richer restore semantics beyond current live `tmux` detection
- structured status ingestion via provider hooks and `tmux` control mode
- timeline and diagnostics panes
- embedded `TerminalSurface`
- permission-profile governance, audit, telemetry, command palette, keyboard-first hardening

Design work should expose room for these features without pretending they already exist.

## Architecture Compatibility Rules

This is the main conversion from the Gemini memo into Hydra-compatible guidance.

### Rule 1: UI owns shell, composition, and tokens only

The `ui` layer should own:

- QML views
- design tokens
- shell layout
- keyboard routing
- animations and transitions
- visual state treatments

The UI should not:

- talk directly to provider CLIs
- talk directly to `tmux`
- decide session truth from raw process output
- invent business state locally in QML

### Rule 2: Domain owns normalized state and launch semantics

The `domain` layer should continue to own:

- session state enum and transitions
- launch intent semantics
- restore semantics
- profile and risk concepts
- future worktree and provider-facing orchestration rules

Visual language can be designed now, but the authoritative state model stays in `domain`.

### Rule 3: Infrastructure owns transport and provider details

`TmuxAdapter`, provider adapters, persistence, future terminal backends, and future Git/worktree operations stay in `infrastructure`.

Design should never require QML to understand provider-specific command shapes or mux behavior.

### Rule 4: Terminal rendering stays deferred

The Gemini suggestion to make the shell more terminal-centric must be constrained.
Hydra's current product value is orchestration, persistence, supervision, and restore.
The embedded terminal path remains Phase 5 behind `TerminalSurface`.

### Rule 5: The shell should scale forward, not jump forward

Do not add permanent panes or controls that do not yet have data behind them.
The shell should reserve expansion paths for future phases, but the current implementation should remain honest to the current slice.

## Design North Star

Primary direction: `Lernaean Workbench`

This is the strongest part of the Gemini direction and should stay.

### Visual Thesis

Hydra should feel like a native orchestration workbench shaped by:

- marsh depth
- bronze wear
- bone paper
- wet stone
- ember heat
- graphite machinery

The goal is not fantasy illustration.
The goal is a premium workstation that quietly carries mythic identity in structure, color, motion, and state transitions.

### Palette Direction

Core surfaces:

- bone / chalk surfaces for readable boards
- graphite / soot for structural shells
- dark slate for heavy containers

Semantic accents:

- oxidized bronze for anchors, selected structure, and launch emphasis
- eel-green for healthy live activity and thinking states
- ember for approvals, waiting-for-input, and dangerous actions
- ash / desaturated gray for backgrounded or exited states

### Typography Direction

Use expressive but disciplined typography:

- a strong display face for rare shell titles if available in assets later
- a compact sans for interface text
- monospace for paths, session identifiers, provider keys, state provenance, and technical metrics

Typography should increase technical legibility, not act as decoration.

### Material Direction

Favor:

- layered planes
- quiet gradients
- subtle texture or structural depth
- sharp or moderately rounded corners
- restrained borders

Avoid:

- glassmorphism as the main language
- neon purple gradients
- generic enterprise white cards with random accent colors
- literal serpent art in product chrome

## Alternate Directions

### Obsidian Venom

Useful as a darker alternate.

- stronger bunker feel
- more terminal-adjacent
- higher risk of becoming another dark AI dashboard

Keep as a secondary exploration only.

### Constellation Hydra

Useful as a more abstract alternate.

- cleaner and more strategic
- better for comparative lane layouts
- weaker tactile identity than Lernaean Workbench

Keep as an exploratory branch, not the main recommendation.

## Myth-To-Interface Translation

The Hydra theme should operate at the level of product behavior and structure.

### Multiplicity

Hydra is about many active heads.
In product terms, that becomes:

- multiple concurrent sessions
- grouped sessions by repo or future worktree
- comparison lanes
- branching supervision rather than one-single-focus interaction

### Regeneration

Hydra regenerates.
In product terms, that becomes:

- restore
- reconnect
- rebind
- resume
- rehydrate UI state around live sessions

Motion should communicate reforming and return, not novelty animation.

### Marsh Depth

Hydra belongs to a swamp, not a vacuum.
In product terms, that becomes:

- layered depth planes
- muted atmospheric backgrounds
- non-flat surfaces
- calm low-contrast large areas with selective heat accents

### Cauterization

Heracles defeats the Hydra by cauterizing.
In product terms, that becomes:

- destructive actions should feel deliberate and irreversible
- high-risk permissions should be visually explicit
- dangerous profiles should carry heat and warning semantics, not just a red dot

### Immortal Core

The immortal head maps well to the stable control locus of the app.
For Hydra today, that is closest to:

- the selected repo anchor
- the launch locus
- the shell-level command surface

This argues for a stable, always-legible left-side control structure rather than over-fragmented floating controls.

## Current QML Critique

This section is intentionally specific to the current files.

### App.qml

Current issues:

- the decorative background circles are visually soft and consumer-like rather than operational
- `anchors.margins: 24` and large corner radii make the shell feel more lounge-like than workstation-like
- the two main panels read as two large cards rather than a coherent shell with hierarchy

Architecture-compatible response:

- keep the two-column composition for now
- remove the decorative circles
- replace them with layered background treatment and stronger shell framing
- reduce margins and radius modestly, not aggressively

Do not jump to a permanent three-pane shell yet. The current data model does not justify a fixed diagnostics pane.

### LaunchSidebar.qml

Current issues:

- app identity, launch lane, repo list, and primary action are all competing in one vertical stack
- repo cards are taller than necessary for the current information density
- the selected repo action zone is visually separate from the repo list but semantically tied to it

Architecture-compatible response:

- keep the sidebar as the Phase 1 launch rail
- tighten the section hierarchy
- make repository selection feel primary
- make launch configuration feel like a controlled chamber beneath the current repo selection
- do not move launch into a modal until launch options actually expand

### SessionBoard.qml

Current issues:

- the session board header is oversized relative to the actual data shown
- the metrics strip is readable but visually broad for the amount of information it contains
- the empty state is honest, but the board does not yet convey future comparability or supervision density

Architecture-compatible response:

- keep the board as a single main canvas for now
- tighten the header, metrics strip, and banner spacing
- make the session list denser and more scannable
- reserve grouped lanes and right-side inspectors for later phases when the backing data exists

### RepoCard.qml and SessionCard.qml

Current issues:

- cards are visually pleasant but too tall for a supervision tool
- repo cards over-allocate space to description and path at all times
- session cards need stronger status hierarchy and more technical texture

Architecture-compatible response:

- keep card-based composition for the vertical slice
- reduce card height and padding
- push status indicators higher in the hierarchy
- introduce monospace sub-lines where useful
- do not add fake data for provider, risk, worktree, or timeline until those concepts exist in the view models

## Information Architecture And Shell Structure

### Current-Slice Shell

The current-slice shell should remain:

- left launch/repo rail
- right session board

That is already compatible with the architecture and current view-model surface.

### Near-Term Evolution

The shell should evolve in this order.

#### Phase 1.5

- denser left rail
- stronger shell hierarchy
- better metric chips
- better card density
- tokenized colors and spacing
- purposeful motion

#### Phase 2

- repo intelligence surfaces
- worktree selection and repo-local Hydra state affordances
- optional contextual drawer, but only if repo/worktree/context data exists

#### Phase 3

- provider-aware launch controls driven by typed view models
- provider badges and model lanes in the launch surface
- no provider-specific logic inside QML

#### Phase 4

- activity badges
- approval badges
- event timeline surface
- inspectable state provenance

#### Phase 5

- optional embedded terminal region or handoff surface
- terminal focus and attention states
- no change to the rule that status truth comes from domain/infrastructure, not the terminal widget

## Component System

Recommended first-pass component family for the `ui` layer:

- `HydraTheme` singleton: semantic colors, spacing, radii, typography sizes, motion durations
- `ShellFrame`: app-level background, depth planes, and primary panel framing
- `RailSectionHeader`: compact section title for the left rail
- `MetricChip`: reusable selected-repo / repo-count / session-count / tmux-ready chips
- `PrimaryLaunchButton`: current main launch action surface
- `StateBadge`: reusable badge for session state chips
- `StatusBanner`: banner for user-facing status and launch/refresh messages
- `EmptyStatePanel`: reusable empty-state treatment for the session board
- `RepoCard`: denser version of current repo card
- `SessionCard`: denser, more state-forward session card

These are all UI concerns and fit the existing architecture.

Deferred component family:

- provider-specific launch controls
- risk badges
- worktree badges
- timeline blocks
- terminal tabs and pane chrome

Those should wait until the backing concepts exist in `domain` and `infrastructure`.

## State Semantics

The state system already exists in `domain::SessionState`, even though only a narrow subset is driven today.
That makes it reasonable to design visual semantics now, as long as implementation remains phase-aware.

| State | Visual treatment | Motion treatment | Emphasis | Implementation note |
| --- | --- | --- | --- | --- |
| `Starting` | bronze edge, warm lift, low-opacity fill | short rise/fade-in | medium | safe to design now; backend support grows later |
| `Idle` | stable graphite/bone balance, quiet badge | none or minimal shimmer | low | fully relevant now |
| `Thinking` | eel-green highlight, alive but calm | slow breathing pulse | medium-high | design now, wire later via structured status |
| `RunningTool` | stronger green/teal edge with sharper contrast | short repeating pulse | high | future Phase 4 signal |
| `AwaitingApproval` | ember badge and warning edge | steady alert pulse | critical | future Phase 4 plus permission governance |
| `WaitingForInput` | ember outline with warmer fill than approval | slower pulse than approval | high | future Phase 4 signal |
| `Backgrounded` | desaturated, flattened contrast | fade-down only | low | future once background semantics exist |
| `Exited` | ash / cooled bronze | none | low | relevant now |
| `Error` | ember-to-red emphasis with clear iconography | one-time arrival emphasis, then static | critical | relevant now |

Design rule:

- color is never the only signal
- every state should also have label, weight, and eventually iconography
- QML animations should be tokenized, not hardcoded ad hoc in each component

## Design Tokens

Create a QML token system before redesigning multiple components.

### Color Tokens

Suggested semantic set:

- `shellBg`
- `shellDepthA`
- `shellDepthB`
- `panelBg`
- `panelBorder`
- `panelMuted`
- `textPrimary`
- `textSecondary`
- `textMuted`
- `accentBronze`
- `accentLive`
- `accentThinking`
- `accentAttention`
- `accentError`
- `accentExited`

### Spacing Tokens

Suggested set:

- `space4`
- `space8`
- `space12`
- `space16`
- `space20`
- `space24`

Current shell should likely settle around `16` outer margin, not `24`, and use `12` to `16` inside cards.

### Radius Tokens

Suggested set:

- `radius8`
- `radius12`
- `radius16`
- `radius20`

Hydra should not lean on oversized pill shapes everywhere.
Use moderate radii and stronger edges.

### Typography Tokens

Suggested set:

- `displayTitle`
- `panelTitle`
- `sectionLabel`
- `body`
- `bodySmall`
- `monoSmall`

### Motion Tokens

Suggested set:

- `motionFast`
- `motionNormal`
- `motionSlow`
- `easeStandard`
- `easeEmphasized`
- `easeFade`

## Motion System

Motion should be purposeful and sparse.

### Appropriate motion now

Safe for the current shell:

- sidebar selection transitions
- session card insertion fade/slide
- status banner appearance
- refresh feedback
- hover/focus emphasis

### Appropriate motion later

Wait for richer state and data:

- restore/rebind reforming animation
- approval pulses
- thinking pulses tied to real activity
- timeline transition language
- terminal attention states

### Motion character

Preferred character for Hydra:

- short fades
- subtle lateral or vertical shifts
- quiet breathing emphasis for live states
- no springy bounce
- no ornamental looping animations on idle surfaces

### Reduced motion

The architecture does not expose settings yet, so reduced motion should be designed now but implemented later with a centralized token or settings source.

## Responsive Rules

### Wide desktop

- keep left rail visible
- keep session board as the dominant canvas
- allow denser session-card layouts later when grouping exists

### Standard laptop

- keep left rail visible but tighter
- reduce outer margins
- reduce title scale
- keep session board single-canvas

### Narrower width

- allow left rail to collapse or become a temporary drawer later
- do not add bottom navigation while the app still behaves like a workstation shell
- if collapse is implemented before feature growth, keep it minimal and reversible

Immediate rule for current QML:

- design for shrinkage through density, not through control proliferation

## Accessibility And Legibility

Hydra should aim for workstation clarity, not atmospheric murk.

Key rules:

- strong focus visibility
- state not conveyed by color alone
- readable contrast in both shell and board surfaces
- technical details use monospace where scanning benefits
- banners and action states remain readable under parallel load
- animation never masks operational truth

This is especially important once approval/risk states arrive.

## Immediate Implementable Moves

These are the practical moves that fit the existing architecture and current files.

1. Create `qml/Hydra/styles/HydraTheme.qml` and move repeated colors, spacing, and radii out of inline literals.
2. Update [App.qml](/home/lycaon/Documents/Dev/Tools/HYDRA%20V2/qml/Hydra/App.qml) to remove decorative circles, strengthen background layering, and reduce outer margin from `24` to a denser shell value.
3. Update [LaunchSidebar.qml](/home/lycaon/Documents/Dev/Tools/HYDRA%20V2/qml/Hydra/components/LaunchSidebar.qml) so repo selection visually dominates, launch state becomes a clearer secondary chamber, and section hierarchy tightens.
4. Update [RepoCard.qml](/home/lycaon/Documents/Dev/Tools/HYDRA%20V2/qml/Hydra/components/RepoCard.qml) to reduce height, tighten path/description treatment, and improve technical scannability.
5. Update [SessionBoard.qml](/home/lycaon/Documents/Dev/Tools/HYDRA%20V2/qml/Hydra/components/SessionBoard.qml) to reduce title/summary bulk and make the board feel more like an orchestration canvas than a marketing panel.
6. Update [SessionCard.qml](/home/lycaon/Documents/Dev/Tools/HYDRA%20V2/qml/Hydra/components/SessionCard.qml) to make state hierarchy stronger and visual density higher without inventing new data fields.
7. Keep all state truth sourced from [session_state.hpp](/home/lycaon/Documents/Dev/Tools/HYDRA%20V2/src/domain/models/session_state.hpp) and the existing view-model bridge rather than hardcoding business state in QML.
8. Do not add provider badges, risk badges, worktree chrome, or timeline blocks yet unless matching data lands in the view models first.

## Phase-Aware Implementation Plan

### Phase 1: Make the existing shell honest and strong

- theme/token singleton
- denser layout
- stronger left-rail hierarchy
- stronger session-card hierarchy
- subtle shell motion
- better focus and hover states

### Phase 2: Add repo intelligence surfaces

- `.hydra/` and worktree-aware launch affordances
- repo-local docs/context presentation
- optional contextual repo drawer if there is meaningful data to show

### Phase 3: Add provider-aware launch surfaces

- provider selection and status from typed models
- model lane and auth lane controls from typed models
- provider brand language only where it reflects real adapter capability

### Phase 4: Add observability surfaces

- timeline and provenance surfaces
- state badges backed by structured status
- approval/attention patterns grounded in real signals

### Phase 5: Add terminal surface without collapsing the architecture

- embedded terminal remains a rendering/input concern
- terminal chrome complements supervision UI rather than replacing it
- session board remains a first-class orchestration surface

### Phase 6: Hardening and workstation polish

- command palette
- keyboard-first travel
- permission-profile governance
- audit export and recovery surfaces

## Open Risks And Unknowns

- The desktop target still cannot be built in the current machine state because Qt declarative development components are missing.
- The eventual density of the session board depends on how grouped sessions, provider badges, and timelines are shaped in the domain/view-model layer.
- Motion choices for `Thinking`, `RunningTool`, and approval states should stay provisional until structured signals exist.
- The eventual terminal surface may affect shell balance, but it should not back-drive current shell design.

## Source Register

Repository grounding:

- `HANDOFF.md`
- `README.md`
- `docs/architecture/system-architecture.md`
- `docs/architecture/provider-adapter-contract.md`
- `docs/planning/implementation-phases.md`
- `docs/recommendations/creative-design-review-2026-03-06.md`
- `qml/Hydra/App.qml`
- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/components/RepoCard.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionCard.qml`
- `src/domain/models/session_state.hpp`
- `src/domain/models/session_state.cpp`

External references:

- Qt Quick Controls customization: https://doc.qt.io/qt-6/qtquickcontrols-customize.html
- Qt Quick performance: https://doc.qt.io/qt-6/qtquick-performance.html
- GNOME HIG adaptive design: https://developer.gnome.org/hig/guidelines/adaptive.html
- Apple Human Interface Guidelines: https://developer.apple.com/design/human-interface-guidelines/
- Britannica, Hydra: https://www.britannica.com/topic/Hydra-Greek-mythology
- Theoi, Lernaean Hydra: https://www.theoi.com/Ther/DrakonHydra.html
- Acropolis Museum, Heracles and the Lernaean Hydra: https://www.theacropolismuseum.gr/en/heracles-and-lernaean-hydra

Inference notes:

- `Lernaean Workbench` as the primary direction is an inference from the repo's product identity plus the mythology references.
- The exact shell-density recommendations are design inference constrained by the existing QML files and the architecture documents.
- The deferred handling of provider/risk/worktree/timeline chrome is an inference from current code reality and planned phase boundaries.
