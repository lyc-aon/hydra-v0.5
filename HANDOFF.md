# Hydra V2 Handoff

Last updated: 2026-03-06

This file is the canonical handoff/resume note for Hydra V2. Update it directly at the end of each meaningful implementation pass.

## Current situation

Hydra V2 now spans a completed Phase 1 vertical slice, a completed Phase 1.5 shell-tightening pass, and a completed Phase 2 repo-intelligence pass.

The repo currently contains:

- preserved research, architecture, ADR, and planning docs
- a real CMake/Qt workspace
- a working native desktop shell
- an explicit-property QML shell with a documented component tree
- SQLite-backed repo and session persistence
- a `tmux` adapter for detached shell launch and liveness checks
- repo-local `.hydra/` bootstrap logic
- `.git/info/exclude` management for `.hydra/`
- Git worktree listing and creation
- worktree-aware launch targeting

The terminal surface is still intentionally deferred. Hydra currently supervises and launches sessions; it does not yet render or attach their terminal buffers inside the app.

## What was done in the last work session

### 0. Completed the maintainability audit pass

The latest pass was not a feature pass. It was a full maintainability cleanup over the existing Phase 1 and 2 shell and supporting backend code.

Main outcomes:

- removed hidden `QQmlContext` global wiring and replaced it with explicit root properties plus `QQmlApplicationEngine::setInitialProperties()`
- split the oversized shell files into smaller named components
- introduced reusable shell primitives for status chips, section headers, framed surfaces, frame corners, divider handling, and help surfaces
- reduced `src/main.cpp` to a small startup entry point by moving desktop launch/screenshot options into `src/app/desktop_launch_options.*`
- added `src/infrastructure/process/process_runner.*` so Git and tmux subprocess logic no longer duplicate the same synchronous process boilerplate
- added `src/domain/support/slug.*` so the domain no longer duplicates the same slug-generation logic
- removed the generic fallback help-topic path from `qml/Hydra/HelpCatalog.js`
- documented the current shell composition in `docs/architecture/ui-shell-component-map.md`

Key new QML files:

- `qml/Hydra/components/ShellBackdrop.qml`
- `qml/Hydra/components/FrameCorners.qml`
- `qml/Hydra/components/DividerHandle.qml`
- `qml/Hydra/components/QuickHelpBubble.qml`
- `qml/Hydra/components/DetailHelpPanel.qml`
- `qml/Hydra/components/HelpSectionCard.qml`
- `qml/Hydra/components/StatusChip.qml`
- `qml/Hydra/components/SectionHeader.qml`
- `qml/Hydra/components/SurfacePanel.qml`
- `qml/Hydra/components/ConsoleHeader.qml`
- `qml/Hydra/components/LaunchBusPanel.qml`
- `qml/Hydra/components/TargetMapPanel.qml`
- `qml/Hydra/components/ExecutePanel.qml`
- `qml/Hydra/components/SessionsSurface.qml`

Key new C++ files:

- `src/app/desktop_launch_options.hpp`
- `src/app/desktop_launch_options.cpp`
- `src/infrastructure/process/process_runner.hpp`
- `src/infrastructure/process/process_runner.cpp`
- `src/domain/support/slug.hpp`
- `src/domain/support/slug.cpp`

Validation rerun after the refactor:

- `cmake --build build/debug --target hydra -j2`
- `cmake --build build/debug-make -j2`
- `./build/debug-make/hydra_core_smoke`
- `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-audit-test.png --quit-after-screenshot`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/maintainability-audit`
- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`

Important validator scope note:

- `scripts/validate_wayland_ui.py` remains the semantic product-flow validator for the real GNOME Wayland session
- `scripts/validate_divider_drag_x11.py` is now intentionally limited to pointer drag resizing under Xwayland
- collapsed-rail startup states remain covered by deterministic app-owned screenshot capture rather than semantic AT-SPI toggle activation, because that interaction is not currently reliable on this GNOME Wayland setup

