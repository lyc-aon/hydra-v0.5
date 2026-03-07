# Deep Visual Pass

Last updated: 2026-03-06

## Scope

This pass closed the two confirmed Phase 1 / 2 follow-up issues and pushed the shell further toward the measured Hydra reference set.

Closed:

- transient refresh-banner lifecycle
- exact palette retuning from sampled inspiration colors
- stronger instrument-panel composition across rail, board, cards, and controls

## Measured palette basis

Representative colors taken directly from the local inspiration images:

- `Oldschool Steam.jpg`
  - `#0B0C0A`
  - `#3D4537`
  - `#515C49`
  - `#A6997A`
- `Hermes.jpeg`
  - `#4A6998`
  - `#6A8CB7`
  - `#DADAE1`
- `Hermes3.jpeg`
  - `#94CFDD`
- `NGE.webp`
  - `#B9943F`
  - `#4A9664`
- `NGE2.webp`
  - `#D8120C`

These samples now directly inform the default QML tokens in `qml/Hydra/styles/HydraTheme.qml`.

## What changed in the shell

### 1. Transient status policy

- `Refresh` now emits a timed confirmation instead of a sticky banner.
- The Wayland GUI validator now proves the message appears and clears.

### 2. Stronger control-room masthead and zoning

- the rail now opens with a real console masthead, sampled-color chip strip, and harder section dividers
- panels use tighter top bands and instrument-line treatment rather than generic card blocks
- board headers now read as a channel register instead of a generic dashboard title

### 3. Sharper component language

- repo and worktree cards are denser and visually closer to list rows on a workstation surface
- metric tiles now behave like segmented readouts instead of soft web cards
- session rows have clearer state striping and bottom instrumentation bands
- selected repo/worktree states now pick up accent-color fill instead of only a border change

### 4. Better interactive hierarchy

- worktree creation and launch controls now read as deliberate shell controls rather than default dark-mode buttons
- divider, refresh, create, end, and launch controls still keep press-state animation, but with stronger material separation

## External references consulted

Qt and desktop references used for this pass:

- Qt Quick Controls customization
  - https://doc.qt.io/qt-6/qtquickcontrols-customize.html
- Qt Quick `MultiEffect`
  - https://doc.qt.io/qt-6/qml-qtquick-effects-multieffect.html
- Qt Quick Shapes module
  - https://doc.qt.io/qt-6/qtquick-shapes-qmlmodule.html
- Qt Quick performance guidance
  - https://doc.qt.io/qt-6/qtquick-performance.html
- KDE NeoChat repository
  - https://invent.kde.org/network/neochat
- KDE Plasma System Monitor repository
  - https://invent.kde.org/plasma/plasma-systemmonitor

These were used as guidance for:

- keeping the shell Qt-native and lightweight
- preferring crisp panel framing over decorative bloat
- keeping effects subtle and targeted
- biasing toward desktop-tool density rather than web-dashboard spacing

## Current judgment

The shell is now materially beyond the earlier “just a color shift” stage.

What is now visibly different:

- measured Steam/Hermes/NGE palette instead of approximate recoloring
- stronger masthead and control-rack identity
- denser list/readout treatment
- clearer selected states
- stronger launch/control hierarchy
- real transient refresh behavior

Still intentionally deferred:

- full emissive glow system
- hex-specific branding motifs
- richer status/alert vocabulary tied to future provider signals
- terminal-surface styling, because the terminal surface itself does not exist yet

Remaining polish from the final Gemini review:

- stronger typographic identity on the largest headers and action surfaces
- more integrated status indicators than simple bracketed tags
- a restrained emissive bloom pass on key headers and active readouts
- a future accent-color rule for truly critical states rather than general shell use
