.pragma library

var TOPICS = {
    "console-overview": {
        title: "Console Overview",
        brief: "Hydra tracks repositories, linked worktrees, and detached tmux sessions from one shell.",
        summary: "This masthead is the high-level register for the current Phase 1 and Phase 2 shell. It tells you whether tmux launch support is live and how much repo or worktree inventory Hydra is currently managing.",
        useNow: [
            "Read MUX first. If tmux is unavailable, Hydra cannot launch shells.",
            "Read R and W as repository count and worktree count for the current workspace state.",
            "Use the left rail to select a target and the main board to inspect or end active shells."
        ],
        controls: [
            "MUX reports whether detached tmux launching is available on this machine.",
            "R counts repositories loaded into Hydra's registry.",
            "W counts linked worktrees for the selected repository snapshot."
        ],
        limits: [
            "This is still a launcher and session register, not an embedded terminal workbench.",
            "Provider-specific execution lanes and richer telemetry arrive in later phases."
        ]
    },
    "launch-bus": {
        title: "Launch Bus",
        brief: "Launch Bus shows whether the detached generic-shell tmux path is actually ready.",
        summary: "Launch Bus is the readiness panel for the only launcher Hydra supports today. It confirms tmux availability and calls out whether the current target can be launched as a detached shell.",
        useNow: [
            "Confirm MUX READY before expecting launches to work.",
            "Use this panel to understand whether the current target can spawn a detached tmux shell.",
            "Treat blocked or warning text here as launch-preflight feedback."
        ],
        controls: [
            "GENERIC SHELL is the current launch type implemented in Phase 1 and 2.",
            "TMUX is the backing multiplexer Hydra uses to keep sessions alive outside the app window.",
            "The short status line below the chips explains whether launch is available or blocked."
        ],
        limits: [
            "No provider-specific launch presets exist yet.",
            "Hydra does not yet embed or render the terminal surface after launch."
        ]
    },
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
    "execute": {
        title: "Execute",
        brief: "Execute arms and launches the current repository or worktree target as a detached tmux shell.",
        summary: "This is the actual launch surface. It reflects the current selected target and is the handoff point between the rail state and the live session board.",
        useNow: [
            "Verify the target name and path before launching.",
            "Press the launch control to start a detached tmux shell in the current target directory.",
            "Use the main session board after launch to confirm the shell is active or end it later."
        ],
        controls: [
            "ARMED means Hydra has a selected repo and tmux is available.",
            "The title row shows the current worktree branch when one is selected, otherwise the repository name.",
            "The path row shows the exact working directory Hydra will hand to tmux."
        ],
        limits: [
            "The launched shell is detached and persists outside the app until ended.",
            "Hydra still does not render the terminal output inside the UI."
        ]
    },
    "sessions": {
        title: "Sessions",
        brief: "Sessions is the live register of detached tmux shells Hydra currently sees as active.",
        summary: "The board is the readout and control surface for active detached sessions. It summarizes the current target state, lets you refresh from SQLite, tmux, and Git, and exposes an explicit end action for each active shell.",
        useNow: [
            "Use REFRESH when you want Hydra to resync live state from persistence and tmux.",
            "Read the session rows as detached background shells that continue after the app closes.",
            "Use END to kill the tmux session and mark the persisted row as exited."
        ],
        controls: [
            "TARGET, ACTIVE, and MUX summarize the current launch context and live session count.",
            "The status banner is for transient feedback, not a permanent log.",
            "Each session row shows the session name, current repo, state, detail text, and end action."
        ],
        limits: [
            "Exited sessions are persisted but not shown in the active board list.",
            "There is still no embedded terminal surface, timeline, or provider telemetry lane here."
        ]
    }
}

function topic(topicId) {
    return TOPICS[topicId]
}
