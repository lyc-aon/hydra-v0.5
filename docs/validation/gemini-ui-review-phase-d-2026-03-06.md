# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/phase-d/phase2-baseline-wide.png`
- `.runtime/ui-captures/phase-d/phase2-live-wide.png`
- `.runtime/ui-captures/phase-d/phase2-live-tight.png`
- `.runtime/ui-captures/phase-d/phase2-live-narrow.png`
- `.runtime/ui-captures/phase-d/phase2-live-compact.png`

I will start by using the `review_ui_with_gemini.py` script to get a baseline analysis of the UI. This will help formulate a comprehensive response to the user's request.
Here is the UI review and Shell V2 direction based on the provided screenshots and aesthetic goals.

### 1. Top 10 Problems in the Current UI

1.  **Flat, Lifeless Typography:** The font (likely a system default) is generic and lacks the "nu-oldtech" character. There's no typographic hierarchy; titles, labels, and data blend together, reducing scannability. The all-caps headings feel more like modern web design than a specialized tool.
2.  **Inconsistent Spacing and Alignment:** Padding and margins are inconsistent throughout the application. Elements within cards, sections, and the main layout grid don't share a common rhythm, making the UI feel disorganized and haphazard.
3.  **Ambiguous Visual Hierarchy:** It's difficult to tell what the most important information on the screen is. The "SESSION BOARD" and the left-hand "SYSTEM // LAUNCH" panel compete for attention. The orange call-to-action buttons are the most dominant elements, but their purpose isn't always primary.
4.  **Overuse of Borders:** Nearly every element is wrapped in a border. This creates a busy, cluttered "box-in-a-box" layout that fragments the UI instead of creating a cohesive whole. It's a common anti-pattern that inhibits a sense of modern, clean design.
5.  **Uninspired Color Palette:** The dark theme is generic. It's a simple dark gray/blue with white/light-gray text and a single bright orange accent. It lacks the subtle, glowing, phosphor-green or amber undertones of the target aesthetic. The orange feels more like a generic tech startup color than a deliberate choice for this theme.
6.  **Wasted Vertical Space:** The main "SESSION BOARD" has large, empty areas. The "SYSTEM STANDBY" message in the baseline screenshot is huge and uninformative. Even with an active session, the card is small, leaving a void.
7.  **Lack of "Grown-Up" Subtlety:** The design is very direct and literal. The "HYDRA" title is just text. There are no subtle mythical undertones, no interesting graphical motifs or textures that would elevate it beyond a basic functional tool.
8.  **Confusing Interactive Cues:** What is clickable? The main repository cards have a "[TARGET]" tag that looks like a button but probably isn't. The "master" worktree has the same tag. The mix of buttons, tags, and plain text interactive elements is confusing.
9.  **Poor Responsiveness:** While there are different layouts, they don't feel like a truly responsive design. The "narrow" and "compact" views just stack elements, creating a long, cumbersome scroll. The core relationships between panels are lost.
10. **Missing "Control Room" Discipline:** The UI feels more like a web dashboard than a dedicated control room. There's no sense of a deliberate, dense, information-rich environment. It lacks the structured, gridded, and almost technical feel of an Evangelion console or a real-world mission control interface.

### 2. What Already Works and Should Be Preserved

*   **Two-Column Layout:** The fundamental concept of a left-hand "control/selection" panel and a right-hand "status/output" area is strong and logical for this application's workflow.
*   **Clear State Indication:** The "READY," "ONLINE," "LIVE," and "IDLE" status indicators, while stylistically simple, are effective at communicating the state of the system at a glance. The use of color (green) is intuitive.
*   **Componentization:** The breakdown of the UI into distinct components (Repo Cards, Worktree Cards, Session Cards) is a solid foundation. The individual components are well-defined in their purpose.
*   **Core Information Architecture:** The grouping of concepts is logical: Repositories and Worktrees are related, Sessions are the result of actions on them, and the System status is a global concern.

### 3. Where the UI Misses the Target Aesthetic

*   **Nu-Oldtech:** The current UI is just "new tech." It looks like a standard modern dashboard. It's missing the "old" part—the texture, the typography of 90s technical manuals, the satisfying chunkiness of older hardware interfaces.
*   **Matrix/Phosphor:** There are no phosphor-like glows, no scan lines, no subtle CRT effects. The colors are flat and opaque. The "glow" is just a drop shadow on the orange borders.
*   **Evangelion Influence:** The UI lacks the density, urgency, and unique iconography of the NERV interface. There are no angular motifs, no hexagonal patterns, no sense of layered, transparent data overlays. The typography is completely off-base.
*   **Mythic Hydra:** There is nothing subtle or grown-up that suggests "Hydra." It's just a name at the top of the screen. No logo, no thematic elements in the UI.

### 4. Concrete Shell V2 Direction

