# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/guidance-followup-2/contact-sheet.png`
- `.runtime/ui-captures/refactor-followup-2/contact-sheet.png`

Based on my analysis of the screenshots and your detailed aesthetic goals, here is a comprehensive UI/UX review and a proposed direction for Shell V2.

### 1. Top 10 Problems in the Current UI

1.  **Inconsistent & Uninspired Color Palette:** The primary accent color (a standard tech-blue) feels generic and clashes with the more thematic amber and green accents. It pulls the design toward a sterile SaaS look, away from the desired "nu-oldtech" aesthetic.
2.  **Weak Typographic Hierarchy:** The typography is clean but lacks the disciplined, technical feel of a control room. There isn't a strong, clear distinction between headers, sub-headers, and body text, making the UI harder to scan.
3.  **Overuse of Rounded Corners:** Nearly every component (cards, buttons, panels) uses rounded corners and soft outlines. This is the primary element giving the UI a "friendly web app" feel, directly contradicting the goal of "old technical control-room and dialog-screen discipline."
4.  **Flat, Low-Contrast Surfaces:** The different panels (sidebar, session board) have very similar background colors. This flattens the UI, removing any sense of physical depth or layering between functional areas.
5.  **"Gimmicky" Frame Decorations:** The `FrameCorners` component feels like a superficial decoration. The lines are too thin and detached to feel structural, adding visual noise rather than contributing to a cohesive, blocky aesthetic.
6.  **Generic Component Styling:** Components like `RepoCard` and `StatusChip` are functionally sound but visually indistinct. They resemble default components from a generic library, missing an opportunity to reinforce the unique Hydra identity.
7.  **Ineffective Use of Space:** Margins and padding feel inconsistent. The density is not managed with intent; some areas are crowded while others are too sparse, breaking the rhythm required for a "disciplined" layout.
8.  **Clunky Divider Design:** The draggable dividers are visually heavy. Their thickness and handle design draw unnecessary attention and interrupt the visual flow between panels.
9.  **Lack of a Strong Grid:** Element alignment across different panels feels loose. The UI lacks the crisp, rhythmic precision of a layout built on a strict underlying grid, a hallmark of technical interfaces.
10. **Jarring Header Style:** The `ConsoleHeader` with its solid, bright blue background is visually disconnected from all other components and headers, creating a point of distracting inconsistency.

### 2. What Already Works (Preserve These)

*   **Core Layout:** The three-part structure (icon rail, sidebar, main content) is a robust and conventional pattern for a desktop workbench. It's the right foundation.
*   **Componentization:** The code is clearly organized into discrete, reusable QML components. This makes a stylistic rewrite highly achievable.
*   **Information Architecture:** The logical flow from Repositories to Worktrees to Sessions is clear and intuitive for a developer workflow.
*   **"Session Board" Metaphor:** Representing sessions as cards on a spatial surface is a strong, effective organizing concept.
*   **Subtle Background Texture:** The faint grid on the `ShellBackdrop` is an excellent starting point for the "nu-oldtech" texture and should be enhanced.
*   **Thematic Color Seeds:** The existing amber and green are the perfect starting points for building the "phosphor/matrix" undertone.

### 3. Where the UI Misses the Target Aesthetic

*   **"Nu-oldtech" vs. "SaaS-lite":** The UI currently leans into modern web/SaaS conventions (rounded corners, blue accents, soft-looking components) instead of the sharp, angular, and functional aesthetic of "oldtech" or "control-room" hardware.
*   **"Matrix/Phosphor":** It's a dark theme, but not a *phosphor* theme. It lacks the focus on a limited, high-contrast palette where color is emitted as light (text, icons, glows) rather than used for large surface fills.
*   **"NGE/Control-Room Discipline":** The current design is too relaxed. It lacks the information density, stark typography, and angular/hexagonal motifs of the NGE reference. It feels polite, whereas the target is purposeful and critical.
*   **"Subtle Hydra Undertones":** There is no visual element that suggests the branching, multi-headed nature of a Hydra. This could be represented through connection lines, tree structures, or other subtle visual metaphors.

### 4. Concrete Shell V2 Direction

This direction pivots away from rounded, "friendly" UI and towards a sharp, disciplined, "functional-brutalist" aesthetic with a strong phosphor glow.

