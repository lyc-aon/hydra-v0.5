# Shell V2 Progress

Last updated: 2026-03-06

## Status

- Phase A `Complete`
- Phase B `Complete`
- Phase C `Complete`
- Phase D `Complete`
- Phase E `Complete`

## Phase A

Scope:

- replaced the old beige/bronze-heavy token foundation with a darker phosphor-oriented system
- introduced a usable local type stack around `Roboto Condensed`, `Noto Sans`, and `JetBrainsMono Nerd Font Mono`
- tightened the global shape language and motion durations

Validation:

- build passed
- `scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/phase-a`
- Gemini visual review: `docs/validation/gemini-ui-review-phase-a-2026-03-06.md`
- `scripts/validate_wayland_ui.py` passed

Readout:

- the shell immediately moved away from the dated beige board treatment
- the product still looked sparse and too dashboard-like after this phase
- function remained intact

## Phase B

Scope:

- rewrote the shell composition in `App.qml`, `LaunchSidebar.qml`, and `SessionBoard.qml`
- compressed the left rail into clearer operational bands
- replaced the centered empty board state with a top-left, terminal-like idle surface
- turned the main board into a more explicit session surface instead of a blank card field

Validation:

- build passed
- `scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/phase-b`
- Gemini visual review: `docs/validation/gemini-ui-review-phase-b-2026-03-06.md`
- `scripts/validate_wayland_ui.py` passed

Readout:

- hierarchy is much stronger
- the shell now reads closer to a technical workstation
- the component layer was still too soft and generic in the repo/worktree/session rows
- responsive behavior still needed deliberate component-level work

## Phase C

Scope:

- rewrote the component layer in `RepoCard.qml`, `WorktreeCard.qml`, `MetricTile.qml`, and `SessionCard.qml`
- replaced loud soft borders with denser edge-led states
- sharpened selection language into side rails and bracket-like labels
- made session rows denser and more technical without dropping the current launch/end flow

Validation:

- build passed
- `scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/phase-c`
- Gemini visual review: `docs/validation/gemini-ui-review-phase-c-2026-03-06.md`
- `scripts/validate_wayland_ui.py` passed

Readout:

- the component system now supports the shell direction instead of fighting it
- the UI still needs stronger responsive intent below the current wide layout
- motion/polish are still restrained but incomplete

## Phase D

Scope:

- added explicit shell breakpoints in the outer frame and reduced the supported minimum desktop width to `960px`
- tightened compact behavior in the rail, summary strip, and session rows instead of letting the wide layout simply compress
- extended the screenshot fixture to capture `1040x720` and `960x700` states
- made the visual artifact set deterministic with an automatic contact sheet per capture run
- extended the Wayland validator to confirm critical controls still remain visible in a compact desktop window

Validation:

- build passed
- `scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/phase-d`
- Gemini visual review: `docs/validation/gemini-ui-review-phase-d-2026-03-06.md`
- `scripts/validate_wayland_ui.py` passed

Readout:

- the summary strip now resolves intentionally instead of wrapping opportunistically
- the `1040px` and `960px` states remain legible and operational
- launch, refresh, repo selection, and worktree selection all remain materially visible in the compact Wayland validation pass
- the shell now behaves like a deliberately constrained desktop workbench rather than a broken wide layout

## Phase E

Scope:

- reduced perpetual ambient motion so the shell feels alive without becoming noisy
- changed the launch sweep into a readiness/hover cue instead of a permanent loop
- added session-list add/remove/displaced transitions
- added bracketed frame accents and a more disciplined system-standby state
- kept the Phase 1 / 2 workflow intact while finishing the visual cleanup

Validation:

- build passed
- `scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/phase-e`
- Gemini visual review: `docs/validation/gemini-ui-review-phase-e-2026-03-06.md`
- `scripts/validate_wayland_ui.py` passed

Readout:

- the shell now has controlled liveness instead of prototype-level ambient animation
- session lifecycle changes read more deliberately in the list surface
- the wide and compact captures are now visually consistent enough to treat Shell V2 as complete for the current Phase 2 product slice
- the remaining emptiness in the board is mainly a product-scope artifact: Hydra still does not have provider lanes, timelines, or an embedded terminal yet

## Final readout

Shell V2 is complete for the current product scope.

What is now true:

- the shell direction is substantially stronger than the earlier Phase 1.5 prototype
- the visual validation loop is real and repeatable
- the responsive desktop states are intentionally designed and functionally checked
- the app can proceed into Phase 3 provider work without carrying the original clunky shell forward unchanged

What is still not true:

- this is not the final product feel
- the board is still structurally sparse because Phase 3 to 5 surfaces do not exist yet
- a future shell revision may still introduce a collapsible or iconified rail once provider density increases

## Post-completion refinement

Scope:

- removed roadmap/meta copy from the visible shell, including the phase marker and explicit phase-status text
- simplified the board summary strip so it stays operational instead of self-referential
- added a real collapsible navigation rail with a full-pane board mode
- extended capture output to include collapsed-wide baseline/live states
- extended the live Wayland validator to drive the rail toggle in both wide and compact windows

Validation:

- build passed
- `scripts/capture_ui_screenshots.py --output-dir .runtime/ui-captures/phase-2-refine`
- Gemini visual review: `docs/validation/gemini-ui-review-phase-1-2-refine-2026-03-06.md`
- `scripts/validate_wayland_ui.py` passed

Readout:

- the shell is less cluttered and less self-referential
- the full-pane board mode is real and functionally validated
- the remaining visual limitation is now mostly missing future product surfaces, not a Phase 1 / 2 shell defect
