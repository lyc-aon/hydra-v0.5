# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/guidance-notification-pass/contact-sheet.png`
- `.runtime/ui-captures/notification-pass/contact-sheet.png`

I will now analyze the provided UI screenshots and formulate a detailed review based on the target aesthetic and constraints.

### UI Analysis & Recommendations

This review assesses the current Hydra V2 shell against its "nu-oldtech" and "control-room" aesthetic goals. The feedback is structured into the five requested deliverables.

---

#### 1. Top 10 Problems in the Current UI (Ranked by Impact)

1.  **Lack of Cohesive Framing & Hierarchy:** The UI feels like a collection of floating panels on a dark background rather than a unified "workbench" or "console." There's no primary container or "bezel" to ground the layout, which makes the hierarchy between the sidebar, session board, and other panels ambiguous.
2.  **Overly Flat, SaaS-style Components:** Cards, buttons, and panels are borderless and rely on shadows and color fills alone. This aligns with modern web SaaS design (e.g., Material Design) but directly contradicts the "nu-oldtech" goal. Old hardware had bezels, frames, and crisp edges.
3.  **Inconsistent and Generic Iconography:** The icons (e.g., the "plus" button, session icons) are generic, solid-color shapes. They lack the technical, diagrammatic precision of control-room interfaces or the stylized look of the NGE aesthetic. The "info dot" is an exception but feels disconnected from the rest.
4.  **Ambiguous Spacing & Grouping:** The gaps between cards and panels feel arbitrary. This, combined with the lack of borders, makes it hard to discern logical groupings at a glance. The visual grammar for what is a container vs. what is a card vs. what is a button is weak.
5.  **Typography is Too "Friendly":** The current sans-serif font is clean and legible but feels like a standard OS UI font. It lacks the "data" or "terminal" character of a monospaced or more technical-looking typeface that would support the desired aesthetic.
6.  **Color Palette Misses "Phosphor" Nuance:** The primary accent green is a bright, standard "tech" green. It feels more like "gaming hardware" than the subtle, decaying glow of a phosphor CRT screen. The palette needs more muted, low-opacity secondary tones to create depth.
7.  **"Info" Pop-ups are Intrusive:** The large, screen-interrupting help/info modals (seen in the notification pass) are jarring. They break the "workbench" flow. This information should be presented in a less intrusive side panel or integrated contextually.
8.  **Divider Handles are Visually Loud:** The thick, pill-shaped divider handles are chunky and draw too much attention. A control-room aesthetic would use simple, sharp, and functional dividers, perhaps just a thin line with a subtle indicator.
9.  **Lack of Visual "Texture":** The entire UI is composed of flat, solid colors. The "nu-oldtech" aesthetic would be enhanced by subtle textural elements like faint scanlines, a barely-perceptible grid pattern on the background, or inner glows on panels that simulate a CRT screen.
10. **Inconsistent Corner Radius:** Some elements have sharp corners, while others (cards, buttons) have a significant radius. This inconsistency cheapens the look. A committed direction (likely sharp, functional corners) is needed.

---

#### 2. What Already Works and Should Be Preserved

1.  **The Dark Theme Foundation:** A dark, low-contrast base is absolutely the correct starting point for this aesthetic.
2.  **Component-Based Architecture:** The breakdown of the UI into discrete components (`SessionCard`, `RepoCard`, `LaunchSidebar`) is a solid, maintainable approach that will make implementing a new design direction straightforward.
3.  **Clear Information Architecture:** The fundamental layout is logical. Having a launch/repo area on the left and a main stage for active sessions on the right is a classic and effective "workbench" pattern.
4.  **Concept of "Status Chips":** The small status indicators (like "Running") are a good feature. They provide at-a-glance information effectively, reminiscent of status lights on hardware. Their styling needs to change, but the concept is sound.
5.  **Data Density:** The UI is not afraid to show a moderate amount of information, which aligns well with the "control-room" goal. It avoids being overly sparse and minimalist.

---

#### 3. Where the UI Misses the Target Aesthetic

*   **"Nu-oldtech" vs. "Modern SaaS":** The current UI leans heavily on the visual language of modern web dashboards (borderless cards, drop shadows, rounded corners, solid icons). The "old-tech" part of the brief is almost entirely absent. It lacks the satisfying tactile feel of old hardware interfaces, which came from clear frames, bevels, and sharp lines.
*   **"Matrix/Phosphor" vs. "Neon Green":** The aesthetic is currently "neon on black," which is a surface-level interpretation. The "phosphor" quality of a CRT is about a soft, decaying glow and the subtle blooming of light on a dark screen, not just the color green itself.
*   **"NGE/Control-Room" vs. "Friendly Dashboard":** NGE's interfaces used dense, highly structured information displays with sharp vector graphics, monospaced fonts, and a sense of layered, overlapping windows and data streams. The current UI is too polite, spacious, and simple; it lacks the urgency and information density of a command console.
*   **"Mythic Hydra" vs. "Generic Branding":** There is currently nothing in the UI that subtly hints at the Hydra mythos. This could be achieved through iconography, background watermarks, or naming conventions.