### 0.5 Completed the Phase 1 / 2 hover-hint follow-up refactor

The next pass focused on one concrete quality issue and the maintainability opportunity behind it:

- the shell was still mixing the themed quick-help surfaces with the default Qt attached `ToolTip` surface
- the info-dot hover text was the most obvious visual mismatch, but the same problem still existed on launch, refresh, divider, target-map, repo/worktree card, end-session, and status-chip hover paths

Main outcomes:

- removed all remaining attached `ToolTip` usage from the QML shell
- added one delayed themed hover-hint surface hosted by `qml/Hydra/App.qml`
- routed hover text for section info dots, launch/refresh/end controls, divider resize guidance, repo/worktree cards, target-map controls, and the console status chips through that same host
- kept click-open quick help and detailed help behavior unchanged
- updated the guidance capture path so it waits for the delayed hover hint before capturing the real X11 window

Files added:

- `qml/Hydra/components/HoverHintBubble.qml`
- `docs/planning/phase-1-2-refactor-roadmap-2026-03-06.md`
- `docs/validation/refactor-followup-report-2026-03-06.md`
- `docs/validation/gemini-refactor-followup-review-2026-03-06.md`

Key files updated:

- `qml/Hydra/App.qml`
- `qml/Hydra/components/InfoDotButton.qml`
- `qml/Hydra/components/DividerHandle.qml`
- `qml/Hydra/components/StatusChip.qml`
- `qml/Hydra/components/ConsoleHeader.qml`
- `qml/Hydra/components/ExecutePanel.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionsSurface.qml`
- `qml/Hydra/components/SessionCard.qml`
- `qml/Hydra/components/TargetMapPanel.qml`
- `qml/Hydra/components/RepoCard.qml`
- `qml/Hydra/components/WorktreeCard.qml`
- `scripts/capture_guidance_states_x11.py`
- `.gitignore`

Validation rerun after the pass:

- `cmake --build build/debug --target hydra -j2`
- `cmake --build build/debug-make -j2`
- `./build/debug-make/hydra_core_smoke`
- `./build/debug/hydra -platform offscreen --screenshot /tmp/hydra-followup-pass-2.png --quit-after-screenshot`
- `python3 scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/refactor-followup-2`
- `python3 scripts/validate_wayland_ui.py`
- `python3 scripts/validate_divider_drag_x11.py`
- `python3 scripts/capture_guidance_states_x11.py --output-dir .runtime/ui-captures/guidance-followup-2`

Research note:

- the local Full Plate Harness Brave helper exists, but `BRAVE_API_KEY` was not configured in the current environment
- this pass therefore used current official Qt, GNOME, and C++ guidance plus the existing local Gemini CLI review path instead

### 1. Audited the phase docs against the real codebase

Verified:

- Phase 1 exit criteria are met
- Phase 1.5 shell work is complete
- the older phase docs and handoff notes were behind the current codebase and needed to be rewritten

Important nuance recorded in the docs:

- the sidebar now supports a validated collapsible rail / full-pane board mode
- `LaunchProfile` exists only as a model stub
- normalized session-state depth is still basic and driven by `tmux` liveness today

### 2. Implemented Phase 2 repo intelligence

Added backend support for:

- repo-local `.hydra/` provisioning
- `.hydra/providers/`
- `.hydra/docs/`
- `.hydra/heartbeats/`
- `.hydra/session-templates/`
- `.hydra/local.json`
- idempotent `.git/info/exclude` wiring for `.hydra/`
- Git repository detection and root resolution
- Git worktree listing via `git worktree list --porcelain`
- Git worktree creation from a branch name

New core files:

- `src/domain/models/worktree.hpp`
- `src/domain/ports/repo_workspace.hpp`
- `src/domain/services/worktree_manager.hpp`
- `src/domain/services/worktree_manager.cpp`
- `src/infrastructure/git/git_repo_workspace.hpp`
- `src/infrastructure/git/git_repo_workspace.cpp`
- `src/ui/viewmodels/worktree_list_model.hpp`
- `src/ui/viewmodels/worktree_list_model.cpp`

