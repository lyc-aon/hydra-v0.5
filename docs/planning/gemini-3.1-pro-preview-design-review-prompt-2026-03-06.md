# Gemini 3.1 Pro Preview Prompt For Hydra V2 Design Review

Date: 2026-03-06
Author: Codex

## Runtime assumption

This prompt is intended for a Gemini execution path with full repository inspection capability and permission to write files through the local agent harness.

Assume:

- you can read any file in this repository
- you can web-research freely
- you can write markdown recommendations back into this repository
- you are not limited to the legacy read-only advisory role of older `gemini_prayer` wrappers

For this pass, do not edit product code yet.
Write a recommendations document only.

## Objective

Review the actual Hydra V2 repository, research current implementation-relevant guidance plus broader creative references, and write a high-fidelity design review / creative design plan for the application.

Your output should help the team design a native, responsive, animated, dynamic desktop shell that fits Hydra's architecture and mythological identity without collapsing into generic cyberpunk or generic enterprise dashboard tropes.

## Deliverable

Write a new file at:

`docs/recommendations/gemini-hydra-design-review-and-plan-2026-03-06.md`

If that filename already exists, overwrite it.

## Repository context you must review first

Read these files before making recommendations:

### Handoff and top-level context

- `HANDOFF.md`
- `README.md`

### Architecture and planning

- `docs/architecture/project-definition.md`
- `docs/architecture/system-architecture.md`
- `docs/architecture/provider-adapter-contract.md`
- `docs/architecture/repo-storage-and-worktrees.md`
- `docs/planning/implementation-phases.md`
- `docs/planning/repository-tree-proposal.md`
- `docs/planning/implementation-start-prompt.md`
- `docs/validation/source-validated-implementation-plan-2026-03-06.md`

### UI and runtime files