---

#### 4. A Concrete Shell V2 Direction

**Theme: "Phosphor Pane"**

This direction treats the entire application window as a single piece of console hardware. The UI is composed of "panes" of glass or phosphor screens layered on top of each other, each with a distinct, sharp frame.

*   **Layout:**
    *   The main window should have a subtle, dark "bezel" effect using a 2-3px border with an inner gradient.
    *   The primary regions (sidebar, main content) should be separated by a simple, crisp 1px divider line. The interactive handle should be a small, sharp-edged rectangle that subtly glows on hover.
    *   Embrace the grid. All elements should align to a strict 4px or 8px grid to enforce a sense of technical precision.

*   **Color:**
    *   **Base:** A near-black (e.g., `#0A0F0A`), not pure black.
    *   **Phosphor Accents:** A family of greens:
        *   `accentPhosphorText`: A legible, slightly muted green for text (e.g., `#7FFF7F`).
        *   `accentPhosphorGlow`: A brighter green for highlights, glows, and active states (e.g., `#A0FFA0`).
        *   `accentPhosphorDim`: A very low-opacity, dark green for borders and background elements (e.g., `rgba(127, 255, 127, 0.1)`).
    *   **Secondary/Status:** Introduce a secondary color for non-primary status indicators, like a muted amber or cyan, to avoid an entirely monochrome look.

*   **Typography:**
    *   **Primary/Data:** A crisp, legible monospaced font (e.g., 'Fira Code', 'IBM Plex Mono', 'Source Code Pro') for all session details, repo names, stats, etc.
    *   **Headers:** A slightly heavier weight of the same monospaced font or a complementary, clean sans-serif for section titles.

*   **Interaction & Motion:**
    *   **Hover:** On cards, don't change the background color. Instead, the 1px border should brighten to `accentPhosphorGlow`.
    *   **Activation:** Clicks should produce a very brief, subtle "flash" or "bloom" effect on the element, mimicking a CRT's response.
    *   **Motion:** Animations should be minimal, sharp, and fast. No bouncy, elastic movements. Think quick fades and positional snaps.

---

#### 5. Specific Rewrite Directives for QML

1.  **Global (`App.qml` / `HydraTheme.qml`):**
    *   Define the new "Phosphor Pane" color palette in `HydraTheme.qml`.
    *   Define the primary and header fonts in the theme file.
    *   Add a root `Rectangle` to `App.qml` to act as the main window "bezel," with a 1-2px border using a dark gradient.
    *   Consider adding a `ShaderEffect` or a near-transparent full-screen `Image` with a scanline texture over the entire background to add subtle texture.

2.  **Cards (`SessionCard.qml`, `RepoCard.qml`):**
    *   Change the root item from a `Rectangle` with a `radius` to one with `radius: 0` and `border { width: 1; color: HydraTheme.colors.accentPhosphorDim }`.
    *   Remove all `shadows`. Depth should come from the border and glow effects.
    *   On `MouseArea` hover (`hoverEnabled: true`), change the `border.color` to `HydraTheme.colors.accentPhosphorGlow`.
    *   Set all `Text` elements inside the card to use `HydraTheme.fonts.monospace`.

3.  **`DividerHandle.qml`:**
    *   Replace the fat, rounded `Rectangle` with a thin `Rectangle` (e.g., `width: 2, height: 32`) with sharp corners.
    *   The divider line itself should be a separate 1px `Rectangle`.
    *   On hover, the handle's color should shift to `accentPhosphorGlow`.

4.  **`LaunchSidebar.qml`:**
    *   Ensure it is framed with its own 1px border on the right side, which acts as the divider.
    *   The "Add New" button should be redesigned. Lose the pill shape. Make it a square button with a sharp, technical "+" icon (e.g., a perfect cross made of 1px lines). The background should be transparent, with only the border and icon visible, lighting up on hover.

5.  **`InfoDotButton.qml` / `DetailHelpPanel.qml`:**
    *   The `InfoDotButton` is a good concept. The icon within it should be a sharper, more technical "?".
    *   **Crucially:** Scrap the modal `DetailHelpPanel`. When the `InfoDotButton` is clicked, the help content should slide in as a temporary right-hand sidebar, pushing the main content to the left. This keeps the user in context. It should be a framed "pane" just like everything else.

6.  **`StatusChip.qml`:**
    *   Make these small rectangles with sharp corners.
    *   The text should use the monospace font.
    *   Instead of just a colored background, give it a 1px border of the status color and a very dark, low-opacity background of the same color. This gives it a more "lit indicator" feel.