### 3. Wired Phase 2 into the app state and launch flow

Updated:

- `SessionSupervisor` now accepts an explicit working directory for launches
- `AppState` now exposes:
  - worktree model
  - selected worktree path
  - selected worktree branch
  - repo-local state paths/status
  - worktree creation
- generic shell launches now target the selected worktree path when one is selected
- session persistence keeps worktree identity through `working_directory`

### 4. Extended the shell for Phase 2

The launch rail now surfaces:

- repo-local state readiness
- `.hydra` path visibility
- worktree count
- worktree creation input
- worktree list and selection
- selected launch target details

New QML file:

- `qml/Hydra/components/WorktreeCard.qml`

Updated QML files:

- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/styles/HydraTheme.qml`
- `qml/Hydra/components/qmldir`

### 5. Cleaned the current shell usability issues

Added:

- in-app tmux session termination from the session board
- a single scrollable launch rail instead of nested internal scroll regions
- removal of the top overlay accents that were conflicting with rounded shell corners

Updated:

- `src/domain/services/session_supervisor.hpp`
- `src/domain/services/session_supervisor.cpp`
- `src/ui/viewmodels/app_state.hpp`
- `src/ui/viewmodels/app_state.cpp`
- `src/ui/viewmodels/session_list_model.hpp`
- `src/ui/viewmodels/session_list_model.cpp`
- `qml/Hydra/App.qml`
- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionCard.qml`

### 6. Fixed blank ghost entries and established a real UI test path

Investigated the reported blank ghost session entries and found a concrete runtime cause:

- several QML delegates were reading `model.*`
- at runtime those delegate contexts did not reliably expose `model`
- the result was `ReferenceError: model is not defined` and partially blank cards

Fixed:

- replaced fragile delegate bindings with explicit required-role wrappers in:
  - `qml/Hydra/components/LaunchSidebar.qml`
  - `qml/Hydra/components/SessionBoard.qml`
- switched the session board to a real `ListView`
- filtered `Exited` sessions out of the current board model so the board only shows live/non-exited sessions until a real history surface exists
- added accessibility names and button actions to the current launch/session controls so AT-SPI can inspect and drive Hydra on the local GNOME Wayland session

Installed for UI observation and feature-driving:

- `python3-pyatspi`
- `accerciser`
- `scrot`
- `imagemagick`
- `gnome-screenshot`

Installed but not currently recommended as the primary driver:

- `python3-dogtail`

Attempted and removed:

- `ydotool`

Documented in:

- `docs/validation/ui-observation-tooling-2026-03-06.md`
- `docs/validation/wayland-ui-operator-guide-2026-03-06.md`
- `docs/validation/phase-1-1.5-2-validation-report-2026-03-06.md`
- `scripts/validate_wayland_ui.py`

### 7. Added deterministic screenshot capture and Gemini visual review

The earlier functional UI path was real, but it still lacked visual evidence.

Added:

- built-in desktop screenshot export in `src/main.cpp`
  - `--screenshot`
  - `--window-width`
  - `--window-height`
  - `--quit-after-screenshot`
- `scripts/ui_fixture.py` for shared isolated repo/database fixture helpers
- `scripts/capture_ui_screenshots.py`
  - builds an isolated Phase 2 fixture
  - captures wide/tight baseline and live-session screenshots into `.runtime/ui-captures/current/`
- `scripts/review_ui_with_gemini.py`
  - sends workspace-local screenshots to `gemini-2.5-pro`
  - writes the review to `docs/validation/gemini-ui-review-2026-03-06.md`

Documented in:

- `docs/validation/visual-review-pipeline-2026-03-06.md`
- `docs/planning/shell-v2-rewrite-plan-2026-03-06.md`

Current design conclusion:

- Phase 1 / 2 functionality is real
- the shell is still visually too generic and clunky for the intended Hydra direction
- a Shell V2 rewrite should happen before Phase 3 provider UI complexity lands

