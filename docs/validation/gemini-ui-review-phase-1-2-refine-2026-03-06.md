# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/phase-2-refine/phase2-baseline-wide.png`
- `.runtime/ui-captures/phase-2-refine/phase2-baseline-collapsed-wide.png`
- `.runtime/ui-captures/phase-2-refine/phase2-live-wide.png`
- `.runtime/ui-captures/phase-2-refine/phase2-live-collapsed-wide.png`
- `.runtime/ui-captures/phase-2-refine/phase2-live-tight.png`
- `.runtime/ui-captures/phase-2-refine/phase2-live-compact.png`

I have completed the UI/UX review. Here are my recommendations:

# Gemini UX/UI Review: Hydra Shell V2 Direction

**Date:** 2026-03-07

This document provides a UX and UI design review of the Hydra V2 desktop shell, based on the provided screenshots and target aesthetic. It outlines problems, successes, and a concrete direction for a "Shell V2" rewrite.

## 1. Top 10 Problems in the Current UI

1.  **Low Information Density & Wasted Space:** The dominant issue. The central area is mostly empty, even when "live." Key information is pushed to the periphery, forcing users to scan large distances. The `phase2-baseline-wide.png` screenshot is the primary example of this.
2.  **Lack of Clear Hierarchy:** The visual relationship between elements is weak. The left rail, the top "SESSIONS" header, and the main content area feel like separate, disconnected panels rather than a cohesive whole. It's difficult to tell what the primary focus of the application should be.
3.  **Inconsistent Component Styling:** Card styles, button styles, and typographic treatments are inconsistent. Compare the "RepoCard" and "WorktreeCard" - they have different internal padding, text alignment, and metadata display. The primary orange buttons have different corner radii and visual weight.
4.  **"Dead" Space & Awkward Centering:** In the collapsed view (`phase2-baseline-collapsed-wide.png`), the main content ("COMMAND CHANNEL IDLE") is awkwardly centered, leaving vast, unused space. This feels unbalanced and unintentional.
5.  **Ambiguous Interactivity:** Which elements are clickable? The `[TARGET]` and `[TGT]` labels look like tags, but their function is unclear. The entire "RepoCard" is a button, but the path inside it isn't. This ambiguity increases cognitive load.
6.  **Typographic Imbalance:** The main `> COMMAND CHANNEL IDLE` text uses a large, heavy font that stylistically clashes with the smaller, lighter fonts used everywhere else. The size difference is too extreme and creates a visual "hole" in the center of the screen.
7.  **Color Palette Misuse:** The bright orange is used for both primary actions (`LAUNCH`), secondary actions (`CREATE`), and status indicators (`MUX READY`, `ONLINE`). This dilutes its meaning. The green "LIVE" and "READY" indicators are too bright and feel more like SaaS/web conventions than the desired "nu-oldtech" aesthetic.
8.  **Redundant "SESSIONS" Title:** The large "SESSIONS" title at the top of the main view is redundant. The application's purpose is managing sessions; this space could be used for more functional UI, like global controls or a command palette.
9.  **Rigid, Unresponsive Layout:** The layout feels static and blocky. There's no sense of how it would adapt to different window sizes or aspect ratios beyond collapsing the rail. The transition between `tight`, `compact`, and `wide` appears to just be truncation or reflow, not a truly responsive reorganization of content.
10. **Missing "Evangelion" / "Control Room" Discipline:** The UI lacks the sense of purpose-driven, dense, and slightly esoteric design seen in the references. It feels more like a standard modern web dashboard (sidebar + content area) than a specialized, powerful tool. There are no subtle grid lines, no data-dense readouts, no sense of a "system" at work.

## 2. What Already Works

*   **Dark Theme Foundation:** The core color scheme (dark gray/blue, orange accent) is a solid starting point. It's moody and aligns with the "phosphor" feel.
*   **Left Rail Concept:** The collapsible "Launch Rail" is a good pattern. It correctly separates the primary context-switching and action controls from the main monitoring view.
*   **Card-Based Metaphor:** Using cards for repositories, worktrees, and sessions is a good, scalable metaphor. It's intuitive and provides clear boundaries for related information.
*   **Clear "Active Target":** The dedicated "ACTIVE TARGET" panel in the sidebar is excellent. It removes ambiguity about the context of the primary launch action.
*   **Minimalist Chrome:** The application has very little unnecessary window chrome, which helps it feel focused and tool-like.

## 3. Where the UI Misses the Target Aesthetic

*   **Too "Web App," Not Enough "Workstation":** The layout is a classic web dashboard. The Evangelion/control-room aesthetic is about information density and purpose-built interfaces. This feels too spacious, too "friendly," and not specialized enough.
*   **Lacks Subtle Complexity:** The aesthetic references are dense with subtle details: fine grid lines, small status indicators, blocky-but-purposeful component grouping, and layers of information. The current UI is flat and lacks this depth.
*   **Typography is Too Generic:** The font choice is clean but lacks the "nu-oldtech" character. It needs a more monospaced, technical, or condensed feel to evoke a terminal or a piece of specialized hardware.
*   **Color is Too Clean:** The orange and green are vibrant and pure. The "matrix/phosphor" feel comes from slightly desaturated, glowing, and imperfect colors. The `READY` green feels like a "success" state from a web form, not a system status indicator.

