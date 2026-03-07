# Gemini UI Review

Model: `gemini-2.5-pro`

Screenshots:
- `.runtime/ui-captures/phase-c/phase2-baseline-wide.png`
- `.runtime/ui-captures/phase-c/phase2-live-wide.png`
- `.runtime/ui-captures/phase-c/phase2-live-tight.png`

Here is a review of the Hydra V2 UI and a concrete direction for the Shell V2 rewrite.

### 1. Top 10 Problems in the Current UI

1.  **Too Clean, Too Modern:** The UI feels like a standard modern web dashboard (SaaS minimalism). It's too clean, with excessive whitespace and thin, delicate lines. It lacks the "heavy," utilitarian feel of old-tech/military hardware.
2.  **Weak Typography:** The font is a generic monospaced font. It's not "blorpy" or stylized enough to evoke the 80s anime/PC-98 aesthetic. The type hierarchy is flat, with insufficient contrast in weight and style.
3.  **Ineffective Use of Color:** The color palette is too safe. While dark, it lacks the high-contrast, limited, and opinionated feel of a "Neon Night" or "Military Green" CRT display. The orange accent is generic and lacks a "phosphor glow" quality.
4.  **Lack of "Greeble" and Information Density:** The UI feels sparse. The "nu-oldtech" aesthetic is characterized by a high density of (often non-functional) data, readouts, and status indicators that make the interface feel complex and powerful.
5.  **Flimsy Component Styling:** Cards, buttons, and borders are styled with thin, single-pixel lines. They lack the "heavy," "armored," and "analog" feel. Components feel like they are floating in space rather than being part of a physical console.
6.  **No CRT/Analog Simulation:** The UI is rendered with perfect, crisp digital clarity. It's missing the signature effects of the target aesthetic: scanlines, bloom, chromatic aberration, and a subtle CRT curvature.
7.  **Generic Layout:** The two-column layout is functional but lacks personality. It doesn't evoke a pilot's cockpit, a command center console, or a terminal screen overlay.
8.  **"Buttons" as Buttons:** The buttons are obvious, modern buttons (e.g., "CREATE," "REFRESH"). The aesthetic calls for more integrated, in-world controls that feel like part of the hardware.
9.  **Flatness:** The UI is almost entirely flat. There's no sense of depth, layering, or perspective that would come from HUD elements or overlapping frames.
10. **Uninspired "Empty State":** The "> NO SESSIONS ACTIVE" text is a missed opportunity. This is a prime location for a more thematic, in-world display, like a blinking cursor, a "SYSTEM STANDBY" message, or a vector graphic logo.

### 2. What Already Works

*   **Dark Theme:** The foundation of a dark, near-black background is correct.
*   **Monospaced Font Choice:** Using a monospaced font is the right instinct, even if the specific font needs to be bolder.
*   **Basic Layout:** The division between a "targeting/system" panel and a "session board" is logical and provides a good starting point for a more stylized layout.
*   **"READY" / "ONLINE" / "LIVE" Indicators:** The small, colored status indicators are a good feature that can be enhanced to feel more like glowing LEDs or CRT status markers.

### 3. Where the UI Misses the Target Aesthetic

The UI misses the **"nu-oldtech"** and **"Evangelion"** aesthetic in its soul. The current design is a *web page*. The target aesthetic is a *piece of hardware*.

*   It's **polite**; it should be **assertive**.
*   It's **spacious**; it should be **dense**.
*   It's **clean**; it should be **cluttered with purpose**.
*   It's **digital**; it should feel **analog-hybrid**.
*   It's **flat**; it should have **simulated depth**.
*   It whispers "SaaS"; it should scream "Mecha cockpit."

The design discipline of modern UI/UX (generous spacing, clear calls to action, minimal chrome) is the enemy of this aesthetic.

### 4. Concrete Shell V2 Direction

**Theme: "ARCHON-7" Mainframe Terminal**

The UI is not a window; it's a direct interface to the Hydra "mainframe." It's a heavy, dedicated piece of hardware from an alternate-history 1980s.

*   **Layout:**
    *   **Framing:** The entire UI should be wrapped in a heavy, "bracketed" frame with corner details (`[  ]`).
    *   **HUD Overlay:** The left sidebar should be reimagined as a semi-transparent HUD overlay, perhaps with a slight perspective shift or a visible grid pattern.
    *   **Grid Lines:** Use a faint, pixel-thin grid across the entire background to reinforce the "terminal" feel.
    *   **Denser Packing:** Reduce all padding and margins by 50-75%. Pack elements tightly. Information density is key.