### 8. Completed the Shell V2 rewrite through responsive and motion/polish passes

Implemented:

- explicit shell breakpoints in `App.qml` with support down to a validated `960px` desktop width
- denser compact behavior in `LaunchSidebar.qml`, `SessionBoard.qml`, `RepoCard.qml`, `WorktreeCard.qml`, `MetricTile.qml`, and `SessionCard.qml`
- deterministic metric-grid behavior in the board summary strip
- tighter compact session rows and rail cards
- automatic contact-sheet generation in `scripts/capture_ui_screenshots.py`
- compact-window validation in `scripts/validate_wayland_ui.py`
- reduced perpetual ambient animation and event-driven launch sweep behavior
- session-list add/remove/displaced transitions
- bracketed frame accents and stronger standby presentation

Validation artifacts:

- `.runtime/ui-captures/phase-d/contact-sheet.png`
- `.runtime/ui-captures/phase-e/contact-sheet.png`
- `docs/validation/gemini-ui-review-phase-d-2026-03-06.md`
- `docs/validation/gemini-ui-review-phase-e-2026-03-06.md`
- `docs/validation/shell-v2-progress-2026-03-06.md`

Current design conclusion:

- Shell V2 is complete for the current Phase 2 product slice
- the shell is materially stronger, denser, and more intentional across wide and compact desktop states
- the board is still naturally sparse because provider lanes, richer status/timeline surfaces, and the embedded terminal do not exist yet
- Phase 3 can now land on top of a credible shell baseline instead of the earlier clunky prototype

### 9. Final Phase 1 / 2 refinement pass

Implemented:

- removed roadmap/meta copy from the visible shell itself
- simplified the board summary strip toward operational context
- added a real collapsible navigation rail with full-pane board mode
- added `Ctrl+B` as a rail-toggle shortcut
- extended screenshot capture to include collapsed-wide baseline/live states
- extended live Wayland validation to drive the rail toggle in both wide and compact windows

Validation artifacts:

- `.runtime/ui-captures/phase-2-refine/contact-sheet.png`
- `docs/validation/gemini-ui-review-phase-1-2-refine-2026-03-06.md`

Refined design conclusion:

- the shell no longer exposes roadmap-phase commentary to the user
- the full-pane board mode is real and validated under live GUI control
- the current limitation is now mostly content density from missing future phases, not a Phase 1 / 2 shell defect

### 10. Active refinement scope

Current follow-up pass is focused on shell mechanics and containment quality:

- move the rail toggle onto the divider between the rail and board
- make the rail width adjustable by dragging that divider control
- add animated collapse/expand behavior and clearer button-press feedback
- investigate and fix active-session action clipping at the bottom edge of the session card
- revalidate the shell with live GUI control, deterministic screenshots, and Gemini image review after the changes land

### 11. Divider and containment refinement pass

Implemented:

- moved the navigation toggle from the board header onto the pane divider itself
- made the divider draggable so the rail width is adjustable in both directions
- animated collapse/expand and divider motion instead of snapping instantly
- added press-state feedback to the refresh, create, launch, end, and divider controls
- fixed the active-session row clipping by making `SessionCard` height content-driven
- added widened-rail screenshot states through `--start-sidebar-width`
- added focused pointer validation under Xwayland in `scripts/validate_divider_drag_x11.py`

Validation artifacts:

- `.runtime/ui-captures/post-divider-refine/contact-sheet.png`
- `docs/validation/gemini-ui-review-divider-refine-2026-03-06.md`

Refined design conclusion:

- the divider now behaves like an actual shell control instead of a header button
- the rail can be hidden, restored, widened, and narrowed without breaking the board
- the session row actions are visually contained in wide, tight, and compact captures
- Wayland remains the semantic validation surface; Xwayland is now the focused pointer-validation surface for divider drag

### 12. Rail motion synchronization refinement

Investigated after noticing that the rail contents, divider, and board were not sliding in perfect lockstep.

