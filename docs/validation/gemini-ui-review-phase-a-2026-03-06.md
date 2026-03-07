# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/phase-a/phase2-baseline-wide.png`
- `.runtime/ui-captures/phase-a/phase2-live-wide.png`
- `.runtime/ui-captures/phase-a/phase2-live-tight.png`

This is an excellent start. The layout is functional and the core elements are in place. However, it currently reads as a standard "SaaS admin dashboard" and misses the specified "nu-oldtech" Evangelion/control-room aesthetic. The following is a detailed review and a concrete plan to align the UI with your target vision.

### 1. Top 10 Problems in the Current UI

1.  **Generic "SaaS" Styling:** The pervasive use of rounded corners on cards, buttons, tags, and inputs is the biggest deviation from the aesthetic. It feels like a modern web framework's default theme, which is antithetical to the sharp, utilitarian, "old-tech" look.
2.  **Weak & Characterless Typography:** The standard sans-serif font lacks the technical or monospace feel of a control-room UI. The typographic hierarchy is too subtle to efficiently guide the eye.
3.  **Muted Color Palette:** The current dark-bluish theme is too safe. It lacks the dramatic contrast and "phosphor glow" of the reference aesthetic. The orange and green accents are used well for status but the base colors don't support the theme.
4.  **Low Information Density:** The UI is too spacious. A "workbench" or "control-room" implies a higher density of information. The large amounts of padding and margin, especially in the empty "Session board", feel inefficient.
5.  **Flat Materiality:** The design is very flat. It lacks the subtle depth, texture, or layering that could suggest physical panels, CRT glass, or distinct hardware modules.
6.  **Loud Selection State:** The bright, thick orange border for "selected" items is distracting and feels tacked on. A more integrated, subtle indicator would be more effective and in-line with a technical UI.
7.  **Ineffective Responsive Behavior:** The "tight" layout simply squishes and wraps content awkwardly. A truly responsive shell would reflow, collapse, or truncate elements gracefully.
8.  **Inconsistent Component Language:** The visual language for interactive elements is mixed. Pill-shaped buttons, bordered cards, and text labels could be more unified under a single, geometric design system.
9.  **Vague Hierarchy in Sidebar:** The "Launch chamber" and "Repo local state" have the same visual weight as the "Repositories" list items. Their distinct nature (controls vs. data) should be clearer.
10. **Missing "Hydra" Identity:** There are no subtle visual cues—iconography, background watermarks, or naming conventions—that hint at the mythic Hydra theme.

### 2. What Already Works (Preserve These)

*   **Dark Theme:** The foundation is correct. A dark UI is essential for the target aesthetic.
*   **Core Layout:** The two-column structure (sidebar for context, main area for primary content) is logical and functional.
*   **Status Indicators:** Using colored tags (`ready`, `Idle`) for at-a-glance status is perfect for a control-room feel.
*   **Clear Information Grouping:** The separation of concerns into "Repositories," "Worktrees," and "Sessions" is clear and well-organized.

### 3. Where the UI Misses the Target Aesthetic

The UI misses the mark primarily by adopting modern, friendly design trends (soft corners, generous whitespace) instead of the stark, functional, and stylized look of its inspirations.

*   **Evangelion:** This aesthetic is defined by sharp angles, high contrast (often near-black with red/amber), urgent typography (heavy, condensed, or monospace fonts), and a sense of critical information being displayed. The current UI is too relaxed and rounded.
*   **Control Room / Old Tech:** These environments prioritize function and density. Screens are packed with data, and UI elements are geometric (rectangles, lines). The current UI feels too light and decorative.
*   **Phosphor Glow:** The colors feel flat. They lack the luminous, emissive quality of text on a CRT monitor.

### 4. Concrete Shell V2 Direction

This direction uses a "Rectilinear-Phosphor" design system. All elements are derived from sharp rectangles and glowing, high-contrast text.

