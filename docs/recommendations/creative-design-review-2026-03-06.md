# Hydra V2 Creative Design Review

Date: 2026-03-06
Author: Codex

## Executive judgment

Hydra V2 is ready for a serious shell-level design pass now.

It is not ready for a final end-to-end product UX pass, because the product surface is still only a Phase 1 vertical slice:

- one bootstrap repository
- one generic shell launch path
- one `tmux`-backed session flow
- one refresh action
- basic repo/session persistence
- very shallow session-state normalization

That means the right ask is not "design the whole finished app." The right ask is:

- design the workstation shell
- design the design system and motion language
- design the responsive layout rules
- design the current repo/session board states in high fidelity
- blueprint how that shell expands into provider lanes, worktrees, event timelines, and embedded terminal surfaces later

## Actual project state

### Proven in code

Hydra already has a real native scaffold:

- C++23 + CMake workspace
- Qt 6 core + SQL build path
- conditional Qt Quick/QML desktop target
- SQLite persistence for `repos` and `sessions`
- a working `TmuxAdapter`
- `HydraApplication` bootstrap
- QML-facing `AppState`, `RepoListModel`, and `SessionListModel`
- a simple two-column QML shell
- a working smoke executable

The smoke path still validates successfully with:

```bash
cmake --preset debug-make
cmake --build build/debug-make -j2
./build/debug-make/hydra_core_smoke
```

and reports:

```text
Hydra core smoke initialized. repos=1 sessions=0
```

### Proven but narrow UI surface

The current QML shell is intentionally limited.

Left rail:

- app title
- launch lane status
- repository cards
- selected-repo action area
- "Launch tmux shell"

Main board:

- session board title
- refresh action
- selected repo / repo count / session count summary
- `tmux` readiness chip
- status banner
- empty state or session card list

This is enough to justify a high-fidelity shell/system design pass, because the composition is real.

### Not done yet

The handoff is explicit that Hydra still lacks:

- Claude/Codex/Gemini provider adapters
- repo add/edit/remove UI
- worktree management
- `.hydra/` repo-local bootstrapping
- restore policy beyond basic live `tmux` detection
- heartbeat system
- event timeline
- audit log
- typed event bus
- meaningful tests
- embedded terminal surface
- secrets-store implementation
- packaging

### Current blocker

The desktop target exists in CMake, but this machine still cannot build it because Qt declarative dev components are missing:

- `Qt6::Gui`
- `Qt6::Qml`
- `Qt6::Quick`

That means the design work should be implementation-aware but not blocked on desktop execution in this environment.

## What the design pass should optimize for

### 1. Native workstation feel

Hydra should feel like an operational desktop tool, not a web dashboard trapped in a desktop window.

The QML shell should read as:

- persistent
- heavy-duty
- information-dense without becoming noisy
- fast under multi-session load
- calm when idle
- explicit when risk rises

### 2. A design system that can survive expansion

The current shell is small, but the architecture clearly grows toward:

- multiple providers
- worktrees
- permission/risk profiles
- event timelines
- embedded terminal panes
- grouped sessions
- restore states
- diagnostics and audit views

The design system needs stable rules for:

- panel hierarchy
- focus hierarchy
- color semantics
- badge semantics
- density modes
- animation timing
- empty/loading/error/attention states

### 3. Motion with discipline

Hydra needs motion, but not decorative churn.

Motion should communicate:

- launches
- attachment/reattachment
- background activity
- focus transfer
- session state change
- panel expansion/collapse
- risk escalation

The target is premium workstation behavior, not generic "AI app" theater.

### 4. Expansion paths, not one-off comps

Gemini should not return a single heroic mockup.
It should return a scalable shell structure with rules for:

- desktop wide layouts
- mid-width laptop layouts
- narrow fallback layouts
- card density changes
- lane collapse behavior
- future SplitView / TabBar / timeline integration

## Design constraints from the repo

Any design proposal needs to stay inside these constraints:

- Native stack: C++23 + Qt 6 + Qt Quick/QML
- Current architecture separates `app`, `ui`, `domain`, `infrastructure`, and `platform`
- UI should remain a view-model-driven shell, not a provider-aware control plane
- Terminal rendering is intentionally deferred behind a `TerminalSurface` abstraction
- The current product value is orchestration, persistence, and status, not terminal chrome
- Dangerous permissions and approval policies are first-class product concerns
- The design must be implementable in Qt Quick without a custom scene-graph science project

## Creative direction recommendation

Use a Hydra-specific visual direction instead of generic cyberpunk or generic enterprise SaaS.

Recommended north star: `Lernaean Workbench`

Core idea:

- bronze, bone, wet stone, marsh green, ember, and dark slate instead of neon purple or pure grayscale
- layered, branching, multi-lane composition that suggests many active heads without turning into snake-themed kitsch
- regenerative motion language: subtle pulse, re-form, braid, coil, and converge
- risk and power communicated through heat and venom accents, not random red/yellow alerts everywhere

