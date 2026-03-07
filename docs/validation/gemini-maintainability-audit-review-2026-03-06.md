# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/maintainability-audit/contact-sheet.png`
- `.runtime/ui-captures/maintainability-audit/phase2-live-wide.png`
- `.runtime/ui-captures/maintainability-audit/phase2-live-collapsed-wide.png`

Based on my analysis of the provided screenshots and your aesthetic goals, here is a comprehensive review and a concrete plan for Shell V2.

### 1. Top 10 Problems in the Current UI

1.  **Severe Lack of Visual Hierarchy:** All elements share the same color and typographic weight, making it impossible to distinguish between headers, interactive elements, static data, and status indicators at a glance. This is the single most impactful issue.
2.  **Monochromatic Flatness:** While the green theme is a strong start, its exclusive use makes the UI feel flat and lifeless. It lacks the depth needed for a modern interface and fails to guide the user's eye.
3.  **Weak Interaction Cues:** Buttons (`[MAX READY]`) and links (`show`, `kill`) are styled like static text labels, providing no clear affordance that they are clickable. This leads to poor usability.
4.  **Rigid, "Boxy" Layout:** The UI is a series of harsh, nested rectangles. This feels dated and constrains the content, working against the goal of a "modern, highly responsive" application.
5.  **Monotonous Typography:** A single font style and size is used for nearly everything, which contributes significantly to the flat hierarchy and makes the dense information hard to parse.
6.  **Cramped Spacing:** Padding and margins are too tight and uniform, giving the interface a cluttered and stressful feel. There is no visual "breathing room" to help separate and define content areas.
7.  **No "Nu" in the "Nu-Oldtech":** The aesthetic leans too heavily on "oldtech," resembling a genuine legacy terminal rather than a modern, stylized interpretation. It's missing the refinement and polish that would make it feel "nu."
8.  **Missing the NGE/Control-Room Vibe:** The current UI is too calm and uniform. The "control-room" and *Evangelion* aesthetics often rely on high contrast, urgency, and a sense of layered, critical information, which is absent here.
9.  **Unrefined Responsiveness:** The contact sheet suggests responsiveness is handled by collapsing the sidebar entirely. A modern shell should reflow, resize, and adapt content more gracefully across a range of widths.
10. **Absence of Depth and Materiality:** All surfaces have the same visual properties. There is no distinction between background, foreground, and interactive layers, making the UI feel like a single, uninteresting plane.

### 2. What Already Works (Preserve These)

*   **Strong Thematic Identity:** The choice of a monochrome green is a powerful and effective foundation. It immediately establishes the desired "phosphor" mood.
*   **Logical Information Architecture:** The separation of concerns between the "Launch Bus" (controls/actions) and "Sessions" (monitoring/output) is clear and intuitive.
*   **High Information Density:** For a developer-focused workbench, presenting a lot of information is a feature, not a bug. The current UI correctly prioritizes density; the challenge is in the *presentation*.
*   **Minimalist Discipline:** The UI avoids unnecessary decoration, gradients, and shadows. This clean, functional-first approach is a core strength that aligns with the "technical control-room" goal.

### 3. Where the UI Misses the Target Aesthetic

*   **"Nu-Oldtech":** It feels like an emulator, not an homage. It needs modern typographic scales, refined spacing, and subtle-but-smooth animations to feel "nu."
*   **"Matrix/Phosphor":** It has the color but lacks the *energy*. It needs a sense of glow, bloom, and contrast to evoke the feeling of light-on-dark that defines the phosphor aesthetic, rather than just green-on-green.
*   **"NGE Influence":** *Evangelion*'s UI design is often brutalist, urgent, and stark (sharp lines, bold red/black/white text). This UI is too polite, rounded, and low-contrast to evoke that feeling.
*   **"Mythic Hydra":** This theme is entirely absent. There are no visual elements (a subtle logo, watermark, or motif) to suggest it.

### 4. A Concrete Shell V2 Direction

#### Layout
*   **Base:** Evolve from nested boxes to a layered, "paneled" system. Use a near-black (`#0A0F0A`) for the absolute window background. Panels should be a dark, desaturated green (`#1A2B1A`), giving them a distinct, floating feel.
*   **Structure:** Retain the two-column layout but implement it with a `SplitView`. The left sidebar should be resizable by the user, not just collapsible.
*   **Header:** Add a thin, global header bar for the "HYDRA" brand mark and perhaps 1-2 critical status indicators.

