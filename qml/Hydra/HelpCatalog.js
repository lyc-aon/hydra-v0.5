.pragma library

var TOPICS = {
    "console-overview": {
        title: "Console Overview",
        brief: "Hydra tracks repositories, linked worktrees, detached tmux sessions, and the currently bound terminal target from one shell.",
        summary: "This masthead is the high-level register for the live Hydra shell. It tells you whether tmux launch support is available and how much repository and worktree inventory Hydra is currently managing while the main workbench binds the selected live session into the embedded terminal.",
        useNow: [
            "Read MUX first. If tmux is unavailable, Hydra cannot launch shells.",
            "Read R and W as repository count and worktree count for the current workspace state.",
            "Use the left rail to select a target, the main board to inspect or end active shells, and the selected session row to drive the terminal pane.",
            "Use F2, F3, and F4 to jump focus directly between the rail, sessions board, and terminal.",
            "Use F6 and Shift+F6 to cycle focus zones forward or backward through the shell.",
            "Read the inline shortcut hints on hotkeyed controls when you want a quick refresher without opening the full help sheet.",
            "Use the FULL toggle or F11 when you want the shell to enter or exit borderless fullscreen.",
            "Use F12 when you want to hide or restore the left rail quickly.",
            "When live sessions exist, closing the Hydra window now asks for confirmation before Hydra shuts down this window's own sessions."
        ],
        controls: [
            "TMUX reports whether detached-shell launch is available on this machine.",
            "REPOS counts repositories loaded into Hydra's registry.",
            "WORKTREES counts linked worktrees for the selected repository snapshot.",
            "The fullscreen icon control switches the shell between windowed and borderless fullscreen mode and shows its `F11` shortcut inline."
        ],
    },
    "theme-system": {
        title: "Theme System",
        brief: "Themes now live behind a persisted controller so Hydra can swap named palettes without rewriting the shell.",
        summary: "Hydra currently ships `STEAM`, `HERMES`, `OPENAI`, `CHATGPT`, `CLAUDE`, `CLAUDE INK`, and `HYDRA`. All seven run through the same semantic token system so the shell can shift mood hard without per-component color hacks.",
        useNow: [
            "Use the theme control in the masthead to cycle the active color theme.",
            "Treat `STEAM` as the current default baseline for the live shell.",
            "Use `HERMES` when you want the black-dominant neon-blue shell.",
            "Use `OPENAI` when you want the sparse monochrome OpenAI look with only a restrained green signal accent.",
            "Use `CHATGPT` when you want the darker graphite chat-app look with green highlights.",
            "Use `CLAUDE` when you want Anthropic's warm paper palette with orange, blue, and green accents.",
            "Use `CLAUDE INK` when you want the same Claude accent family on a darker console surface.",
            "Use `HYDRA` when you want the high-contrast black-and-red instrument-panel palette."
        ],
        controls: [
            "The theme control changes the active named palette.",
            "Theme changes are persisted locally so Hydra reopens with the same palette.",
            "Session-state tones, rails, boards, and hover surfaces now all resolve through the theme tokens.",
            "The embedded terminal follows the active theme for background and primary text while keeping a readability-first terminal palette so provider output stays legible across themes."
        ],
    },
    "target-map": {
        title: "Target Map",
        brief: "Target Map is where you add repositories, browse for folders, unbind the current target, and open worktrees only when you need a branch-isolated launch path.",
        summary: "This section resolves the working directory for new sessions. Add repository folders into Hydra's registry here, browse when you do not want to type a path, select a repo when you want a bound target, or unbind the current target when you want HOME launches. Once a repository is selected, the repo root remains the default bound path while the worktree drawer is the advanced path for linked branch targets.",
        useNow: [
            "Use ADD to register another repository folder with Hydra.",
            "Use BROWSE when you want Hydra to open a folder picker instead of typing the path manually.",
            "Select a repository card when you want a bound launch target.",
            "Use UNBIND to remove the current repository/worktree target and return launch to HOME mode.",
            "Leave the worktree drawer closed if the repo root is enough for the next shell.",
            "Open the worktree drawer only when you want the next shell to start away from the repository root.",
            "Type a branch name and create a worktree when you need a fresh Git-linked target."
        ],
        controls: [
            "Repository cards set the current repository context.",
            "The repository path field plus ADD registers another repository folder with Hydra.",
            "BROWSE opens a folder picker and can feed the same add flow without manual path typing.",
            "UNBIND removes the current target selection without touching files on disk.",
            "The worktree drawer exposes linked worktree selection and creation.",
            "The branch field plus CREATE issues a linked Git worktree creation request."
        ],
    },
    "execute": {
        title: "Execute",
        brief: "Execute is the primary launch surface: choose launch target mode, provider, and sandbox mode, then start that CLI inside a detached tmux session.",
        summary: "This is the primary launch surface. It reflects either the selected repository/worktree or HOME mode, keeps provider choice compact, lets you choose Hydra's sandbox posture for launch, and then hands that configuration to the detached tmux session board. When Hermes is selected, this panel also exposes the profile inheritance mode Hydra should use for that isolated Hermes runtime.",
        useNow: [
            "Verify whether the next launch should use TARGET or HOME before launching.",
            "Choose the provider you want Hydra to start for this target from the compact provider selector.",
            "If the provider is Hermes, choose whether Hydra should seed that private Hermes home from your global `~/.hermes`, keep it blank, or use a repo template path.",
            "Choose the sandbox mode you want Hydra to map into the provider's CLI flags.",
            "Press the launch control to start the selected provider in the current target directory.",
            "Use F7 to launch from the keyboard once the current target mode, provider, and sandbox posture are set.",
            "Read the inline `F7` hint in the Execute subtitle if you need a fast reminder while configuring a launch.",
            "Use the main session board after launch to confirm the shell is active or end it later."
        ],
        controls: [
            "READY means Hydra can launch with the current target mode, tmux is available, and the selected provider is installed.",
            "The title row shows either the selected repository/worktree or HOME DIRECTORY.",
            "The path row shows the exact working directory Hydra will hand to tmux.",
            "TARGET binds the next shell to the selected repository root or linked worktree.",
            "HOME starts the next shell in your home directory with no bound repo target.",
            "The provider selector and sandbox strip are the two launch choices Hydra currently owns.",
            "When Hermes is selected, the HERMES PROFILE strip lets you choose `GLOBAL`, `BLANK`, or `REPO TEMPLATE`, and `REPO TEMPLATE` reveals a template-path field."
        ],
    },
    "sessions": {
        title: "Sessions",
        brief: "Sessions is the live register of detached tmux shells Hydra currently sees as active, and it now drives which shell is bound into the terminal surface.",
        summary: "The board is the observability surface for active detached sessions. It summarizes state, recent activity, signal provenance, and a bounded persisted trace for each tmux-backed shell, while also selecting the live shell shown in the embedded terminal panel. Each session carries a unique alias (e.g. @alpha, @bravo) that the master terminal uses for routing.",
        useNow: [
            "Use REFRESH when you want Hydra to resync repository, worktree, SQLite, and tmux state.",
            "Read the session rows as Hydra-owned live tmux shells that Hydra will close when the owning Hydra window exits.",
            "Select a session row to bind that shell into the embedded terminal surface and transfer typing focus into that terminal immediately.",
            "Each session gets an auto-assigned alias from the NATO phonetic alphabet (Alpha, Bravo, Charlie...). The alias is shown on the session card as @alpha, @bravo, etc.",
            "Click the alias label on a session card or use the [ALIAS] chip to rename it. Enter confirms, Escape cancels. Aliases must be unique across live sessions.",
            "Aliases persist across app restarts and session resume. The master terminal will use @alias to route commands to specific sessions.",
            "Use TRACE to inspect the recent persisted event history for a session.",
            "Use EXPORT to write the selected session audit into Hydra's local export directory.",
            "Use END to kill the tmux session and mark the persisted row as exited.",
            "Use the Resume rail section for closed provider sessions you want Hydra to reopen.",
            "Leave the board in MANUAL mode if you want a stable, persisted row hierarchy and drag rows into the order you prefer.",
            "Enable AUTO only when you want Hydra to group idle rows first, approval/input-gated rows next, and active thinking/tool rows last.",
            "Closing the whole Hydra window with live sessions now prompts first so Hydra can shut down this window's own sessions cleanly before exit.",
            "Use Ctrl+Shift+Up and Ctrl+Shift+Down to change the selected session from the keyboard.",
            "Use F8 to toggle TRACE, Shift+F8 to end the selected session, and F10 to export the selected session audit."
        ],
        controls: [
            "The top strip summarizes the current target, live session count, and mux failure state if tmux is blocked.",
            "The MANUAL / AUTO control decides whether Hydra preserves your row hierarchy or re-groups rows by session state.",
            "EXPORT writes the selected session into a local JSON audit artifact.",
            "The status banner is for warnings, errors, and transient guidance, not a permanent log.",
            "Each session row shows the session name, alias, current repo, launch posture, normalized state, activity badge, provenance badge, detail text, alias and end actions.",
            "The @alias label on each session card is clickable. Click it to rename the alias inline.",
            "The [ALIAS] chip in the action rail is an explicit control for entering alias edit mode. It shows [DONE] while editing to confirm the change.",
            "The selected session is the terminal target for the adjacent terminal surface, and mouse row selection now also hands keyboard focus to that terminal.",
            "When MANUAL ordering is active, you can drag session rows to reorder the live board.",
            "TRACE expands a persisted event list so you can inspect what changed and why Hydra believes the current state."
        ],
    },
    "resume": {
        title: "Resume",
        brief: "Resume is the closed-session rail for provider conversations Hydra can reopen after tmux has been shut down.",
        summary: "When Hydra closes or when you explicitly end a resumable provider session, Hydra tears down tmux but keeps the provider conversation metadata in SQLite. Hydra now resolves pending Codex, Gemini, OpenCode, and Hermes provider ids before teardown so Hydra-launched resumable sessions remain reopenable from this rail, and only Hydra-owned conversations are eligible.",
        useNow: [
            "Use Resume when you want Hydra to reopen a closed Codex, Gemini, Claude, Hermes, or OpenCode session in a fresh tmux runtime.",
            "Read the stored status line if you want to know whether the session was ended directly or closed during app shutdown.",
            "Use the dedicated RESUME control on a stored row when you want Hydra to reopen it.",
            "Resume uses the launch-safety mode currently selected in Execute, not necessarily the safety the session last ran under.",
            "Use the selection toggle on a row when you want to delete stored sessions without reopening them.",
            "Use CLEAR ALL when you want Hydra to drop every closed resumable provider session from the local store.",
            "If app shutdown reaches a provider session whose resume metadata never resolved, Hydra now closes it and drops resumability for that run instead of leaving tmux alive across restarts.",
            "The stored-session list keeps its own internal scroll position during normal refresh cycles instead of jumping back to the top."
        ],
        controls: [
            "Each row shows the stored session name, provider, repo context, working directory, and last close detail.",
            "The RESUME control is the reopen action for that stored provider session.",
            "The selection toggle marks stored sessions for bulk deletion.",
            "The per-row DELETE control removes one stored session without reopening it.",
            "DELETE removes the selected stored sessions from Hydra's local resume store.",
            "CLEAR ALL removes every closed resumable provider session currently stored by Hydra.",
            "Resumed sessions leave this rail and return to the active Sessions board."
        ],
        limits: [
            "Resume depends on provider-native conversation persistence.",
            "External provider conversations are not eligible to satisfy Hydra resume entries."
        ]
    },
    "master-terminal": {
        title: "Master Terminal",
        brief: "The Master Terminal is a dedicated command deck with a live master terminal on the left and a live router rail on the right.",
        summary: "Master Terminal replaces the split Workbench layout with a purpose-built orchestration surface. The top orbit matrix condenses worker-session state into animated sync nodes backed by real session state and recent timeline data. The master terminal is the left-side primary surface. The router terminal is a true right-side peer surface that can collapse into a slim rail without pretending to be a separate trace console.",
        useNow: [
            "Press F9 to toggle between Workbench and Master Terminal views.",
            "Use the [WORKBENCH] and [MASTER] toggle buttons in the board frame header.",
            "Click any session sync node in the orbit matrix to switch the selected worker session.",
            "The aggregate pulse ring on the right side of the orbit matrix shows live worker-state distribution at a glance.",
            "Use the right-rail collapse control to compress the router into a slim always-visible status rail.",
            "The heartbeat traces above the master and router terminals reflect the actual current control-plane session state.",
            "Use F3 to focus the orbit matrix, F4 to focus the master terminal, and Shift+F4 to focus the router terminal.",
            "Orbit node motion corresponds to real state: slow breathing for idle, smoother scan for thinking, faster segmented churn for tool execution, locked amber pulse for approval, and unstable red motion for error.",
            "The view mode (Workbench or Master) persists across restarts."
        ],
        controls: [
            "The orbit matrix shows worker sync nodes with condensed motion language tied to actual session state and recent activity.",
            "The aggregate pulse ring is a segmented donut showing the distribution of session states.",
            "The master terminal surface is the left-side embedded terminal for the master agent.",
            "The router terminal rail is the right-side embedded terminal for the smart router agent and can collapse into a narrow rail.",
            "The heartbeat traces above both terminals reflect the live control-plane state instead of decorative ambient motion."
        ],
        hotkeys: [
            { key: "F9", action: "Toggle between Workbench and Master Terminal" },
            { key: "F3", action: "Focus orbit strip (Master) or session board (Workbench)" },
            { key: "F4", action: "Focus master terminal surface" },
            { key: "Shift+F4", action: "Focus router terminal surface" }
        ],
    },
    "terminal": {
        title: "Terminal Surface",
        brief: "The terminal surface embeds the currently selected live tmux shell inside Hydra through a real terminal widget.",
        summary: "This embedded terminal binds to the selected tmux session through a native terminal widget. Hydra still owns session lifecycle, launch posture, and resume semantics, while the terminal surface owns live rendering, input, selection, copy, paste, and external attach behavior.",
        useNow: [
            "Select a live session row first.",
            "Clicking a session row now transfers keyboard focus here, so you can usually start typing immediately without pressing FOCUS first.",
            "Use FOCUS before typing if the terminal viewport does not already own keyboard focus.",
            "Use Escape to trigger the selected provider's interrupt behavior when the terminal owns focus. Hydra only consumes Escape itself when a help overlay is open.",
            "Use ATTACH when you want the same tmux session in an external terminal window.",
            "Use right-click or the panel controls for Copy, Paste, Select All, and Clear Line.",
            "Use normal paste for text. Hydra keeps multiline text paste on the tmux bracketed-paste path when it routes paste itself.",
            "If the selected session is Codex and the clipboard currently holds an image, Hydra routes panel and menu paste into Codex's native image-paste behavior instead of trying to fake the provider UI.",
            "Use F4 to move focus into the terminal panel quickly.",
            "Use Ctrl+Shift+Backspace to clear the entire input line. Hydra sends the readline kill-line sequence to tmux so this works in bash, zsh, and most CLIs.",
            "Use Ctrl+Shift+Up and Ctrl+Shift+Down to change the selected session without leaving the terminal."
        ],
        controls: [
            "The selected-session posture chip tells you whether the bound shell is sandboxed or running in bypass mode.",
            "LIVE means tmux still reports the selected session as active.",
            "COPY copies any current mouse selection from the embedded surface.",
            "PASTE routes through Hydra's provider-aware clipboard path for the currently selected session.",
            "For Codex sessions, image clipboard payloads are routed through Codex's own image-paste key path when you paste from Hydra's controls.",
            "ATTACH launches an external terminal and attaches it to the same tmux session.",
            "CLEAR LINE sends readline's beginning-of-line plus kill-to-end sequence, clearing the current input in one action."
        ],
        hotkeys: [
            { key: "F4", action: "Focus the terminal panel" },
            { key: "Escape", action: "Trigger provider interrupt (e.g. Ctrl+C to the session)" },
            { key: "Ctrl+Shift+Backspace", action: "Clear the entire input line" },
            { key: "Ctrl+Shift+Up", action: "Select previous session" },
            { key: "Ctrl+Shift+Down", action: "Select next session" },
            { key: "Ctrl+A", action: "Move cursor to beginning of line (readline)" },
            { key: "Ctrl+E", action: "Move cursor to end of line (readline)" },
            { key: "Ctrl+U", action: "Kill entire line (zsh) or kill to start (bash)" },
            { key: "Ctrl+K", action: "Kill from cursor to end of line" },
            { key: "Ctrl+W", action: "Kill previous word" },
            { key: "Ctrl+Y", action: "Yank (paste back killed text)" },
            { key: "Ctrl+C", action: "Interrupt / SIGINT" },
            { key: "Ctrl+D", action: "EOF / exit shell" },
            { key: "Ctrl+L", action: "Clear screen" },
            { key: "Ctrl+R", action: "Reverse history search" }
        ],
    }
}

function topic(topicId) {
    return TOPICS[topicId]
}
