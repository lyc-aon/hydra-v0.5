# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/post-divider-refine/phase2-baseline-wide.png`
- `.runtime/ui-captures/post-divider-refine/phase2-baseline-resized-wide.png`
- `.runtime/ui-captures/post-divider-refine/phase2-baseline-collapsed-wide.png`
- `.runtime/ui-captures/post-divider-refine/phase2-live-wide.png`
- `.runtime/ui-captures/post-divider-refine/phase2-live-resized-wide.png`
- `.runtime/ui-captures/post-divider-refine/phase2-live-collapsed-wide.png`
- `.runtime/ui-captures/post-divider-refine/phase2-live-tight.png`
- `.runtime/ui-captures/post-divider-refine/phase2-live-compact.png`

Here is a review of the Hydra V2 UI based on the provided screenshots and goals.

### 1. Top 10 Problems in the Current UI

1.  **Low Information Density & Wasted Space:** The UI is dominated by excessive padding and margins. Cards are large with little content, forcing scrolling and making the app feel empty and inefficient. This is the highest impact problem.
2.  **Lack of Visual Hierarchy:** All elements exist on the same visual plane. There's no clear distinction between the control sidebar and the main content area, or between primary actions (like "LAUNCH") and status indicators. This creates a flat, confusing user experience.
3.  **Inconsistent and "Soft" Component Styling:** Components look like they come from different UI kits. Buttons have different padding and corner radii. The overall aesthetic is too rounded, friendly, and reminiscent of generic web dashboards, directly contradicting the "oldtech" and "control-room" goals.
4.  **Monotonous & Low-Contrast Typography:** A single font style with minor weight changes makes the UI hard to scan. The gray description text on the dark background has poor contrast and is difficult to read.
5.  **Confusing Layout Relationships:** The sidebar and main panel use the same visual language (cards), failing to establish a clear "controller" and "content" relationship. The resizable divider's utility is questionable when both sides feel the same.
6.  **Static and Lifeless:** The UI appears completely static. There's no indication of hover states, focus, or click feedback, which makes the application feel unresponsive and dead.
7.  **Uninspiring "Empty State":** The "COMMAND CHANNEL IDLE" screen is a massive missed opportunity. It's visually boring and doesn't guide the user or establish the desired aesthetic.
8.  **Generic Color Palette:** The dark theme is a good start, but the over-reliance on a bright, warm orange feels generic and misaligned with the "phosphor" or "Evangelion" themes, which call for cooler, more deliberate color use.
9.  **Poor Responsiveness:** The layout breaks down ungracefully. In `phase2-baseline-resized-wide.png`, the sidebar content becomes cramped and collides, indicating a lack of robust responsive rules.
10. **Missing "Mythic Hydra" Identity:** There are no visual elements—subtle or overt—that connect the UI to its namesake. It lacks a unique brand or character.

### 2. What Already Works

*   **The Two-Column Layout:** The fundamental structure of a control sidebar and a main content area is correct and a solid foundation.
*   **Dark Theme:** The choice of a dark, low-contrast background is appropriate for the target aesthetic.
*   **Core Data Grouping:** Using cards to encapsulate repositories, worktrees, and sessions is a logical way to organize information.

### 3. Where the UI Misses the Target Aesthetic

*   **"Nu-oldtech" vs. SaaS Minimalism:** It currently looks like a generic SaaS dashboard. It's too clean, too rounded, and too spacious. "Oldtech" implies a certain density, sharpness, and utilitarian focus that is absent.
*   **Matrix/Phosphor Tones:** The aesthetic is not achieved. It needs a cooler, more limited color palette (like phosphor green or amber) and effects like subtle glows or scan lines, not a bright, solid orange.
*   **Evangelion/Control-Room Discipline:** Evangelion's UI is known for its extreme information density, overlapping data, sharp angles, and unconventional typography. This UI is the opposite: sparse, flat, and conventional.
*   **Mythic Undertones:** The theme is entirely absent. There's no sense of power, mystery, or the multi-headed nature of a hydra.

### 4. Concrete Shell V2 Direction