*   **Layout:**
    *   Maintain the two-column layout but refine its proportions and relationship.
    *   Introduce a strict grid system (e.g., 8pt grid) for all spacing and alignment.
    *   In the right-hand panel, instead of cards floating in space, have them dock to the top, creating a "log" or "feed" of events and active sessions. The "SYSTEM STANDBY" message should be a more compact, elegant status line, not a giant centered block of text.
*   **Color:**
    *   **Base:** A very dark, slightly desaturated teal or blue-gray (#0A1A1F).
    *   **Primary Text/UI:** A slightly off-white with a hint of green or cyan, like a classic phosphor screen (#B0C9C0).
    *   **Accent (for interaction/selection):** A sharp, glowing amber/orange (#FFB86C), but used more sparingly. Use it for selection highlights, active cursors, and key status indicators, not for every button.
    *   **Secondary Accent (for secondary info/status):** A muted, deep cyan (#4A9D9C).
    *   **Glows:** Apply subtle outer glows of the accent color to selected items to simulate a CRT phosphor effect.
*   **Typography:**
    *   **Primary Font:** Use a monospaced font with character, like `Fira Code`, `IBM Plex Mono`, or a more stylized choice like `Share Tech Mono`. This immediately gives it a more technical feel.
    *   **Headings:** Use the mono font, but play with weight and character spacing. Avoid all-caps. Instead of large font sizes, use horizontal lines or other graphical separators. e.g., `> SESSION_BOARD`.
    *   **UI Text:** Use a clean, complementary sans-serif for descriptions, like `Inter` or `IBM Plex Sans`. This creates a professional hierarchy between data/labels and descriptive text.
*   **Spacing & Interaction:**
    *   Get rid of most borders. Use spacing (padding and margins on the grid) and subtle background color shifts to group elements.
    *   For interactive cards (like Repositories), on hover, the entire card should have a subtle inner-glow/highlight, and the cursor should change. The "[TARGET]" tag should be redesigned as a status indicator, not an interactive element.
    *   Buttons should be less rounded and have a more "machined" feel. Consider a slight bevel or inset effect instead of a flat color.
*   **Motion:**
    *   On selection, don't just instantly change state. Have a brief, subtle "blink" or "flicker" effect on the text/card, as if a CRT screen is refreshing.
    *   When new sessions appear, they should fade in and slide down smoothly.
    *   UI updates should feel crisp and instantaneous, like a terminal, not slow and bubbly like a web app.
*   **Responsive Rules:**
    *   **Wide:** The two-column layout is ideal.
    *   **Narrow:** Instead of stacking, the left panel should become a collapsible sidebar, perhaps triggered by a "HYDRA" logo/icon button. The main session board would then take up the full view.
    *   **Compact:** Prioritize the session view. The selection/launch controls could be in a modal or a bottom sheet. The key is to preserve the context of the main "board," not just list all the components vertically.

### 5. Specific QML Rewrite Directives

1.  **Create `Theme.qml`:**
    *   Define all colors (base, text, primary accent, secondary accent) as properties.
    *   Define font families and sizes (e.g., `fontMono`, `fontSans`, `fontSizeH1`, `fontSizeBody`).
    *   Define standard spacing units (e.g., `spacingUnit: 8`). All margins and padding should use multiples of this.

2.  **Rethink `RepoCard.qml` and `WorktreeCard.qml`:**
    *   Remove the outer `Rectangle` border. Use a `Rectangle` for the background, but group elements with spacing.
    *   The `[TARGET]` tag should be a `Text` or `Label` component, styled differently from a button, perhaps just colored text.
    *   Implement `MouseArea` that covers the entire card. On `hover`, change the background color or apply a `Glow` effect.

3.  **Overhaul `SessionBoard.qml`:**
    *   Change the main layout from a free-form `Item` to a `ListView` or `ColumnLayout` docked to the top.
    *   The "SYSTEM STANDBY" message should be a separate component that is only visible when the session list is empty. Style it as a single line of text, perhaps with a blinking cursor effect: `> COMMAND CHANNEL IDLE _`
    *   The "ACTIVE SESSIONS" header should be a styled `Label`, not just plain text. Consider a horizontal `Rectangle` line to separate it from the content.

4.  **Refactor `LaunchSidebar.qml`:**
    *   Replace all-caps `Text` headers with a new `SectionHeader.qml` component that uses the mono font, a horizontal line, and proper spacing.
    *   The `Create` and `Launch` buttons should be a new `PrimaryButton.qml` component. This button should have a less-rounded, more technical design. Use the amber accent color, but perhaps as a border or on hover, with the main button being a darker color.

5.  **Implement a `MonospaceLabel.qml` component:**
    *   Create a reusable `Label` that is pre-styled with the chosen monospaced font. Use this for all data, identifiers, and technical status indicators. This will ensure consistency and make it easy to update the font globally.
