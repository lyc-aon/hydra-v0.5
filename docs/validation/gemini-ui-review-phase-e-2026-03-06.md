# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/phase-e/phase2-baseline-wide.png`
- `.runtime/ui-captures/phase-e/phase2-live-wide.png`
- `.runtime/ui-captures/phase-e/phase2-live-tight.png`
- `.runtime/ui-captures/phase-e/phase2-live-narrow.png`
- `.runtime/ui-captures/phase-e/phase2-live-compact.png`

Here is a review of the Hydra V2 desktop application UI.

### 1. Top 10 Problems in the Current UI

1.  **Inconsistent Responsive Layout:** The layout breaks significantly across different screen widths, with overlapping elements and awkward resizing. This is the highest impact usability issue.
2.  **Lack of Visual Hierarchy:** Uniform typography makes it difficult to scan and distinguish between headings, labels, and content.
3.  **Monotonous & Low-Contrast Color Palette:** The dark gray-on-gray scheme lacks the "phosphor" energy of the target aesthetic, feeling flat and causing eye strain.
4.  **Inefficient Use of Space:** Large, unbalanced empty areas, especially in the main "Session Board", make the application feel sparse rather than like a dense, information-rich workbench.
5.  **Ambiguous Interaction Cues:** It is not immediately clear which elements are clickable. The "[TGT]" lozenge, for instance, indicates a selected state but doesn't look like an interactive element.
6.  **Generic Component Styling:** Components like buttons and cards are basic rectangles, lacking the "nu-oldtech" character and feeling more like wireframe placeholders.
7.  **Static, Rigid Layout:** The two-column layout feels disconnected. A more integrated, fluid design would better match the "workbench" concept.
8.  **Jargon-Heavy Labeling:** Terms like "System // Launch" and "Launch Vector" are not intuitive and create a learning curve.
9.  **Missed Thematic Undertones:** The design is purely functional, with no graphical elements or motifs that hint at the "Evangelion" or "Mythic Hydra" aesthetic.
10. **No Feedback or Motion:** The UI is static, lacking hover effects, transitions, or micro-interactions that make an application feel responsive and alive.

### 2. What Already Works

*   **Core Layout Concept:** The two-column structure (controls on the left, status on the right) is a strong and intuitive foundation.
*   **Information Grouping:** The logical organization of the left sidebar into `System`, `Targeting`, and `Worktrees` is clear and effective.
*   **Clear Status Indicators:** The "MUX READY" and "LOCAL STATE READY" elements provide an excellent at-a-glance understanding of the system's state.
*   **Unambiguous Primary Call-to-Action:** The "LAUNCH" button is well-placed and its function is immediately obvious.

### 3. Where the UI Misses the Target Aesthetic

*   **"Nu-oldtech" / "Matrix/Phosphor":** The UI feels like a generic "dark mode" application. It lacks the high-contrast, glowing text on a true black background, characteristic of phosphor displays. The typography is a generic sans-serif, not a monospaced or technical font that would evoke a terminal or control screen.
*   **"Evangelion Influence":** This is completely absent. The Evangelion UI is known for its dense, layered information, strong grid lines, diagnostic text, and stark, angular graphics. The current UI is the opposite: sparse, soft, and lacking in graphic tension.
*   **"Mythic Hydra":** There are no thematic elements—no logo, serpentine motifs, or other visual cues—to suggest the Hydra mythos.

### 4. Concrete Shell V2 Direction

*   **Layout:** Adopt a strong, visible grid system using thin, semi-transparent lines to evoke a technical layout screen. The left sidebar should be collapsible to an icon bar, allowing the Session Board to expand. Introduce controlled overlapping of UI elements to create depth and information density.
*   **Color:** Switch to a true black background (`#000000`). Use a "phosphor" green or amber as the primary text color. Reserve the bright orange for critical status indicators (like "LIVE"), selections, and primary action buttons, applying a "glow" effect via shadows.
*   **Typography:** Change the UI font to a high-quality monospaced family (e.g., Fira Code, IBM Plex Mono, JetBrains Mono) to instantly achieve a more technical feel. Establish a clear typographic hierarchy using size, weight, and color brightness.
*   **Spacing & Sizing:** Reduce padding and margins to create a denser, more information-rich interface. Use a consistent 8px grid for spacing. Adopt a more angular design language, using clipped or chamfered corners for components instead of rounded ones.
*   **Interaction & Motion:** Add subtle hover effects (e.g., a slight glow on text or borders). Animate new session cards into view. Use micro-interactions, like a brief "scan line" animation on a button press, to provide feedback.
*   **Responsive Rules:**
    *   **Wide (>1200px):** Full two-column layout.
    *   **Tight (900-1200px):** Collapse sidebar, showing icons only.
    *   **Narrow (<900px):** Switch to a single-column layout, with sidebar sections becoming tabs or an accordion at the top.
    *   **Compact (<600px):** Show only the Session Board, with a hamburger menu to access launch controls.

### 5. Specific QML Rewrite Directives

1.  **Centralize Styling in `HydraTheme.qml`:** Create a `pragma Singleton` QML file to define all theme properties: colors (`backgroundColor: "#05080A"`, `textColor: "#95A1B0"`, `accentColor: "#FF4F00"`), fonts (family, sizes), and spacing units.
2.  **Create `BaseCard.qml`:** Define a base component for all cards. Use a `Rectangle` with a semi-transparent, angular border. Consider using a `Shape` with a custom `ShapePath` to create clipped corners for a more technical look.
3.  **Create a `StyledText.qml` Component:** Replace all `Text` elements with a custom component that takes a style property (e.g., `style: "heading"`, `style: "body"`). This component will apply the correct font, size, and color from `HydraTheme.qml`, ensuring consistency.
4.  **Implement Responsive States with a `StateMachine`:** In your main `App.qml`, use the window's `width` property to drive a state machine (`State { name: "wide"; when: root.width > 1200 }`, etc.). Use `PropertyChanges` within each state to alter layout properties, such as the sidebar's width and visibility.
5.  **Add Hover Effects and Animations:** Use `MouseArea`'s `onHoveredChanged` signal to trigger `ColorAnimation` or `PropertyAnimation` for glow and highlight effects. Use `Behavior` on layout properties or explicit `SequentialAnimation` to animate state changes and the appearance of new items.
