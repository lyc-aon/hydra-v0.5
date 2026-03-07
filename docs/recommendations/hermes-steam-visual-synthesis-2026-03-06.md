# Hermes / Steam Visual Synthesis

Last updated: 2026-03-06

Primary inputs:

- `docs/validation/gemini-inspiration-image-analysis-2026-03-06.md`
- `docs/validation/inspiration-palette-extraction-2026-03-06.md`
- `.runtime/ui-captures/sidebar-sync-refine/contact-sheet.png`

## Core synthesis

The reference set converges on a consistent visual system:

- near-black or charcoal foundations
- high-contrast illuminated foreground elements
- strict panel zoning and functional grid logic
- sharp or nearly sharp geometry
- monospaced technical readouts paired with condensed operational headings
- restrained accent usage, with each accent color carrying a specific meaning
- subtle display texture: scanlines, bloom, halation, backlit-film feel
- industrial seriousness rather than consumer-polished softness

The references are not asking for one literal style copy.

They split into three compatible layers:

1. `Oldschool Steam`
- desktop utility layout
- charcoal surfaces
- dense pane-based composition
- power-user information density

2. `Hermes`
- technical monitor vocabulary
- CRT/phosphor logic
- cool holographic blue-gray secondary accents
- industrial instrument-panel seriousness

3. `NGE / Matrix / BeNotAfraid`
- alert discipline
- emissive-on-black drama
- subtle grid / scanline / systems-room atmosphere
- mythic scale and energy without literal iconography

## What Hydra should become

Hydra's default Phase 1 / 2 shell should read as:

- a dark industrial desktop console
- old-Steam-adjacent in layout and utility tone
- Hermes-adjacent in monitor logic and technical severity
- NGE-adjacent in alert framing and zoned hierarchy
- only lightly Matrix-like in display texture
- only lightly mythic in atmosphere

Not:

- anime fan UI
- literal CRT simulator
- skeuomorphic fake hardware replica
- pure green hacker parody
- bubbly SaaS dark mode

## Comparison to current implementation

The current shell is structurally competent, but visually it still misses the reference pack in several ways:

- it is still too orange-led
- the base surfaces are too uniform and too soft
- panel corners are still round enough to dilute the control-room feel
- the current palette lacks the cool blue-gray and olive-charcoal undertones present in the references
- the board and rail feel more like styled cards than like an integrated workstation surface
- the atmosphere is present, but not yet disciplined enough

## Recommended Hydra visual system

### Base surfaces

- move the shell to a darker charcoal / olive-black base
- keep rail and board distinct, but closer in family
- let the board feel like a large instrument surface, not a generic dark container

### Accent semantics

- `amber / brass`: headings, rails, selection edges, primary launch emphasis
- `phosphor green`: live state, ready state, scanline activity, active metrics
- `cool steel / cyan`: secondary technical emphasis, structural highlights, quiet focus cues
- `signal red`: destructive or emergency-only states

### Geometry

- reduce radii further
- rely more on thin borders and edge markers
- keep outer containers slightly softened, but make internal cards/rows sharper

### Texture and atmosphere

- keep scanline logic subtle
- add halation only where it serves hierarchy
- avoid visible fake noise overlays on every surface
- favor backlit-panel feeling over neon glow

### Typography

- keep `Roboto Condensed` for headings and control labels
- keep `JetBrainsMono` for paths, metrics, timestamps, and status text
- bias label styling toward uppercase or high-discipline casing only where it improves hierarchy
- avoid verbose prose inside the shell

## Concrete implementation delta map

Minimal write surface for this pass:

- `qml/Hydra/styles/HydraTheme.qml`
- `qml/Hydra/App.qml`
- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/components/RepoCard.qml`
- `qml/Hydra/components/WorktreeCard.qml`
- `qml/Hydra/components/MetricTile.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionCard.qml`

## Directives for the next implementation step

1. Retune the default palette around charcoal, olive, brass, phosphor green, and cool steel-blue.
2. Reduce global radii so the shell feels more instrument-like.
3. Make board and rail surfaces feel more like contiguous equipment panels and less like separate web cards.
4. Reserve strong orange/amber for true primary-action or selected-edge use.
5. Shift "live" visual energy toward phosphor green instead of orange.
6. Use cool blue-gray sparingly for secondary technical structure and quiet emphasis.
7. Keep scanline and glow effects subtle enough that they still read as serious.
8. Do not introduce literal hex overload, Matrix rain, or retro bitmap fonts.