*   **Layout:**
    *   **Asymmetric & Dense:** The sidebar should be significantly narrower, acting as a true control panel. Drastically reduce padding and margins across the entire application to increase information density. Adopt a strict 4px grid for all spacing.
    *   **Visual Distinction:** The sidebar and main panel must be visually distinct. Give the sidebar a darker background or a distinct border to separate it from the content area.
*   **Color:**
    *   **Monochromatic + Accent:** Use a monochromatic palette of dark, cool grays. Select a single, vibrant accent color inspired by phosphor displays (e.g., a sharp green, amber, or cyan). This accent should be used sparingly for interactive elements, highlights, and status indicators.
    *   **Glow/Bloom:** Apply a subtle `DropShadow` or bloom effect to the accent color on text and icons to simulate the glow of a CRT monitor.
*   **Typography:**
    *   **Primary Font:** Switch to a high-quality monospaced font (e.g., Fira Code, IBM Plex Mono, Hack) for *all* UI text. This is critical for the "technical" feel.
    *   **Hierarchy:** Create a rigid typographic scale. Use capitalization, font weight, and the accent color for differentiation, not just size. For example: `ACCENT_COLOR CAPS` for section headers, `White/Bold` for item titles, `Gray/Regular` for descriptions.
*   **Spacing & Interaction:**
    *   **Sharp & Defined:** Use sharp 0-2px corner radii for all rectangles and borders. Eliminate drop shadows from cards. Define components with thin, hard-edged borders.
    *   **Responsive Feedback:** All interactive elements must have clear hover and active states. A simple background color change or a brightening of the accent glow is sufficient. Use `State` and `Transition` in QML for instant, snappy feedback, avoiding slow fades.
*   **Responsive Rules:**
    *   The sidebar should collapse to an icon-only bar on narrow widths.
    *   Card layouts should transition from a grid to a single-column list as horizontal space decreases.

### 5. Specific QML Rewrite Directives

1.  **Create a `Theme.qml` Singleton:**
    *   Define all colors (`backgroundColor`, `panelColor`, `accentColor`, `accentGlowColor`, `textColor`, `subtleTextColor`).
    *   Define all font properties (e.g., `font.family`, `font.pixelSize`) and create explicit properties for sizes: `theme.fontSizeSmall`, `theme.fontSizeMedium`, `theme.fontSizeLarge`.
    *   Define all spacing: `theme.paddingSmall`, `theme.paddingMedium`, etc.

2.  **Rewrite `App.qml` Layout:**
    *   Use a `SplitView` to manage the sidebar and main content.
    *   Set a `maximumWidth` on the sidebar `Item` to enforce asymmetry.
    *   Apply the `theme.backgroundColor` to the root `ApplicationWindow`.

3.  **Refactor Sidebar Components:**
    *   Stop using generic `RepoCard`-style components in the sidebar.
    *   Create new, dense components like `SidebarSection.qml` (for "TARGET", "WORKTREES") and `SidebarListItem.qml`.
    *   These items should not be cards, but simple `Item`s with `Text` and maybe a thin `Rectangle` border, prioritizing density. The "LAUNCH" section should use compact buttons.

4.  **Create a `StyledCard.qml` for the Main Content Area:**
    *   This component will be used for session listings.
    *   It should have a `Rectangle` with a `color` of `theme.panelColor` and a sharp `radius` (e.g., 2). Use a `border.color` instead of a drop shadow.

5.  **Create a `StyledButton.qml`:**
    *   Base it on `Button` but override the `contentItem` and `background`.
    *   The background should be a `Rectangle` that changes color on hover (`MouseArea`).
    *   The `contentItem` `Text` should use the monospaced font and appropriate theme color. Create a `primary` property that, when true, uses the `theme.accentColor`.

6.  **Implement the "Idle" Screen:**
    *   In the main content area, when there are no active sessions, display a large block of ASCII art of a hydra or the word "HYDRA" in a stylized way. This is a low-effort, high-impact way to build the desired aesthetic.

7.  **Add `GlowText.qml` Component:**
    *   Create a custom `Text` component that adds a `DropShadow` behind it with the `accentGlowColor`, a low `radius`, and high `samples`. Use this for status indicators like "READY" or "LIVE" to create the phosphor effect.