Root cause:

- the shell was animating related geometry off separate properties:
  - `railFrame.width`
  - `dividerTrack.x`
  - rail opacity/content fade
- the sidebar content was also re-evaluating compact/dense breakpoints while the rail width was shrinking, so internal layout changes were happening mid-transition

Implemented:

- replaced the separate width/x animation path with a single animated rail reveal scalar in `qml/Hydra/App.qml`
- bound the rail width and divider position directly to that reveal-driven visible width
- kept the sidebar content on a stable expanded layout width while the rail viewport clips it during collapse/expand
- switched the board sizing heuristics to use a stable target layout width during rail transitions instead of recomputing mode thresholds every frame
- hardened `scripts/validate_divider_drag_x11.py` so it waits for post-drag/post-toggle geometry transitions instead of sampling once and flaking

Validation:

- desktop rebuild still passes
- `scripts/validate_wayland_ui.py` still passes end-to-end
- `scripts/validate_divider_drag_x11.py` now passes again after being updated for the new animated timing
- deterministic captures still export successfully through `.runtime/ui-captures/sidebar-sync-refine/`

### 13. Hermes / Steam inspiration synthesis pass

Active follow-up focus:

- inspect the local inspiration set under `Image Inspiration/`
- use the local Gemini CLI OAuth path already used by FPH to analyze the actual reference images
- synthesize those references into a tighter default Hydra theme
- compare the synthesis against the current shell captures before changing code

Planning note:

- this pass is tracked in `docs/planning/hermes-steam-visual-synthesis-pass-2026-03-06.md`

Completion note:

- per-image Gemini analysis of the local inspiration set is stored in `docs/validation/gemini-inspiration-image-analysis-2026-03-06.md`
- objective palette extraction is stored in `docs/validation/inspiration-palette-extraction-2026-03-06.md`
- the resulting visual synthesis is stored in `docs/recommendations/hermes-steam-visual-synthesis-2026-03-06.md`
- the post-change review is stored in `docs/validation/gemini-hermes-steam-pass-review-2026-03-06.md`
- the shell palette now leans old-Steam / Hermes with darker olive-charcoal surfaces, brass-amber primaries, and restrained steel-blue secondary accents
- geometry is slightly sharper and less soft
- live-state atmospheric motion now leans green rather than orange
- the validation harness now scopes AT-SPI lookups to the launched Hydra PID so local testing is not confused by unrelated open Hydra windows

### 14. Deep Phase 1 / 2 visual follow-up pass

Closed in this pass:

- the sticky `Refresh` success banner
- the approximate default palette problem
- the earlier complaint that the shell changes read mostly as a color shift instead of a structural visual pass

Implemented:

- transient status handling in `AppState` for refresh confirmations
- exact token retune in `qml/Hydra/styles/HydraTheme.qml` from measured colors sampled out of:
  - `Oldschool Steam.jpg`
  - `Hermes*.jpeg`
  - `NGE*.webp`
- stronger instrument-panel zoning in:
  - `qml/Hydra/App.qml`
  - `qml/Hydra/components/LaunchSidebar.qml`
  - `qml/Hydra/components/SessionBoard.qml`
- denser list/readout styling in:
  - `qml/Hydra/components/RepoCard.qml`
  - `qml/Hydra/components/WorktreeCard.qml`
  - `qml/Hydra/components/MetricTile.qml`
  - `qml/Hydra/components/SessionCard.qml`
- live GUI proof that the refresh message now appears and clears in `scripts/validate_wayland_ui.py`

New docs and artifacts:

- `docs/recommendations/deep-visual-pass-2026-03-06.md`
- `docs/validation/deep-visual-pass-report-2026-03-06.md`
- `docs/validation/gemini-phase1-2-delta-review-2026-03-06.md`
- `docs/validation/gemini-deep-visual-pass-review-2026-03-06.md`
- `.runtime/ui-captures/deep-visual-pass/contact-sheet.png`

Current visual readout:

- the shell now reads as a measured old-Steam / Hermes / NGE control surface instead of a generic dark dashboard
- the rail, board, cards, and controls changed structurally, not just chromatically
- the remaining visual gap is mostly higher-order polish:
  - richer emissive glow
  - future alert vocabulary
  - future provider/terminal surfaces

### 15. Steam palette correction pass

Active follow-up focus:

- the current palette is still not matching the original green Steam reference closely enough
- the next pass should treat `Image Inspiration/Oldschool Steam.jpg` as the primary palette source for the default shell
- exact sampled values should be documented together with the extraction method, rather than inferred from mixed references
- pass tracker: `docs/planning/steam-palette-correction-pass-2026-03-06.md`

Completion note:

- exact Steam extraction now lives in `scripts/extract_steam_palette.py`
- the Steam-only palette report is in `docs/validation/steam-palette-extraction-2026-03-06.md`
- raw extraction output is in `.runtime/steam-palette-extraction-2026-03-06.json`
- a swatch artifact is in `.runtime/steam-palette-swatch-2026-03-06.png`
- the default shell tokens in `qml/Hydra/styles/HydraTheme.qml` now derive from the measured Steam pane colors instead of the earlier mixed-reference blend
- validated screenshots for this pass are in `.runtime/ui-captures/steam-palette-correction/`
- validation record is in `docs/validation/steam-palette-correction-report-2026-03-06.md`

### 16. Micro-alignment refinement pass

Active follow-up focus:

- darker bottom sub-bands are currently colliding with content in some small display cards and session rows
- some header labels and compact readout labels are straddling split-tone panel bands instead of sitting cleanly inside one zone
- pass tracker: `docs/planning/micro-alignment-refinement-pass-2026-03-06.md`

Completion note:

- removed the small-card bottom sub-bands from `RepoCard.qml`, `WorktreeCard.qml`, and `SessionCard.qml`
- removed the problematic internal split bands from the compact rail panels and metric tiles
- tightened the session-row timestamp inset and status-chip padding
- visual artifacts for this pass are in `.runtime/ui-captures/micro-alignment-refine/`
- validation record is in `docs/validation/micro-alignment-refinement-report-2026-03-06.md`

### 17. UX assistance refinement pass

Active follow-up focus:

- the shell needs clearer in-context explanations of what each section does
- hover states on some interactive elements are still too subtle
- help should be anchored to the relevant section, lightweight on hover, and deeper on click
- pass tracker: `docs/planning/ux-assistance-refinement-pass-2026-03-06.md`

## Build and validation status

### Successful builds

Core:

```bash
cmake --preset debug-make
cmake --build build/debug-make -j2
./build/debug-make/hydra_core_smoke
```

Desktop:

```bash
cmake --preset debug
cmake --build build/debug --target hydra -j2
```

### Validation completed in this session

Passed:

- `hydra_core_smoke`
- desktop build for `hydra`
- offscreen desktop launch: `timeout 4 ./build/debug/hydra -platform offscreen`
- temporary end-to-end Phase 2 harness against a throwaway Git repo verifying:
  - `.hydra/` creation
  - `.git/info/exclude` wiring
  - worktree creation
  - worktree listing
  - worktree-bound `tmux` launch
- temporary unborn-repo harness verifying worktree creation via the orphan-path fallback
- AT-SPI inspection on the real GNOME Wayland session
- AT-SPI-driven launch/end cycle against a temporary `XDG_DATA_HOME`, verifying:
  - `Launch tmux shell` can be invoked semantically
  - a temporary SQLite session row is created
  - `End session Hydra V2 shell` can be invoked semantically
  - the persisted row moves to `exited`
- deterministic offscreen screenshot export at wide and tight sizes
- workspace-local headless Gemini image review over the captured shell states
- isolated Phase 1 / 1.5 / 2 validation against a temporary Git repo and temporary app data, verifying:
  - repo selection from the UI
  - `.hydra/` bootstrap and Git exclude wiring
  - worktree card visibility and selection
  - worktree-bound launch persistence
  - restart reload of the live session
  - clean end-session removal without duplicate visible session actions