### Myth translation into product language

Translate the Hydra myth into UI behavior rather than literal monster art.

Useful motifs:

- multiplicity: many concurrent heads -> many concurrent sessions / branches / lanes
- regeneration: session restore, resume, reconnect, rehydrate
- marsh depth: layered backgrounds, low-contrast depth planes, atmospheric gradients
- immortal core head: a stable command center / selected repo anchor / primary launch locus
- cauterized wounds: explicit, deliberate destructive actions with irreversible visual language

Avoid:

- fantasy poster illustration dominating the shell
- cartoon snake heads in product chrome
- over-literal scales or tribal ornament noise
- glowing purple gradients and hacker-movie clichés

## Three viable visual concepts

### 1. Lernaean Workbench

Best overall fit.

Palette:

- chalk / bone surfaces
- oxidized bronze
- eel-green / marsh green
- soot / graphite
- ember orange for high-risk actions

Character:

- premium, ancient-industrial, calm, tactical
- textured depth without visual clutter
- strongest bridge between mythology and workstation utility

### 2. Obsidian Venom

Good alternate if you want a darker product.

Palette:

- obsidian
- green-black
- acid lime accents used sparingly
- copper / brass highlights
- pale ash text

Character:

- more aggressive, more nocturnal, more terminal-adjacent
- useful if Hydra should feel closer to a command bunker
- higher risk of looking too much like another dark "AI ops" dashboard

### 3. Constellation Hydra

Best if you want a more strategic / celestial tone.

Palette:

- deep blue-black
- starlight white
- desaturated brass
- storm gray
- faint aquatic teal

Character:

- more abstract and elegant
- good for layouts that show branching paths, threads, and comparative lanes
- less tactile than the other two concepts

## Structural recommendations for the next design pass

### Shell

Preferred shell evolution:

- collapsible left command rail
- central session board / orchestration canvas
- optional right diagnostics or context drawer
- top-level command strip for search, filters, global state, and quick actions

### Session board

The session board should evolve from a simple list into a flexible orchestration surface with:

- group headers
- provider chips
- repo/worktree badges
- session attention states
- resumability states
- timeline affordances
- density modes for 3, 6, 12+ active sessions

### Launch rail

The launch lane should become the main control surface for:

- repo selection
- provider selection
- model lane
- auth lane
- risk profile
- worktree target
- launch templates

It should be dense, fast, and keyboard-friendly.

### Status semantics

Design the normalized state model visually now, even if the backend is not fully wired yet:

- `Starting`
- `Idle`
- `Thinking`
- `RunningTool`
- `AwaitingApproval`
- `WaitingForInput`
- `Backgrounded`
- `Exited`
- `Error`

The system needs color, motion, icon, and emphasis rules for each.

## Research directions Gemini should explore

### Architecture and implementation reality

Review at minimum:

- `HANDOFF.md`
- `README.md`
- `docs/architecture/*`
- `docs/planning/*`
- `docs/validation/source-validated-implementation-plan-2026-03-06.md`
- `qml/Hydra/*.qml`
- `src/app/hydra_application.cpp`
- `src/domain/services/session_supervisor.cpp`
- `src/ui/viewmodels/*`
- `tests/CMakeLists.txt`

### Official implementation guidance

Use official sources for:

- Qt Quick Controls customization
- Qt Quick performance
- native/adaptive desktop layout guidance
- accessibility and reduced-motion behavior

### Creative/reference research

Search broadly across:

- Hydra myth references
- museum depictions of Heracles and the Hydra
- ancient bronze / wet stone / marsh / serpent visual language
- workstation-grade desktop UIs
- command-center, ops, trading, DAW, and pro tool interfaces
- modern sidebar + canvas + inspector patterns
- motion systems that communicate state without noise

## Source register

Official / primary references already relevant to this direction:

- Qt Quick Controls customization: https://doc.qt.io/qt-6/qtquickcontrols-customize.html
- Qt Quick performance: https://doc.qt.io/qt-6/qtquick-performance.html
- GNOME HIG adaptive design: https://developer.gnome.org/hig/guidelines/adaptive.html
- Apple Human Interface Guidelines: https://developer.apple.com/design/human-interface-guidelines/
- Britannica, Hydra: https://www.britannica.com/topic/Hydra-Greek-mythology
- Theoi, Lernaean Hydra: https://www.theoi.com/Ther/DrakonHydra.html
- Acropolis Museum, Heracles and the Lernaean Hydra: https://www.theacropolismuseum.gr/en/heracles-and-lernaean-hydra

## Bottom line

Send Gemini now, but give it the correct scope.

Ask for:

- a design-system and shell-structure review grounded in the current codebase
- a high-fidelity creative plan for the current Phase 1 slice
- a forward-looking blueprint for Phases 2 through 6
- source-backed rationale for motion, responsiveness, accessibility, and mythology-driven design language

Do not ask it for final pixel-perfect product truth across flows that do not exist yet.