- `qml/Hydra/App.qml`
- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/components/RepoCard.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionCard.qml`
- `src/app/hydra_application.cpp`
- `src/domain/services/session_supervisor.cpp`
- `src/infrastructure/mux/tmux_adapter.cpp`
- `src/infrastructure/persistence/database_manager.cpp`
- `src/ui/viewmodels/app_state.hpp`
- `src/ui/viewmodels/app_state.cpp`
- `src/ui/viewmodels/repo_list_model.cpp`
- `src/ui/viewmodels/session_list_model.cpp`
- `tests/CMakeLists.txt`

## What you should infer from the codebase

Be explicit about the difference between:

- what exists in architecture docs
- what exists in the code right now
- what is safe to design now
- what should remain a future blueprint

The repository is not at the final product stage.
It is at an early but real native-shell vertical slice.

## Research requirements

Perform broad web research before writing recommendations.
Use current sources.
Separate confirmed source-backed guidance from your own design inference.

### 1. Official implementation guidance

Research official or primary sources for:

- Qt Quick Controls customization
- Qt Quick performance and animation discipline
- desktop-native layout/adaptive guidance
- accessibility and reduced-motion guidance
- any current Qt/QML patterns that materially affect a responsive workstation shell

### 2. Product and interaction reference hunting

Research a broad range of design references relevant to Hydra's problem space, such as:

- desktop command/workstation interfaces
- command centers / ops dashboards
- trading terminals
- digital audio workstations
- IDE or devtool sidebars + canvases + inspectors
- multi-session supervision surfaces
- terminal-adjacent design systems
- animated productivity shells

You are not limited to official docs for this part. Use judgment and pull in diverse visual references.

### 3. Mythology and symbolic direction

Because the product is named Hydra, research mythological and visual references, including:

- the Lernaean Hydra in primary or reputable secondary sources
- museum or academic references for Hydra depictions
- visual motifs related to multiplicity, regeneration, heads/branches/lanes, marsh/water, bronze, stone, and heroic combat
- abstract ways to encode these motifs into software UI without becoming literal fantasy illustration

### 4. Competitive and adjacent product language

Research how modern premium desktop tools communicate:

- danger / permission scope
- active versus idle status
- background processes
- focus and attention
- comparison between multiple parallel tasks
- density without chaos

## Design goals

Your recommendations must fit these goals:

- native-feeling macOS and Linux desktop UX
- fast, stable, premium workstation feel
- high-fidelity but implementable in Qt Quick/QML
- responsive across large desktop, laptop, and narrower widths
- animated, but disciplined and performance-aware
- explicit handling of dangerous permissions and approval policies
- clear session supervision and restore semantics
- future compatibility with provider adapters, worktrees, event timelines, and embedded terminal surfaces

## Explicit anti-goals

Avoid recommending a design that feels like:

- a web dashboard pasted into a desktop shell
- an Electron SaaS admin panel
- a stock Material clone
- a purple-gradient AI toy
- a cyberpunk login screen
- a literal fantasy game HUD
- a terminal emulator with a thin sidebar bolted on

## Existing product truths you must respect

Hydra is currently centered on orchestration, persistence, and status.
Do not over-index on terminal rendering.
Respect the architecture boundary where the UI is driven by view models and should not directly own provider logic.

Current validated slice includes:

- repo bootstrap and persistence
- session persistence
- detached `tmux` launch
- basic live/dead refresh against `tmux`
- QML app shell definition
- smoke executable validation

Current missing pieces include:

- real provider adapters
- typed event bus
- worktree flows
- timeline/diagnostics panes
- embedded terminal surface
- meaningful tests
- packaging

## What the output document must contain

Your recommendations document must include these sections.
Use clear headings.

### 1. Executive assessment

State the current project maturity.
State whether the product is ready for a shell-level design pass, a system-level design pass, or a full end-to-end UX pass.

### 2. State of the product today

Summarize the implemented vertical slice based on the real code and handoff.
Do not repeat aspirational architecture as if it already exists.

### 3. Design north star

Define one primary design direction and at least two alternate directions.
Each direction should include:

- concept name
- visual thesis
- emotional tone
- palette direction
- typography direction
- shape/radius strategy
- texture/material ideas
- motion style
- why it fits Hydra
- risks of the direction

### 4. Myth-to-interface translation

Explain how Hydra mythology should influence:

- layout
- hierarchy
- iconography
- motion
- state changes
- destructive actions
- restore/resume flows
- multi-session comparison

Make this sophisticated, not gimmicky.

### 5. Information architecture and shell structure

Recommend the shell layout for:

- wide desktop
- standard laptop
- constrained width

Cover:

- left rail behavior
- main board behavior
- optional right drawer / inspector behavior
- top command strip or not
- command palette role
- where provider, repo, risk, and status controls belong

### 6. Component system

Specify the first-pass component family and their roles, including:

- app shell
- navigation rail
- repo cards
- launch lane controls
- session cards
- session groups
- status chips
- risk badges
- empty states
- toast/banner treatments
- drawers / inspectors
- timeline blocks

### 7. Design tokens

Define a proposed token system for:

- colors
- spacing
- radii
- elevation/depth
- typography scale
- motion durations
- easing families
- border treatments
- semantic states

Make the system Qt/QML-implementable.

### 8. Motion system

Recommend specific motion behavior for:

- startup
- sidebar collapse/expand
- launching a session
- session-card insertion/removal
- state changes
- refresh/rebind
- background activity
- errors
- dangerous-action confirmation
- reduced-motion mode

### 9. Responsive rules

Explain how the shell should adapt as width shrinks.
Be concrete about what collapses, stacks, becomes tabbed, or becomes drawer-based.

### 10. Accessibility and legibility

Cover:

- contrast discipline
- focus visibility
- keyboard-first navigation
- color not being the only state carrier
- reduced motion
- density tradeoffs
- readable status semantics under heavy load

### 11. Phase-aware implementation plan

Map your design recommendations onto the current repo reality.
Break this into:

- what can be implemented now against the current Phase 1 slice
- what should wait until repo/worktree/provider features exist
- what should wait until terminal embedding exists

### 12. Immediate recommendations for the current QML shell

Give a practical critique of the current shell and list the highest-value changes the team should make first.
Be specific.

### 13. Open risks and unknowns

State where design decisions depend on future backend capabilities or actual user testing.

### 14. Source register

List the sources you used with links.
Clearly mark which recommendations are your own inference.

## Quality bar

The output should be:

- concrete
- opinionated
- implementation-aware
- visually ambitious
- not generic
- not bloated
- not afraid to pick a direction

## Final instruction

Write the document directly into the repository at the path specified above.