- deterministic screenshot export at `1500x920`, `1180x760`, `1040x720`, and `960x700`
- deterministic screenshot export of collapsed-wide board states
- automatic contact-sheet generation for the captured shell states
- Wayland AT-SPI validation of the compact `960x700` desktop window for key controls
- Wayland AT-SPI validation of live rail collapse/restore behavior in both wide and compact windows

Observed local repo state after Phase 2 bootstrap:

- `.hydra/` exists at repo root
- `.git/info/exclude` contains `.hydra/`
- `.runtime/ui-captures/current/` contains current visual review artifacts

### QML lint state

`qmllint` currently reports non-fatal warnings, mainly:

- unqualified access to the `appState` context property

The previous runtime `model is not defined` delegate failure has been fixed. There are no known fatal QML runtime errors in the verified offscreen launch path.

## Current architecture state in code

### Domain layer

Implemented model types:

- `Repository`
- `LaunchProfile`
- `SessionRecord`
- `SessionState`
- `Worktree`

Implemented port interfaces:

- `RepoStore`
- `SessionStore`
- `MuxAdapter`
- `RepoWorkspace`
- `ProviderAdapter`
- `SecretsStore`

Implemented services:

- `RepoRegistry`
- `SessionSupervisor`
- `WorktreeManager`

### Infrastructure layer

Implemented:

- `TmuxAdapter`
- `DatabaseManager`
- `SqliteRepoStore`
- `SqliteSessionStore`
- `GitRepoWorkspace`

### UI bridge layer

Implemented:

- `AppState`
- `RepoListModel`
- `SessionListModel`
- `WorktreeListModel`

### QML layer

Implemented shell surfaces:

- app shell
- launch sidebar
- repo cards
- worktree cards
- session board
- session cards

## Important runtime behavior right now

### Repo-local state

On startup or repo selection, Hydra ensures repo-local state exists:

- `.hydra/`
- `.hydra/providers/`
- `.hydra/docs/`
- `.hydra/heartbeats/`
- `.hydra/session-templates/`
- `.hydra/local.json`

If the repo is a Git repository, Hydra also ensures `.hydra/` is listed in `.git/info/exclude`.

### Worktree flow

Hydra can now:

- inspect worktrees for the selected repo
- create a new worktree from the launch rail
- select a worktree as the current launch target
- launch a detached `tmux` shell into that worktree

### Session supervision

`SessionSupervisor` currently:

- launches detached `tmux` sessions
- persists sessions to SQLite
- reloads persisted sessions
- maps live sessions to `Idle`
- maps dead sessions to `Exited`

UI behavior note:

- exited sessions remain persisted in SQLite
- the current session board hides exited rows and only surfaces live/non-exited sessions
- preserves `Error` when already set

### Terminal boundary

Hydra still does not:

- render terminal output
- attach to the running shell inside the app
- embed a terminal backend

That work remains deferred behind `TerminalSurface`.

## Things intentionally not done yet

- provider adapters for Claude/Codex/Gemini
- `.hydra` docs/context management UI beyond path provisioning
- worktree prune/remove UI
- provider-native restore
- `tmux` control-mode subscriptions
- heartbeat system
- event timeline
- audit log
- typed event bus implementation
- embedded terminal rendering
- secrets-store implementation
- packaging

## Exact files that matter most on resume

### Build system

- `CMakeLists.txt`
- `CMakePresets.json`
- `cmake/HydraWarnings.cmake`

### App/bootstrap

- `src/main.cpp`
- `src/app/hydra_application.hpp`
- `src/app/hydra_application.cpp`

### Domain

- `src/domain/models/session_state.hpp`
- `src/domain/models/session_state.cpp`
- `src/domain/models/worktree.hpp`
- `src/domain/services/repo_registry.hpp`
- `src/domain/services/repo_registry.cpp`
- `src/domain/services/session_supervisor.hpp`
- `src/domain/services/session_supervisor.cpp`
- `src/domain/services/worktree_manager.hpp`
- `src/domain/services/worktree_manager.cpp`