*   **Layout:**
    *   **Grid:** Enforce a strict, grid-based alignment for all components.
    *   **Density:** Significantly reduce padding and margins. Use a 4px base unit.
    *   **Sidebar:** Make the left sidebar collapsible to a vertical bar of icons or vertical text labels, allowing the "Session board" to take full focus.
    *   **Panels:** Replace bordered cards with "panels" demarcated by fine, 1px hairlines or subtle differences in background color (e.g., `#0A0B0D` vs `#0F1014`).

*   **Color Palette (Evangelion/Phosphor Inspired):**
    *   **Background:** True black/deep charcoal (`#0A0B0D`).
    *   **Panel Background:** A slightly lighter charcoal (`#101114`).
    *   **Primary Accent (Phosphor Amber):** A vibrant, glowing amber/orange (`#FFB800`) for all interactive elements, selections, and highlights. This is your main "glow" color.
    *   **Danger/Stop Accent:** A muted, serious red (`#D95C5C`) for "End" buttons or critical warnings.
    *   **Text (Primary):** A slightly off-white (`#EAEAEA`) to simulate screen text.
    *   **Text (Secondary):** A muted grey (`#888888`) for non-critical descriptions and paths.
    *   **Status Green:** A sharp, digital green (`#33FF57`) for "ready," "idle," "success" text labels.

*   **Typography:**
    *   **Primary Font:** Switch to a proper monospace font. **IBM Plex Mono** or **JetBrains Mono** are excellent choices.
    *   **Headings:** Use uppercase, increased letter spacing, and the accent color for all panel titles (`SESSION BOARD`, `REPOSITORIES`).
    *   **Hierarchy:** Rely on `font-weight`, `color`, and `case` for hierarchy, not just size.

*   **Interaction & Motion:**
    *   **Hover:** No background color changes. Interactive elements' text/icons should brighten to the full accent color. Add a subtle `Glow` effect.
    *   **Selection:** Instead of a border, use a 2px vertical bar of `Primary Accent` color on the left edge of the selected item.
    *   **Motion:** All transitions should be sharp and fast (100ms). Use quick fades or vertical "scanline" wipes, not soft eases. A blinking block cursor on the "No sessions yet" text would be a strong atmospheric touch.

### 5. Specific QML Rewrite Directives

1.  **File: `HydraTheme.qml`**
    *   Replace the entire color scheme with the new palette described above.
    *   Define font properties for `monospaceFont` and `headingFont`.

2.  **Components: `RepoCard.qml`, `SessionCard.qml`, etc.**
    *   **Remove all `radius` properties.** All corners must be sharp (radius: 0).
    *   Remove the `border` property. The background should be the `Panel Background` color.
    *   Create a `selected` state that shows a 2px-wide `Rectangle` on the left edge with the `Primary Accent` color. This replaces the orange border.

3.  **Components: Buttons**
    *   Create a new, site-wide `Button` component. It must be a `Rectangle` with `radius: 0`.
    *   **Default State:** Transparent background, text in `Text (Secondary)` color.
    *   **Hover State:** Text color changes to `Primary Accent`.
    *   **Toggled State (for "tmux"):** Background remains transparent, but text becomes `Primary Accent` and a 2px `Rectangle` "underline" appears, also in the accent color.
    *   The primary "Create" button can have a solid `Primary Accent` background, but with black (`#000`) text to make it stand out.

4.  **Components: Tags (`ready`, `Idle`)**
    *   Rework these completely. They should not be pills.
    *   Make them simple, uppercase `Text` elements. The status is conveyed purely by `color`: `Status Green` for "READY", a warning yellow for "IDLE", etc. No background or border.

5.  **View: `SessionBoard.qml`**
    *   For the "No sessions yet" state:
        *   Use the `monospaceFont`.
        *   Change the text to `> NO SESSIONS ACTIVE_`
        *   Add a `SequentialAnimation` on the trailing underscore character (`_`) to make it blink like a terminal cursor.
        *   Give the entire text a subtle, slow-pulsing `OpacityAnimator` (e.g., from 0.8 to 1.0 over 3 seconds) to feel like a CRT "breathing".

By implementing these changes, the application will shift dramatically from a generic dashboard to a unique, stylized workbench that strongly reflects your desired aesthetic.
