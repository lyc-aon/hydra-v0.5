# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/phase-b/phase2-baseline-wide.png`
- `.runtime/ui-captures/phase-b/phase2-live-wide.png`
- `.runtime/ui-captures/phase-b/phase2-live-tight.png`

This is an excellent and thorough request. I will analyze the provided screenshots and deliver the requested critique and design direction.

### 1. Top 10 Problems in the Current UI

1.  **Low Information Density & Wasted Space:** The UI feels sparse, especially in the wide view. The "control-room" aesthetic implies a high volume of data presented efficiently. The large gaps between metrics on the `SESSION BOARD` and the excessive internal padding in the left sidebar underutilize the available screen real estate.
2.  **Weak Visual Hierarchy:** There is no clear focal point. The "HYDRA" title is large but passive. Key status indicators like "MUX READY" and "1 LIVE" are small, low-contrast, and scattered, failing to draw the user's attention to the most critical information. The relationship between the "TARGETING" panel and the "SESSION BOARD" is ambiguous.
3.  **Ineffective & Inconsistent Color Use:** The palette is a monotonous dark teal. The single, high-visibility accent color (orange) is used confusingly for both a primary action (`CREATE`) and a status tag (`GENERIC SHELL`), diluting its semantic value. The `selected` state is a low-contrast color shift that is easily missed.
4.  **Generic and Uninspired Typography:** The UI uses a standard sans-serif font that lacks character and fails to support the "nu-oldtech" aesthetic. A flat typographic scale where all text has similar weight and size makes it difficult to quickly scan and parse information.
5.  **Failure to Evoke the Target Aesthetic:** The design reads as a generic "dark mode SaaS" dashboard. It completely misses the "Evangelion" (dense, angular, technical data), "Matrix" (luminous phosphor glow), and "Mythic Hydra" (subtle thematic motifs) influences. The soft, rounded corners are antithetical to the requested "control-room discipline."
6.  **Poor Responsive Adaptation:** The "tight" view is simply a compressed version of the wide layout, leading to cramped, awkward component scaling. A proper responsive design would reflow, resize, or restructure components (e.g., collapsing the sidebar) to fit the viewport.
7.  **Ambiguous and Passive Language:** Terms like "MUX READY" are jargon without explanation. The "NO SESSIONS ACTIVE" state is passive and empty, whereas a "control room" should feel armed and "Awaiting Input." The "IDLE" status is nearly invisible.
8.  **Underdeveloped Interaction Design:** The user flow from selecting a repository to launching a session is not visually guided. Destructive actions like "End" are presented as small, casual buttons with no confirmation. Key components lack hover states or other visual feedback to indicate interactivity.
9.  **Generic Component Styling:** The cards, buttons, and input fields are basic, unstyled rectangles that feel like wireframe placeholders. They lack the sharp, functional, and slightly brutalist feel of a purpose-built technical interface.
10. **Lack of "Liveness":** For an application managing live processes, the UI is entirely static. There are no pulsing indicators, subtle animations, or visual "heartbeats" to give the user confidence that the system is active and responsive.

### 2. What Already Works and Should Be Preserved

*   **Logical Information Architecture:** The fundamental grouping of concepts into `SYSTEM // LAUNCH`, `TARGETING`, and `SESSION BOARD` is sound. The domain model is clear and provides a solid foundation to build upon.
*   **Two-Panel Layout:** The core concept of a sidebar for selection/configuration and a main panel for active state is a strong, conventional pattern that works well for this type of application.
*   **Minimalist Base:** The current UI is not over-designed. Its simplicity, while currently sterile, is a better starting point than a cluttered or overly decorated interface. It provides a clean canvas.

### 3. Where the UI Misses the Target Aesthetic

*   **"Nu-Oldtech" vs. Sterile SaaS:** It looks like a modern web dashboard. The "old-tech" is missing; it needs the sharp edges, deliberate framing, and character of specialized hardware interfaces.
*   **"Matrix/Phosphor" vs. Dull Teal:** The palette lacks energy and contrast. It needs a deep, dark background to make luminous, glowing accents pop, creating the sense of a CRT monitor in a dark room.
*   **"Evangelion" vs. Soft & Sparse:** It lacks the key Evangelion UI tropes: dense, layered information fields, angular/hexagonal motifs, bold technical typography, and high-impact alert coloring.
*   **"Mythic Hydra":** This subtlest layer is completely absent. There are no thematic hints in iconography, background textures, or component relationships.

### 4. A Concrete Shell V2 Direction