*   **Layout:**
    *   **Foundation:** Keep the rail/sidebar/main layout but enforce a strict 8px grid for all component sizing and spacing.
    *   **Shape:** **Eliminate all rounded corners.** Embrace sharp, 90-degree angles. Introduce chamfered corners or hexagonal motifs for key frames and buttons as a nod to the NGE aesthetic.
*   **Color Palette (Phosphor Core):**
    *   **Base:** `#0A0F0A` - A near-black with a subtle hint of green, like a turned-off CRT.
    *   **Surfaces:** Use 2-3 progressively lighter, low-contrast shades for layering.
        *   `#101510` (Main Background)
        *   `#151A15` (Cards, Sidebars)
        *   `#202520` (Pop-ups, Modals)
    *   **Phosphor Accent:** `#FFD900` (Amber/Gold) OR `#33FF33` (CRT Green). **Choose one.** This is for all interactive elements, selected text, and glows. It should feel like light.
    *   **Alert:** `#FF4100` - A stark, desaturated orange-red for critical errors. Use extremely sparingly.
*   **Typography:**
    *   **UI/Headers:** A condensed, technical sans-serif (e.g., **Roboto Condensed**). Use ALL-CAPS for major section headers.
    *   **Data/Code:** A clean, legible monospace font (e.g., **Fira Code** or **Source Code Pro**).
    *   **Hierarchy:** Rely on `font.weight` (Bold, Regular), `text-transform: 'uppercase'`, and the Phosphor accent color.
*   **Interaction & Motion:**
    *   **Feedback:** Interaction feedback must be sharp and immediate. No soft fades. Hovers/selections are shown with a crisp, glowing edge or underline.
    *   **Animation:** All animations should be brief (100-150ms) and use linear or `EaseInOutQuad` easing. Think "snapping" into place, not "drifting."
*   **Responsive Rules:**
    *   **Compact:** Sidebar collapses to the icon rail.
    *   **Narrow:** Session Board switches from a grid layout to a single-column list.

### 5. Specific QML Rewrite Directives

1.  **Create `Theme.qml`:** Define all tokens in a global singleton. Every component MUST reference these.
    ```qml
    // qml/Hydra/styles/Theme.qml
    pragma Singleton
    import QtQuick 2.15
    QtObject {
        readonly property color base: "#0A0F0A"
        readonly property color surface1: "#101510"
        readonly property color surface2: "#151A15"
        readonly property color phosphor: "#FFD900" // Amber example
        readonly property color alert: "#FF4100"
        readonly property int gridUnit: 8
        // ... other fonts, etc.
    }
    ```
2.  **Purge `radius`:** Remove all `radius` properties from `Rectangle`s. The UI must be sharp.
3.  **Refactor Cards (`SessionCard`, `RepoCard`):**
    *   Remove `border`. Use a `Rectangle` with `color: Theme.surface2` on a `color: Theme.surface1` background.
    *   On hover, show a crisp `Rectangle { width: 2; height: parent.height; color: Theme.phosphor }` on the left edge.
    *   Refactor internal text to use the new typographic hierarchy.
4.  **Redesign Headers (`SectionHeader`):**
    *   Use a `Label` with `font.family: "Roboto Condensed"`, `font.pixelSize: 14`, `font.weight: Font.Bold`, `text: "SECTION TITLE".toUpperCase()`, `color: Theme.phosphor`.
    *   Place a 1px `Rectangle` divider below it, stretching the width of the panel.
    *   Abolish the separate blue `ConsoleHeader` design; unify all headers.
5.  **Rebuild Dividers (`DividerHandle`):**
    *   Make the main line a 1px `Rectangle`.
    *   On hover, reveal a larger, more explicit `MouseArea`. The visual handle could be a small stack of three 2px-high rectangles. It should look like a technical grip, not a bubble.
6.  **Implement `PhosphorGlow`:** Use `DropShadow` to create a text/icon glow effect, not a shadow.
    ```qml
    DropShadow {
        anchors.fill: source
        source: source
        horizontalOffset: 0
        verticalOffset: 0
        radius: 8
        samples: 17
        spread: 0.2
        color: Theme.phosphor // Faint glow
    }
    ```
7.  **Replace `FrameCorners`:** Instead of floating lines, integrate the frame. Create a `FramedPanel.qml` component that draws its own border with sharp, inset corner cutouts, making the frame look integral to the component.
8.  **Introduce a `HexagonButton`:** As a replacement for `InfoDotButton`, create a button based on the `Shape` type to introduce a recurring NGE-style motif for high-value actions or info tooltips.