*   **Color:**
    *   **Palette: "Nerv-After-Dark"**:
        *   **Background:** `#0D0C14` (A deep, near-desaturated indigo).
        *   **Primary Text/UI:** `#E0E0E0` (Slightly off-white).
        *   **Primary Accent (The "Phosphor"):** `#FF55A3` (A hot, vibrant magenta, for highlights, active states, and accents).
        *   **Secondary Accent (Status/System):** `#00BFFF` (Deep sky blue, for secondary info and status).
    *   **Glow/Bloom:** All accent-colored text and indicators should have a subtle `drop-shadow` or bloom effect of the same color to simulate CRT glow.

*   **Typography:**
    *   **Font:** Use `VT323` or a similar "blorpy," low-resolution, fixed-width font.
    *   **Hierarchy:**
        *   **Headers (SESSION BOARD, TARGETING):** `UPPERCASE`, with increased letter-spacing.
        *   **Sub-headers:** `UPPERCASE`.
        *   **Body Text:** `lowercase`.
        *   **Blinking Cursor:** Use a solid, blinking block cursor in text fields or as an idle indicator.

*   **Interaction & Motion:**
    *   **Low FPS:** Animations should not be smooth. They should be "janky," stuttery, and reminiscent of low-framerate computer graphics (12-15 fps).
    *   **Typewriter Effect:** Text should appear character-by-character.
    *   **Scanline/Flicker Effects:** On hover or activation, introduce subtle scanline or flicker effects.
    *   **No Fades:** Use hard cuts, not fades.

*   **Responsive Rules:**
    *   **Tight View:** Instead of wrapping, the left "HUD" panel should become a tab or an icon-based bar at the bottom or top, preserving the "single screen" feel. The viewport should feel like a constrained physical monitor.

### 5. Specific QML Rewrite Directives

1.  **Root `ApplicationWindow`:**
    *   Set the background to a `Rectangle` with the `#0D0C14` color.
    *   Add a faint, repeating `Image` with a grid pattern or use a custom `ShaderEffect` for a grid.
    *   Implement a master `ShaderEffect` for a subtle, full-screen CRT effect (barrel distortion, scanlines, faint vignette).

2.  **Typography:**
    *   Load the `VT323.ttf` font using `FontLoader`.
    *   Create a `HydraTheme.qml` singleton that exposes font objects (e.g., `fontHeader`, `fontBody`) with pre-set sizes, letter spacing, and capitalization.
    *   All `Text` elements should use these theme fonts.

3.  **Component: `HeavyCard.qml` (Replaces current cards)**
    *   Use a `Rectangle` with a 2px or 3px `Border` instead of 1px.
    *   The border should be a `BorderImage` that uses a custom sprite for corners, giving them a bracketed `[ ]` or reinforced look.
    *   Remove rounded corners. All corners are sharp right angles.

4.  **Component: `GlowingText.qml`**
    *   Create a custom `Text` component.
    *   In it, place a `Text` element and a `DropShadow` from `QtGraphicalEffects` behind it. The shadow should have `radius: 8`, `samples: 16`, `color: <text color>`, and `spread: 0.2`. This creates the bloom.

5.  **Left Sidebar ("HUD")**
    *   Make its background `color: "#0D0C14"` with an opacity of `0.85`.
    *   Give it a 2px `border` on the right side using the primary accent color (`#FF55A3`).
    *   The "GENERIC SHELL" and "LOCAL STATE" cards should be instances of `HeavyCard`.

6.  **Session Board (Right Side)**
    *   When empty, display a `GlowingText` element with a blinking block cursor and the text "AWAITING COMMAND..."
    *   Active session cards (`SessionCard.qml`) must be `HeavyCard` instances.
    *   The "IDLE" / "LIVE" status indicators should be `GlowingText` and should have a `SequentialAnimation` with a `PropertyAnimation` that subtly pulses their opacity to feel more "alive."

7.  **Buttons (`CREATE`, `END`)**
    *   Rethink these. Instead of a standard `Button`, use a `Rectangle` with a `HeavyCard` border and `GlowingText` inside. The text should change color on hover/press. For example: `[ CREATE ]` in `#E0E0E0` becomes `[ CREATE ]` in `#FF55A3` on hover.