### Infrastructure

- `src/infrastructure/mux/tmux_adapter.hpp`
- `src/infrastructure/mux/tmux_adapter.cpp`
- `src/infrastructure/git/git_repo_workspace.hpp`
- `src/infrastructure/git/git_repo_workspace.cpp`
- `src/infrastructure/persistence/database_manager.hpp`
- `src/infrastructure/persistence/database_manager.cpp`
- `src/infrastructure/persistence/sqlite_repo_store.hpp`
- `src/infrastructure/persistence/sqlite_repo_store.cpp`
- `src/infrastructure/persistence/sqlite_session_store.hpp`
- `src/infrastructure/persistence/sqlite_session_store.cpp`

### UI binding

- `src/ui/viewmodels/app_state.hpp`
- `src/ui/viewmodels/app_state.cpp`
- `src/ui/viewmodels/repo_list_model.hpp`
- `src/ui/viewmodels/repo_list_model.cpp`
- `src/ui/viewmodels/session_list_model.hpp`
- `src/ui/viewmodels/session_list_model.cpp`
- `src/ui/viewmodels/worktree_list_model.hpp`
- `src/ui/viewmodels/worktree_list_model.cpp`

### QML

- `qml/Hydra/App.qml`
- `qml/Hydra/components/LaunchSidebar.qml`
- `qml/Hydra/components/RepoCard.qml`
- `qml/Hydra/components/WorktreeCard.qml`
- `qml/Hydra/components/SessionBoard.qml`
- `qml/Hydra/components/SessionCard.qml`
- `qml/Hydra/styles/HydraTheme.qml`

### Planning and context docs

- `README.md`
- `docs/planning/implementation-phases.md`
- `docs/planning/phase-1-2-followup-refinement-2026-03-06.md`
- `docs/architecture/system-architecture.md`
- `docs/architecture/repo-storage-and-worktrees.md`
- `docs/recommendations/deep-visual-pass-2026-03-06.md`
- `docs/validation/deep-visual-pass-report-2026-03-06.md`
- `docs/validation/source-validated-implementation-plan-2026-03-06.md`

## Recommended next steps in order

1. Add external-terminal handoff or attach flow so launched sessions become directly usable from the shell.
2. Start Phase 3 provider adapters, beginning with the cleanest provider contract boundary.
3. Add structured status ingestion before attempting richer state semantics in the board.
4. Keep embedded terminal work behind `TerminalSurface` and treat it as its own subsystem.

### 18. Completed the UX assistance refinement pass

Implemented:

- section-level info buttons for the current shell areas:
  - console overview
  - launch bus
  - target map
  - execute
  - sessions
- short hover guidance on the main interactive shell controls
- shared quick-help and detailed-help surfaces at the app-shell level
- stronger hover feedback on repository and worktree cards plus the main action buttons
- startup hooks for help topics in `src/main.cpp`
- actual-X11 help/hover capture script in `scripts/capture_guidance_states_x11.py`

Important validation note:

- deterministic offscreen capture remains the correct path for baseline/live shell states
- actual hover/help surfaces are now reviewed from real X11 window captures instead of offscreen startup captures
- the live Wayland validator now verifies that a section info button opens quick help, promotes to detailed help, and closes cleanly

Artifacts:

- `.runtime/ui-captures/ux-assistance-refine/contact-sheet.png`
- `.runtime/ui-captures/guidance-x11/contact-sheet.png`
- `docs/validation/ux-assistance-refinement-report-2026-03-06.md`
- `docs/validation/gemini-ux-assistance-review-2026-03-06.md`

Current readout:

- the shell now explains itself without leaking roadmap/meta copy into the product UI
- the guidance layer is honest to the current Phase 1 / 2 architecture boundary
- the remaining limitation is future-surface absence, not a missing explanation path for the current shell