#### Color & Typography
*   **Palette:**
    *   **Background:** Near-black (`#0A0F0A`)
    *   **Panels:** Dark desaturated green (`#1A2B1A`)
    *   **Primary Text/UI:** A bright, slightly-desaturated phosphor green (`#7FFF7F`)
    *   **Secondary Text:** A dimmer, less-saturated green (`#4C8C4C`) for labels, paths, etc.
    *   **Accent/Interactive:** A stark, off-white or "bone" color (`#E0E0D8`). **This is critical.** Use it for buttons, links, cursors, and active state indicators.
    *   **Glow/Highlight:** Use a semi-transparent version of the primary text color for subtle glow effects.
*   **Typography:**
    *   **UI Chrome:** Use a modern, proportional sans-serif (like Inter or a system default) for panel headers and buttons to differentiate them from content.
    *   **Data/Content:** Stick with a clean monospaced font (like Fira Code, JetBrains Mono, or Hack) for session details, paths, and terminal data.
    *   **Scale:** Establish a clear type scale. Ex: Headers = 14pt Bold, Sub-headers = 12pt Bold, Body = 11pt Regular, Labels = 10pt Regular.

#### Spacing & Interaction
*   **Rhythm:** Use a 4px or 8px grid to define all padding, margins, and component dimensions. Be more generous with padding inside panels (`12px` or `16px`).
*   **Interaction:**
    *   **Buttons:** Give them a solid background (dark panel green) that lightens on hover. Use the accent color for the text.
    *   **State:** Use the accent color for borders or "pills" to clearly mark `[ACTIVE]`, `[ONLINE]`, etc. The current bracket `[]` syntax is good; it just needs color.
    *   **Motion:** Use subtle, fast (100-150ms) fade and slide transitions for elements appearing/disappearing. The `SplitView` divider drag should be smooth.

### 5. Specific QML Rewrite Directives

1.  **Create a `Theme.qml` Singleton:**
    ```qml
    // Theme.qml
    pragma Singleton
    import QtQuick

    QtObject {
        readonly property color background: "#0A0F0A"
        readonly property color panel: "#1A2B1A"
        readonly property color textPrimary: "#7FFF7F"
        readonly property color textSecondary: "#4C8C4C"
        readonly property color accent: "#E0E0D8"
        // ... etc.
    }
    ```

2.  **Abstract `StyledText` and `StyledHeader` Components:**
    *   Create components that accept the text and a `level` (`primary`, `secondary`, `label`) and pull color/size from the `Theme.qml` singleton. This enforces consistency.

3.  **Build a Reusable `Button.qml`:**
    *   It should use `MouseArea` to manage its own `hovered` and `pressed` states, changing its background/text color based on the `Theme` accent colors. Do not style buttons manually across the app.

4.  **Use `SplitView` for the Main Layout:**
    *   This provides resizability out-of-the-box and better aligns with a modern workbench feel.
    *   ```qml
      SplitView {
          anchors.fill: parent
          Sidebar { ... }
          ContentView { ... }
      }
      ```

5.  **Use `Layouts` for Spacing:**
    *   Use `RowLayout`, `ColumnLayout` and `GridLayout` inside panels instead of manual `x`/`y` positioning. Use `Layout.fillWidth` and `Layout.preferredHeight` to manage sizing. This is key for a responsive design.

6.  **Implement a `Card.qml` or `Panel.qml`:**
    *   This component will be a `Rectangle` with the `Theme.panel` color, a standard `radius`, and consistent internal padding provided by a `Layout`. All major UI sections will be built from these cards.

7.  **Add Subtle Glow with `DropShadow`:**
    *   Apply a `DropShadow` to key text elements like headers.
    *   ```qml
      DropShadow {
          anchors.fill: source
          horizontalOffset: 0
          verticalOffset: 0
          radius: 8.0
          samples: 17
          color: Theme.textPrimary // or a dedicated glow color
          source: source
      }
      ```

8.  **Leverage `Behavior` for Smooth Transitions:**
    *   Apply `Behavior on opacity {}` or `Behavior on y {}` to elements that appear, disappear, or move, ensuring UI changes are fluid, not jarring.
