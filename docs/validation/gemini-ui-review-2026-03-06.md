# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/current/phase2-baseline-wide.png`
- `.runtime/ui-captures/current/phase2-live-wide.png`
- `.runtime/ui-captures/current/phase2-live-tight.png`

Based on my review of the screenshots and your design goals, here is a comprehensive analysis and a proposed direction for Shell V2.

### 1. Top 10 Problems in the Current UI

1.  **Generic "Dark Mode" Aesthetic:** The UI looks like a standard admin dashboard template. It lacks the unique, opinionated "nu-oldtech" identity you're targeting.
2.  **Inefficient Use of Space:** The "Session board" is mostly empty space, even with an active session. The centered "No sessions yet" and the wide, lonely session card feel unintentional.
3.  **Weak Component Styling:** Components feel borrowed from generic web frameworks (e.g., Bootstrap-style buttons, basic outlined cards). This directly contributes to the "sterile SaaS" feel you want to avoid.
4.  **Tepid Color Palette:** The current charcoal, muted orange, and soft green are too safe. They lack the vibrance and contrast needed for a "phosphor" or "Evangelion control-room" feel.
5.  **Uninspired Typography:** The single sans-serif font is legible but has no character. It doesn't help differentiate between data, labels, and titles, missing an opportunity to build the "technical workbench" identity.
6.  **Poor Responsiveness:** The "tight" view simply squishes the wide layout, causing awkward text wrapping and component scaling. It isn't a deliberate, responsive adaptation.
7.  **Flat Hierarchy in Main Panel:** The Session Card floats in a sea of gray. There is no sense of a grid, log, or structured system, which would be core to a control-room aesthetic.
8.  **Low Information Density:** A workbench or control room implies data is readily available. The current UI feels sparse and airy, prioritizing whitespace over functional density.
9.  **Static Feel:** The UI lacks cues for interaction and state. There's no sense of "motion" or responsiveness, making it feel inert.
10. **Missing Thematic Undertones:** The "Hydra" and "mythic" identity is entirely absent. The visual language is purely functional and generic.

### 2. What Already Works (Preserve These)

*   **Core Layout:** The two-column structure (left sidebar for navigation/state, main area for content) is a strong, intuitive foundation.
*   **Sidebar Information Architecture:** The grouping into `Launch chamber`, `Repo local state`, `Repositories`, and `Worktrees` is logical and clear.
*   **Metric Tiles:** The `Repos`, `Sessions`, and `Mux` tiles at the top of the board are excellent, scannable "at-a-glance" status indicators.
*   **Status Lozenges:** The use of tags like `ready`, `selected`, and `Idle` is effective for communicating state clearly and concisely.

### 3. Where the UI Misses the Target Aesthetic

*   **Control-Room vs. Web Dashboard:** It looks like a web app, not a native desktop tool. A control room is dense, data-first, and utilitarian. This UI is spacious, card-based, and presentation-first. The sharp angles, data grids, and purpose-built feel of an Evangelion UI are missing.
*   **Phosphor/Matrix Undertones:** The aesthetic requires a high-contrast, glowing effect on a deep black background. The current UI uses a soft dark gray with low-contrast text and accents, completely missing the "text on a screen" energy.
*   **Nu-Oldtech vs. Modern Generic:** The "oldtech" is absent. The shapes, fonts, and spacing all conform to modern web design trends rather than drawing from the unique language of classic technical interfaces.

### 4. Concrete Shell V2 Direction

This direction aims for a high-density, high-contrast, text-forward interface that feels like a specialized piece of native software.

*   **Layout:**
    *   Adopt a strict grid system (e.g., 8px) for alignment and spacing.
    *   The left sidebar becomes more compact. Reduce padding within cards.
    *   The "Session board" becomes a top-to-bottom list, not a free-floating space. The "No sessions" message becomes the first item in this list, aligned top-left.
    *   Session "cards" become rows in this list, like entries in a system log or process manager.

