# Shell V2 Rewrite Plan

Last updated: 2026-03-06

## Execution Status

- Phase A: `Complete`
- Phase B: `Complete`
- Phase C: `Complete`
- Phase D: `Complete`
- Phase E: `Complete`

Latest checkpoint: `docs/validation/shell-v2-progress-2026-03-06.md`

Follow-up note:

- the Shell V2 rewrite itself is complete
- the deeper post-rewrite refinement pass is now tracked separately in `docs/planning/phase-1-2-followup-refinement-2026-03-06.md`
- its outcome is recorded in `docs/recommendations/deep-visual-pass-2026-03-06.md`

## Goal

Rewrite the current Hydra shell before Phase 3 provider work so the product does not accumulate more behavior on top of a visually weak and structurally clunky interface.

This is not a product reset.

Keep:

- the Phase 1 / 2 architecture
- the existing repo/worktree/session model boundaries
- the QML shell approach
- the detached `tmux` orchestration flow

Rewrite:

- the visual system
- shell composition
- information density
- responsiveness
- interaction styling
- motion language

## Current diagnosis

Functionally, Hydra is where the roadmap says it is: Phase 1, 1.5, and 2 are real and locally validated.

Visually, the current shell is not yet the intended product direction.

Direct local review of the generated screenshots and the Gemini image critique converge on the same problems:

1. the UI reads as a generic dashboard rather than a specialized orchestration workbench
2. the main board wastes too much empty space
3. the left rail is still a long stack of similar-weight cards instead of a sharper operational surface
4. responsiveness is weak; the tight capture mostly compresses the wide composition instead of reflowing it
5. the beige/orange treatment feels dated and too soft for the target mood
6. corner radii are overly soft and contribute to a bubbly feel
7. session presentation is too sparse and card-like, not enough like a system console or active roster
8. the current shell has almost no visual sense of stateful motion, readiness, or controlled energy

Bottom line:

- the current shell is acceptable as a validated prototype shell
- it is not acceptable as the visual base for Phases 3 to 5

## Target direction

The target is a native desktop workbench with:

- a `nu-oldtech` identity rather than a web-dashboard identity
- a darker, harder, more technical shell
- matrix/phosphor undertones used with restraint
- Evangelion-style control-room discipline in grid, type, labeling, and density
- subtle mythic Hydra undertones through color, naming, recurrence, and multi-lane structure
- modern Qt-native responsiveness and motion, not nostalgia cosplay

The correct tone is:

- sharp, not bubbly
- dense, not cramped
- animated, not busy
- technical, not sterile
- mythic, not fantasy-themed

## Visual system direction

### Color

Move away from the current beige-heavy board treatment.

Proposed direction:

- shell black: `#090b0d`
- shell graphite: `#11161b`
- panel graphite: `#151c22`
- line dim: `#24303a`
- primary text: `#d7e0e7`
- secondary text: `#7f919f`
- phosphor green: `#7cff9b`
- deep phosphor green: `#1f8f57`
- signal orange: `#ff6a2b`
- bronze tertiary: `#8e5a2a`
- warning ember: `#ff9348`

Rules:

- phosphor green is for health, readiness, live state, and key metrics
- signal orange is for launch, selection, active focus, and destructive attention
- bronze is tertiary only; it should stop dominating the shell
- large beige surfaces should be removed or reduced drastically

### Typography

Use a mixed system instead of one generic family.

Locally available candidates already present on this machine:

- `JetBrainsMono Nerd Font` for paths, IDs, metrics, timestamps, structured labels
- `Roboto Condensed` for major headings, section titles, rail labels, actions
- `Noto Sans` as the fallback general UI family

Rules:

- monospace for technical content and status metadata
- condensed sans for hierarchy and operative labeling
- tighter tracking and clearer case rules on section headers
- reduce verbose explanatory copy in the main board

### Geometry

Change the shape language.

Rules:

- main panels should use smaller radii or nearly sharp corners
- internal cards/rows should be sharper than outer containers
- borders should do more work than fills
- selection should read as brackets, rails, or edge markers rather than soft filled pills

## Composition changes

### Shell frame

Keep a two-region layout, but make it more disciplined.

- the left rail becomes denser and more compact
- the right surface becomes a structured session ledger/workbench, not a blank canvas with one floating card
- the shell should feel anchored to a grid immediately on first paint

### Left rail

Refactor the current rail into three clearer bands:

1. identity + launch mode
2. repo/worktree targeting
3. launch target confirmation + primary action

Rules:

- shrink descriptive copy
- reduce vertical padding
- make repos/worktrees feel list-like, not stacked marketing cards
- use stronger selection cues
- preserve scrolling, but keep the rail visually tighter

### Session board

The session board needs the largest change.

Rules:

- top summary strip stays, but becomes slimmer and more technical
- empty state aligns to the top-left inside the list region, not the center of a blank field
- live sessions should render as rows or dense slabs in a structured list
- the board background should be subdivided subtly, so it reads like a live system surface
- the first live session should no longer look isolated in a sea of empty neutral color

## Responsive rules

The current tight state is not a designed breakpoint.

Shell V2 needs explicit behavior at least for:

- `>= 1440px`
- `1180px to 1439px`
- `960px to 1179px`

Rules:

- rail width should step down across breakpoints
- metric tiles should reflow instead of just compressing text
- session rows should truncate and wrap intentionally
- overly long paths should fade or elide cleanly
- if needed, the rail can collapse secondary copy before collapsing structure

No mobile fiction should be added yet. This is still a desktop workbench.

## Motion rules

