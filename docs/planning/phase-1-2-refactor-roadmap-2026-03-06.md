# Phase 1 / 2 Refactor Roadmap

Date: 2026-03-06
Status: Active

## Current state

Phase 1, 1.5, and 2 are functionally complete and validated.
The current codebase is materially cleaner after the maintainability audit pass, but Phase 1 / 2 still has a few quality and maintainability tracks worth finishing before Phase 3 adds more product surface.

Current validated baseline:

- explicit-property QML shell instead of hidden context globals
- modular shell components and reusable shell primitives
- repo / worktree / tmux launch flow is real and validated
- deterministic screenshots, Wayland semantic validation, and X11 pointer validation are all in place
- in-app help surfaces exist and are architecture-aligned

## Active problems

1. The info-dot hover hint still uses the default Qt attached `ToolTip`, which does not match the shell style and appears too quickly.
2. Hover guidance is split between the themed quick-help surface and the default tooltip surface, which weakens visual consistency.
3. The repo currently has no local commit history even though `.git/` exists; a local baseline commit should be created after the next clean pass.
4. The docs need one clearer roadmap for what remains in Phase 1 / 2 refactoring so later work does not drift.

## Research anchors

- Qt Quick Controls `ToolTip` is a shared attached tooltip surface and is not a good fit when the product needs a bespoke themed hover-help layer.
- Qt Quick best practices still favor explicit component boundaries and clear data flow over implicit global state.
- Qt Quick performance guidance still favors simple bindings, careful clipping, and avoiding unnecessary scene churn.
- C++ Core Guidelines still favor explicit interfaces, small functions, and abstractions that match the real domain.

## This pass

1. replace the info-dot attached tooltip path with a delayed themed hover-hint surface
2. keep click-to-open quick help unchanged
3. validate the refined help path with real GUI capture and semantic automation
4. update docs and handoff notes to reflect the current state precisely
5. create the first local git commit after the pass is stable

## Current outcome

Completed in the current pass:

- all remaining attached `ToolTip` usage was removed from the shell
- the shell now uses one delayed, themed hover-hint system hosted by `App.qml`
- section info dots, launch/refresh/end actions, divider resize guidance, repo/worktree cards, target-map controls, and key status chips now route hover text through the same visual system
- the ugly native tooltip path is gone from the real-window capture flow
- the repo is ready for its first local baseline commit after docs are finalized

Still active after this pass:

- keep `AppState` and `git_repo_workspace.cpp` from regressing as Phase 3 grows
- continue using the split validator model instead of trying to force one harness to do every interaction

## Follow-on refactor tracks

### UI shell

- remove the remaining attached-tool-tip usage from the shell in favor of themed hover hints where it materially improves consistency
- keep shared shell primitives small and avoid rebuilding large inline panels
- continue enforcing explicit property boundaries between `App.qml`, shell panels, and cards

### View-model boundary

- keep `AppState` from becoming the next accumulation point as Phase 3 lands
- prefer focused helpers or narrower models when new state groups appear

### Infrastructure

- split `git_repo_workspace.cpp` further only if Phase 3 meaningfully expands its responsibilities
- keep subprocess work routed through the shared process runner instead of reintroducing duplicated boilerplate

### Validation

- preserve the split validator model:
  - Wayland AT-SPI for semantic product flow
  - X11 pointer harness for divider resizing
  - deterministic app-owned screenshots for stable state capture

## Exit condition for the refactor track

Phase 1 / 2 refactoring is in a good stopping state when:

- hover/help surfaces are visually coherent
- docs accurately describe the code as it exists now
- the repo has a local baseline commit
- no large file has regressed into multi-responsibility sprawl