## 4. Concrete Shell V2 Direction

### Layout
*   **Adopt a Multi-Panel, Tiled Layout:** Move away from the simple "sidebar + content" model. The main area should be a tiled grid where different modules (e.g., "Active Sessions," "System Status," "Logs," "Target Details") can live. This immediately creates density and a "control room" feel.
*   **Centralize the "Command Channel":** Instead of a large, empty prompt, integrate this into a global "Command Palette" at the top of the screen (a-la VS Code, Discord). This makes it always accessible and frees up the main view.
*   **Merge Sidebar and Main View:** The distinction is artificial. The "Target" and "Worktree" lists are primary information. Integrate them into the main tiled layout. The rail could be reserved *only* for global actions or application modes (e.g., "Launch," "Manage," "Settings").

### Color & Typography
*   **Refined Color Palette:**
    *   **Primary Background:** Keep the dark charcoal/blue.
    *   **Accent:** Use the orange more sparingly. It should be for *primary, irreversible actions only* (e.g., `LAUNCH`, `END`).
    *   **Secondary/Status:** Introduce a muted, desaturated green (phosphor-like) for "Ready/Live/Online." Use a muted amber/yellow for "Idle/Standby."
    *   **Borders & UI:** Use a slightly lighter shade of the background for panel borders and UI elements to create subtle separation.
*   **Typography:**
    *   **UI Font:** Switch to a high-quality, condensed, sans-serif font with good clarity at small sizes (e.g., Inter, Roboto Condensed).
    *   **Monospace/Data Font:** Use a dedicated monospace font (e.g., Fira Code, Source Code Pro) for all data-heavy readouts, paths, and identifiers. This creates a clear distinction between UI labels and system data.

### Spacing & Interaction
*   **Tighter, Consistent Spacing:** Reduce padding within cards and between panels. Use a consistent 8px grid for all spacing and component sizes.
*   **Clear Affordances:** Buttons should look like buttons. Interactive elements should have a clear hover state (e.g., a subtle glow or background color change). Use visual cues like a `>` chevron to indicate navigation.
*   **Motion:** Use subtle, fast animations. Panel transitions should be quick fades or slides. Hover effects should be immediate. The goal is responsiveness, not "delightful" motion.

### Responsive Rules
*   **Tile Reflow:** On narrower screens, the tiled panels should reflow into a single column.
*   **Dynamic Sidebar:** The sidebar could transform from a full panel of controls to an icon-only bar on smaller screens, with tooltips on hover.
*   **Content Truncation:** Be more aggressive with truncating long paths or descriptions, showing the full text in a tooltip.

## 5. Specific QML Rewrite Directives

1.  **`App.qml`:**
    *   Replace the `Sidebar` and `SessionBoard` split-view with a `GridLayout`. This will be the foundation for the new tiled layout.
    *   Investigate creating a global `CommandPalette` component, perhaps anchored to the top of the `ApplicationWindow`.

2.  **`SessionBoard.qml`:**
    *   This component should be broken up. The "COMMAND CHANNEL IDLE" text should be removed (in favor of the command palette).
    *   The "Active Sessions" list should become its own distinct module/tile within the new `GridLayout`.

3.  **`LaunchSidebar.qml`:**
    *   Refactor this into smaller, more focused components that can be placed in the `GridLayout`. For example, `RepoListModel` could feed a `RepoTile` component, `WorktreeList` a `WorktreeTile`, etc.
    *   The "Launch" button should be context-aware, perhaps living in a dedicated "Actions" tile that updates based on the selected target.

4.  **`RepoCard.qml`, `WorktreeCard.qml`, `SessionCard.qml`:**
    *   Create a single, generic `InfoCard.qml` base component that handles the background, border, and basic structure.
    *   Extend this base for `Repo`, `Worktree`, and `Session` specifics.
    *   Standardize the internal padding, font usage, and metadata display across all cards. Use `RowLayout` and `ColumnLayout` to enforce consistent alignment.
    *   Implement clear hover and active states (e.g., using `MouseArea` and changing a `Rectangle`'s color).

5.  **`HydraTheme.qml`:**
    *   Update the color palette as defined in the "Shell V2 Direction." Add new color roles (e.g., `statusReady`, `statusIdle`, `accentAction`).
    *   Define font properties here (e.g., `font.ui`, `font.monospace`) and use them consistently across all components.
    *   Define the standard 8px grid unit here (e.g., `property int gridUnit: 8`) and use it for all margins, padding, and component dimensions (`Layout.margins: theme.gridUnit * 2`).