Hydra should feel alive, but controlled.

Rules:

- no perpetual decorative animation
- motion should communicate state entry, refresh, selection, and launch response
- durations should stay short: roughly `80ms` to `180ms`
- use opacity and positional easing sparingly
- active-state pulses should be subtle and rare
- avoid glow spam, blur spam, or exaggerated neon effects

## Rewrite phases

### Phase A: Theme V2 Tokens

Deliverables:

- new color tokens
- new typography tokens
- spacing/radius normalization
- explicit component states for idle/hover/selected/active/error

Gate:

- app builds
- capture script still works
- visual captures show the new palette consistently across wide/tight states

### Phase B: Shell Composition

Deliverables:

- rail hierarchy redesign
- board summary strip redesign
- top-left anchored empty state
- better use of vertical space in the right surface

Gate:

- `scripts/capture_ui_screenshots.py`
- `scripts/review_ui_with_gemini.py`
- `scripts/validate_wayland_ui.py`

### Phase C: Component System Rewrite

Deliverables:

- repo/worktree cards become sharper and denser
- session card becomes a structured session row or denser slab
- action styling becomes text-forward and technical, not generic pill-button UI
- selected state becomes edge-led and unmistakable

Gate:

- visual captures show a clear reduction in clunkiness
- no AT-SPI names or core button actions regress

### Phase D: Responsive Pass

Deliverables:

- explicit breakpoint handling
- no clipped rail sections in tight mode
- tighter path truncation and metric behavior

Gate:

- wide and tight captures both look intentional
- geometry checks in `validate_wayland_ui.py` still pass

Result:

- completed on 2026-03-06
- summary metrics now resolve through a deterministic grid
- explicit compact states are captured at `1040x720` and `960x700`
- the Wayland validator now confirms key controls remain visible in a compact desktop window

### Phase E: Motion and Polish

Deliverables:

- restrained state transitions
- launch feedback that feels immediate
- cleaner hover/focus behavior
- remove any remaining prototype styling artifacts

Gate:

- no performance regressions obvious in local testing
- animations remain secondary to readability and control

Result:

- completed on 2026-03-06
- perpetual motion was reduced and centralized behind `HydraTheme.ambientEnabled`
- the launch sweep is now an event cue instead of constant ambient noise
- session-list add/remove/displaced transitions are in place
- bracketed frame accents and tighter standby/readiness presentation complete the current Shell V2 pass

## Required test loop between iterations

Every significant UI pass must run this exact loop:

1. `cmake --build build/debug --target hydra -j2`
2. `python3 scripts/capture_ui_screenshots.py`
3. inspect `.runtime/ui-captures/current/contact-sheet.png` or the individual captures
4. `python3 scripts/review_ui_with_gemini.py`
5. `python3 scripts/validate_wayland_ui.py`
6. update docs only after both the visual and functional paths still hold

If the visual result improves but the functional validator regresses, the change is not complete.

## Current recommendation

Shell V2 is complete enough to stop being the blocker in front of Phase 3.

The next UI-heavy work should be tied to real product surfaces:

- provider adapters
- richer session/status ingestion
- eventual terminal-surface integration

Do not restart the shell again before those surfaces exist unless a concrete usability regression appears.

Post-completion refinement on 2026-03-06:

- removed roadmap/meta copy from the visible shell
- added a validated collapsible navigation rail and full-pane board mode
- extended the capture and Wayland validation loops to cover the collapsed shell state explicitly

Active follow-up refinement scope:

- move the rail toggle onto the pane divider instead of leaving it in the board header
- make the rail width user-adjustable with drag interaction
- add animated collapse/expand behavior rather than instant jumps
- add clearer press feedback on primary shell controls
- audit session-card action containment and eliminate bottom-edge clipping

Completion note:

- completed on 2026-03-06
- divider-mounted toggle is now real
- rail width now supports direct drag resizing
- shell controls now have press-state motion
- session-card action clipping was fixed by making row height content-driven
- rail collapse/expand now runs from a single reveal value so the rail, divider, and board stay synchronized
- the sidebar no longer recomputes compact/dense/tight layout breakpoints frame-by-frame while being collapsed
- validation now uses three lanes:
  - deterministic offscreen screenshots
  - Gemini image critique
  - live semantic Wayland validation plus focused Xwayland pointer-drag validation

Post-completion follow-up now active:

- use the local `Image Inspiration/` reference set to refine the default shell palette and surface language
- bias the default theme toward old-school Steam plus Hermes/NGE control-room cues
- keep this refinement inside the current Phase 1 / 2 product boundaries rather than inventing future chrome

Follow-up result on 2026-03-06:

- completed a Hermes / Steam visual-direction pass
- analyzed the local reference set image-by-image through the Gemini CLI OAuth path
- produced a synthesis note and applied a palette / geometry refinement to the current shell
- validated the result with fresh captures plus Wayland and X11 GUI automation

## Research references

Technical references:

- Qt Quick Controls customization: https://doc.qt.io/qt-6/qtquickcontrols-customize.html
- Qt Quick performance guidance: https://doc.qt.io/qt-6/qtquick-performance.html
- GNOME adaptive layout guidance: https://developer.gnome.org/hig/guidelines/adaptive.html

Aesthetic and thematic references:

- Pedro Fleming, Evangelion screen graphics: https://www.pedrofleming.com/neongenesisevangelion
- Theoi, Lernaean Hydra: https://www.theoi.com/Ther/DrakonHydra.html
- Britannica, Hydra: https://www.britannica.com/topic/Hydra-Greek-mythology