*   **Color:**
    *   **Background:** True black (`#000000`) or a near-black with a hint of color (`#0D0D10`).
    *   **Primary Text:** Off-white (`#E0E0E0`), not pure white.
    *   **Accent (Selection/Action):** A vibrant, electric orange (`#FF4F00` or similar), used for selection borders, focused elements, and key action buttons.
    *   **Secondary Accent (Status):** A bright, "phosphor" green (`#00FF41`), used for "ready", "idle", "success" status indicators and key metrics. This should feel like it's glowing.
    *   **Secondary Text/Borders:** A dimmer gray (`#666666`) for non-critical text, paths, and inactive borders.

*   **Typography:**
    *   **Data/Code Font:** A quality monospaced font (e.g., `JetBrains Mono`, `IBM Plex Mono`). Use this for all paths, IDs, metrics, and technical labels.
    *   **UI/Title Font:** A sharp, geometric sans-serif (e.g., `DIN`, `Inter`, `Teko`). Use for titles, section headers, and buttons.
    *   Establish a clear type scale to enforce hierarchy.

*   **Spacing & Interaction:**
    *   **Density:** Tighten spacing significantly. The UI should feel dense but not cramped.
    *   **Shape Language:** Eliminate rounded corners on major panels and cards. Use sharp 90-degree angles. Buttons can have a minimal corner radius (e.g., 2px) or also be sharp.
    *   **Interaction:** On hover, elements gain a sharp, 1px outline in the accent color. No fills. Clicks are instant.
    *   **Motion:** Animate nothing by default. State changes should be instant. If motion is used, it should be for entry/exit and be extremely brief and snappy (e.g., a new session row snaps into place, <100ms).

*   **Responsive Rules:**
    *   Instead of squishing, the UI reflows. At the "tight" breakpoint, the metric tiles could stack vertically or form a single row. Session card content should wrap intelligently or be truncated.
    *   Consider having the left sidebar collapse into an icon-only bar on the narrowest views.

### 5. Specific QML Rewrite Directives

1.  **Create `Theme.qml`:**
    *   Define all colors (`black`, `accentOrange`, `phosphorGreen`, `textPrimary`, `textSecondary`, `borderDim`) and font families/sizes as properties in a central singleton. This is the single source of truth for all styling.

2.  **Refactor Main Layout:**
    *   Use `GridLayout` for the main screen structure.
    *   The main "Session board" panel should be a `ListView` or `ColumnLayout` that is always anchored to the top, not centered.
    *   The "No sessions" message is just a component delegate displayed when the session list model is empty. Style it like a terminal message (`monospace` font).

3.  **Rewrite Sidebar Cards (`RepoCard`, `WorktreeCard`):**
    *   `Rectangle` with sharp corners (`radius: 0`).
    *   Border should be a `border.color: Theme.borderDim`, `border.width: 1`.
    *   **Selected State:** Change `border.color` to `Theme.accentOrange` and `border.width` to `2`. Do not change the card size. Consider adding a small `Rectangle` element shaped like a bracket `[` on the left edge that only appears when selected.
    *   Use the `monospace` font for paths and the `sans-serif` for the main title.

4.  **Redesign `SessionCard` as `SessionRow`:**
    *   Use a `RowLayout` or `GridLayout` to structure it horizontally.
    *   **Column 1 (Status):** A small `Rectangle` (e.g., 4px wide) whose color is bound to the session state (`Theme.phosphorGreen` for Idle, `Theme.accentOrange` for active, etc.).
    *   **Column 2 (Primary Info):** A `ColumnLayout` with the Session Name (`sans-serif`, bold, `Theme.textPrimary`) and metadata below it (path, shell type, using `monospace` and `Theme.textSecondary`).
    *   **Action Buttons ("End"):** Style them as plain `Text` elements (`sans-serif`, `Theme.accentOrange`). On hover, the text becomes brighter or gets an underline. No background fill.

5.  **Restyle Metric Tiles:**
    *   Keep as `RowLayout` of three `ColumnLayout`s.
    *   For each tile, use the `monospace` font for the number (make it large and `Theme.phosphorGreen`) and the `sans-serif` font for the label below it (`Theme.textSecondary`).
    *   Give them a faint `border.color: Theme.borderDim` to visually ground them.