*   **Layout:**
    *   **Density:** Drastically reduce padding and margins. Use thin, 1px dividers or changes in color/texture to delineate regions, not large gaps of empty space.
    *   **Hierarchy:** The `SESSION BOARD` is the primary focus. It should occupy ~70% of the screen width. The `LAUNCH` sidebar should be narrower and more utilitarian.
    *   **Responsive Rules:**
        *   **Wide (>1200px):** Two-panel layout as described above.
        *   **Medium (768px-1200px):** The `LAUNCH` sidebar collapses into a vertical icon-based toolbar. Clicking an icon opens the relevant panel as an overlay.
        *   **Tight (<768px):** The `SESSION BOARD`'s child elements stack vertically.

*   **Color Palette:**
    *   **Background:** `#0D1117` (Deep, desaturated charcoal blue).
    *   **Primary Text/Borders:** `#CDD9E5` (Slightly cool off-white).
    *   **Accent/Glow:** `#39FF14` (A vibrant, luminous "phosphor" green). For active states, selections, and primary interactive elements.
    *   **Warning/Destructive:** `#FF4500` (An intense orange-red). For "End" buttons or error states.

*   **Typography:**
    *   **Headings/Titles:** **Eurostile Extended** or a similar wide, technical font (e.g., `Orbitron` as a free alternative). Use in ALL CAPS for major UI sections.
    *   **Body/Data:** **IBM Plex Mono**. A clean, modern, and highly legible monospace font that feels technical without being cliché.
    *   **Scale:** Establish a clear typographic scale (e.g., Titles: 20pt, Sub-heads: 14pt, Body: 12pt, Metadata: 10pt).

*   **Spacing, Interaction, and Motion:**
    *   **No Rounded Corners:** Use sharp 90-degree angles or 45-degree chamfered corners for all containers to create a "machined" look.
    *   **Framing:** Use thin (1px) `accent/glow` colored inner borders to frame active or selected components instead of fills.
    *   **Interaction:**
        *   **Hover:** Interactive elements should gain a subtle `accent/glow` outline or background glow.
        *   **Selection:** The `selected` item in `Repositories` should be encased in sharp, glowing `[brackets]`.
        *   **Liveness:** The status indicator for a `LIVE` session must pulse. A subtle, slow-repeating scan-line animation across the card would be even better.
    *   **Motion:** All transitions should be sharp and quick. No slow, soft fades. New elements should "flicker" into existence over a few frames.

### 5. Specific Rewrite Directives for QML

1.  **Create `Theme.qml` Singleton:** Define all fonts, colors, and spacing constants (`Theme.headerFont`, `Theme.colorPrimary`, `Theme.spacingLarge`, etc.) in a single, globally accessible file to ensure consistency.

2.  **Rewrite Base Components with Sharp Corners:** Replace `Rectangle`s in `RepoCard`, `SessionCard`, etc., with a custom `FramedBox.qml` component. This component will not use the `radius` property. Instead, it can use a `BorderImage` or custom `Canvas` drawing to achieve sharp, chamfered corners if desired.

3.  **Implement a `Header.qml` Component:**
    ```qml
    // Header.qml
    import QtQuick 2.15
    import QtQuick.Controls 2.15

    Text {
        font.family: Theme.headerFont.name
        font.pointSize: 20
        font.capitalization: Font.AllUppercase
        color: Theme.colorPrimary
    }
    ```

4.  **Style `Button` for Primary/Destructive Actions:** Create a `SystemButton.qml` component with `isPrimary` and `isDestructive` boolean properties.
    *   If `isPrimary`, the button has a solid `Theme.accentGlow` background on hover.
    *   If `isDestructive`, it has a `Theme.warning` border.
    *   By default, it's a ghost button with a `Theme.colorPrimary` border that brightens on hover.

5.  **Animate the `LIVE` Indicator:** In `SessionCard.qml`, add a `Rectangle` for the status dot and apply a looping `SequentialAnimation` to its `opacity`.
    ```qml
    Rectangle {
        id: liveIndicator
        // ... styling
        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 0.7; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 1.0; to: 0.7; duration: 800; easing.type: Easing.InOutQuad }
        }
    }
    ```
6.  **Refactor Main Layout with `State` for Responsiveness:** In your main application QML file, define states based on window width and adjust component visibility and layout properties accordingly.
    ```qml
    states: [
        State {
            name: "tight"
            when: root.width < 768
            PropertyChanges { target: launchSidebar; visible: false }
            PropertyChanges { target: iconToolbar; visible: true }
        },
        State {
            name: "wide"
            when: root.width >= 768
            // ... default properties
        }
    ]
    ```
