# HYDRA V2

Hydra V2 is the planning and build repository for a native desktop orchestration workbench for parallel agent CLIs.

Current direction:

- Native desktop shell in C++23 with Qt 6, Qt Quick/QML, CMake, Ninja, SQLite, and CPack.
- `tmux` as the primary session engine on macOS and Ubuntu-class Linux.
- Provider-native adapters for Claude Code, Codex CLI, Gemini CLI, and generic commands.
- A terminal rendering abstraction that keeps session logic independent from any single embedded terminal backend.

This repo currently holds the preserved research document plus the expanded architecture, planning, ADR, and validation notes that drive implementation.

Document map:

- `HANDOFF.md`
- `docs/research/original-user-research-verbatim.md`
- `docs/validation/source-validated-implementation-plan-2026-03-06.md`
- `docs/validation/ui-observation-tooling-2026-03-06.md`
- `docs/validation/wayland-ui-operator-guide-2026-03-06.md`
- `docs/validation/phase-1-1.5-2-validation-report-2026-03-06.md`
- `docs/validation/visual-review-pipeline-2026-03-06.md`
- `docs/validation/gemini-ui-review-2026-03-06.md`
- `docs/validation/gemini-inspiration-image-analysis-2026-03-06.md`
- `docs/validation/gemini-hermes-steam-pass-review-2026-03-06.md`
- `docs/validation/gemini-phase1-2-delta-review-2026-03-06.md`
- `docs/validation/gemini-deep-visual-pass-review-2026-03-06.md`
- `docs/validation/inspiration-palette-extraction-2026-03-06.md`
- `docs/validation/steam-palette-extraction-2026-03-06.md`
- `docs/validation/deep-visual-pass-report-2026-03-06.md`
- `docs/validation/steam-palette-correction-report-2026-03-06.md`
- `docs/validation/micro-alignment-refinement-report-2026-03-06.md`
- `docs/validation/ux-assistance-refinement-report-2026-03-06.md`
- `docs/validation/gemini-ux-assistance-review-2026-03-06.md`
- `docs/validation/maintainability-audit-report-2026-03-06.md`
- `docs/validation/gemini-maintainability-audit-review-2026-03-06.md`
- `docs/validation/refactor-followup-report-2026-03-06.md`
- `docs/validation/gemini-refactor-followup-review-2026-03-06.md`
- `docs/architecture/project-definition.md`
- `docs/architecture/system-architecture.md`
- `docs/architecture/ui-shell-component-map.md`
- `docs/architecture/provider-adapter-contract.md`
- `docs/architecture/repo-storage-and-worktrees.md`
- `docs/planning/repository-tree-proposal.md`
- `docs/planning/implementation-phases.md`
- `docs/planning/maintainability-audit-pass-2026-03-06.md`
- `docs/planning/phase-1-2-refactor-roadmap-2026-03-06.md`
- `docs/planning/shell-v2-rewrite-plan-2026-03-06.md`
- `docs/planning/phase-1-2-followup-refinement-2026-03-06.md`
- `docs/planning/ux-assistance-refinement-pass-2026-03-06.md`
- `docs/recommendations/deep-visual-pass-2026-03-06.md`
- `docs/planning/implementation-start-prompt.md`
- `docs/adr/0001-qt-qml-shell.md`
- `docs/adr/0002-tmux-session-backend.md`
- `docs/adr/0003-terminal-surface-abstraction.md`

## Current implementation status

Implemented and verified locally:

- desktop Qt Quick shell builds and launches in the current Ubuntu environment
- repo registry and bootstrap repo registration
- generic `tmux` shell launch with SQLite persistence
- persisted session refresh against live `tmux` state
- Phase 1.5 shell styling pass
- repo-local `.hydra/` provisioning
- `.git/info/exclude` wiring for `.hydra/`
- Git worktree listing, creation, selection, and worktree-bound launch targeting
- initial AT-SPI accessibility naming for launch-rail and session-board controls
- deterministic screenshot export and Gemini-based visual review of the shell
- Shell V2 rewrite completed through responsive and motion/polish passes
- collapsible navigation rail with validated full-pane board mode
- divider-mounted navigation toggle with adjustable rail width and animated collapse/expand
- Hermes / Steam visual-direction refinement pass over the Phase 1/2 shell
- exact-sampled Steam/Hermes/NGE token retune plus a deeper instrument-panel visual pass
- transient refresh confirmation validated through the live GUI harness
- Steam-only palette correction pass based on exact pane extraction from `Image Inspiration/Oldschool Steam.jpg`
- micro-alignment cleanup for small-card strip treatment and split-band label placement
- contextual hover/help layer for the current Phase 1/2 shell
- maintainability audit pass with explicit QML property wiring, modular shell decomposition, shared process execution, and reduced file sizes
- follow-up Phase 1/2 refactor pass with one shared delayed hover-hint system and no remaining native Qt attached tooltips in the shell

Still pending:

- provider adapters
- structured status ingestion
- embedded terminal surface
- command palette and keyboard-first hardening
- audit/telemetry/permission governance features

## Local build commands

Core smoke:

```bash
cmake --preset debug-make
cmake --build build/debug-make -j2
./build/debug-make/hydra_core_smoke
```

Desktop app:

```bash
cmake --preset debug
cmake --build build/debug --target hydra -j2
./build/debug/hydra
```

Visual capture and review:

```bash
python3 scripts/capture_ui_screenshots.py
python3 scripts/review_ui_with_gemini.py
python3 scripts/capture_guidance_states_x11.py
python3 scripts/validate_wayland_ui.py
python3 scripts/validate_divider_drag_x11.py
```

## Notes

- The current product value is orchestration, persistence, repo-local state, and worktree-aware launch targeting.
- Terminal rendering remains intentionally deferred behind `TerminalSurface`.
- Local UI observation and feature-driving is currently best done through AT-SPI on the real GNOME Wayland session; see `docs/validation/ui-observation-tooling-2026-03-06.md`.
- Visual shell review is now done through built-in screenshot export plus headless Gemini review; see `docs/validation/visual-review-pipeline-2026-03-06.md`.
- `scripts/capture_ui_screenshots.py` now exports deterministic wide, resized-wide, collapsed-wide, tight, narrow, and compact shell states plus a contact sheet.
- `scripts/capture_guidance_states_x11.py` captures real X11 window images for hover tooltips, quick help, and detailed help.
- the shell now uses a delayed themed hover-hint layer instead of native Qt attached tooltips, so guidance screenshots reflect the same visual language as the rest of the app.
- `scripts/validate_wayland_ui.py` checks the normal shell and the compact `960x700` shell for semantic product behavior.
- `scripts/validate_divider_drag_x11.py` validates the divider click/drag path under Xwayland with `xdotool`.
- both GUI validators now scope AT-SPI lookups to the specific Hydra process they launched, which avoids interference from unrelated open Hydra windows.
- the current shell component relationships are documented in `docs/architecture/ui-shell-component-map.md`.
