# Hydra V2 Guidance System Review (2026-03-07)

This document provides a detailed review of the Hydra V2 guidance system, based on the provided screenshots and an analysis of the underlying QML code.

## 1. Evaluation

The guidance system is a well-designed, two-stage help system that effectively provides information to the user without being intrusive.

### 1.1. Hover Descriptions

The on-hover tooltips for interactive elements are clear, concise, and useful. They provide immediate context without requiring a click. The delay is slightly too long, which can make them feel sluggish.

### 1.2. Info Buttons

The circular "i" buttons are discoverable and their placement next to section headers is intuitive. They don't create visual noise and successfully invite interaction.

### 1.3. Quick Help Bubble

The quick help bubble is excellent.
- **Clarity:** The information hierarchy is clear, with a title, a brief summary, and a more detailed summary.
- **Placement:** The bubble appears close to the source of the click, which is good for context.
- **Affordance:** The "[DETAILS]" and "[DISMISS]" buttons are unambiguous and well-styled.

### 1.4. Detailed Help Panel

The detailed help panel is also well-executed.
- **Readability:** The text is easy to read, with good contrast and font choices.
- **Information Architecture:** The content is well-organized into logical sections ("USE IT NOW", "CONTROLS", "CURRENT LIMITS").
- **Visual Fit:** The panel's design is consistent with the application's old-Steam/Hermes aesthetic.

## 2. Top 5 Remaining Issues

1.  **Redundant Information:** The detailed help panel repeats the title and summary from the quick help bubble.
2.  **Developer Jargon:** The subtitle "current shell documentation // phase 1-2" in the detailed help panel is internal development information and not useful for the end-user.
3.  **Inconsistent Terminology:** The quick help uses "[DISMISS]" while the detailed help uses "[CLOSE]".
4.  **Distracting Background:** The background pattern in the detailed help panel, while aesthetically pleasing elsewhere, is distracting when trying to read dense text.
5.  **Technical Language:** The help content in `HelpCatalog.js` is often too technical, using terms like "MUX", "repo-local state", and "Phase 1 and 2 shell".

## 3. What Should Be Preserved

- The two-stage **quick help -> detailed help** flow is excellent and should be kept.
- The **`InfoDotButton` component** is a great way to trigger help non-intrusively.
- The **retro-futuristic aesthetic** is strong and should be maintained.
- The **layout and information hierarchy** of both the quick help bubble and the detailed help panel are effective.

## 4. QML-Directed Refinement Instructions

### 4.1. `qml/Hydra/App.qml`

1.  **Make Dismissal Consistent:** In the `detailHelpPanel`, change the `Text` inside the `closeHelpButton` to use the same text as the quick help bubble.
    ```qml
    // In the detailHelpPanel's closeHelpButton
    Text {
        anchors.centerIn: parent
        text: "[DISMISS]" // Changed from "[CLOSE]"
        color: closeDetailArea.containsMouse ? HydraTheme.accentBronze : HydraTheme.textOnDarkMuted
        font.family: HydraTheme.monoFamily
        font.pixelSize: 9
        font.bold: true
    }
    ```
2.  **Improve Focus on Detailed Help:** Increase the opacity of the background scrim to make the detailed help panel stand out more.
    ```qml
    // In the item with z: 21 (the detailed help overlay)
    Rectangle {
        anchors.fill: parent
        color: HydraTheme.withAlpha(HydraTheme.shellDepth, 0.85) // Increased from 0.72
    }
    ```
3.  **Reduce Distraction in Detailed Help:** Change the background of the `detailHelpPanel` to a solid color to improve readability.
    ```qml
    // In the detailHelpPanel Rectangle
    Rectangle {
        id: detailHelpPanel
        // ...
        color: HydraTheme.railPanelStrong // Use a solid color
        // ...
    }
    ```
4.  **Remove Redundant Header in Detail View:** The large title and subtitle in the detailed view are redundant.
    ```qml
    // In the detailHelpPanel Column
    Row {
        width: parent.width
        spacing: HydraTheme.space8
        visible: false // Hide this entire row
    
        Column {
            // ...
        }
    
        Rectangle {
            id: closeHelpButton
            // ...
        }
    }
    ```

### 4.2. `qml/Hydra/HelpCatalog.js`

**Rewrite for Clarity:** The content of the help catalog should be rewritten to be more user-centric and less technical.

**Example for `target-map`:**

**Before:**
```javascript
"target-map": {
    title: "Target Map",
    brief: "Target Map is where you choose the repository root or a linked worktree for the next shell.",
    summary: "This section resolves the working directory for new sessions. You choose a repository, inspect its linked worktrees, optionally create a new worktree, and then hand that target to the execute panel.",
    useNow: [
        "Select a repository card first.",
        "Select a linked worktree if you want the next shell to start away from the repository root.",
        "Type a branch name and create a worktree when you need a fresh Git-linked target."
    ],
    controls: [
        "Repository cards set the current repository context.",
        "Worktree cards switch the working directory for future launches.",
        "The branch field plus CREATE issues a Phase 2 worktree creation request against Git."
    ],
    limits: [
        "Hydra provisions repo-local state and Git worktree support, but it does not yet manage richer context packs here.",
        "Text-field automation is still weaker than button automation in the current GUI harness."
    ]
},
```

**After:**
```javascript
"target-map": {
    title: "Target Map",
    brief: "Choose the working directory for the next shell.",
    summary: "The Target Map is where you select the folder where new command-line shells will start. You can choose a main repository or a specific worktree.",
    useNow: [
        "Click on a repository to select it.",
        "If the repository has worktrees, you can select one to use it as the working directory.",
        "To create a new worktree, type a name for a new branch and click 'CREATE'."
    ],
    controls: [
        "The list of repositories shows all the projects Hydra knows about.",
        "The list of worktrees shows folders linked to the selected repository.",
        "The 'CREATE' button will create a new worktree for the selected repository."
    ],
    limits: [
        "Hydra can only create worktrees for Git repositories.",
        "Some advanced repository features are not yet supported."
    ]
},
```

This change would need to be applied to all topics in the `HelpCatalog.js` file.

### 4.3. `qml/Hydra/components/InfoDotButton.qml` (and other tooltips)

To make tooltips feel more responsive, reduce the `ToolTip.delay` property where `ToolTip` is used. A value around 150-200ms is usually a good starting point.

**Example from `LaunchSidebar.qml`:**
```qml
// In LaunchSidebar.qml, for the muxChip
ToolTip.delay: 150 // Reduced from 280
```
This change should be applied to all tooltips to ensure a consistent user experience.
