# Wayland UI Operator Guide

Last updated: 2026-03-06

## Purpose

This guide explains how Hydra can currently be inspected and driven from automation on the local Ubuntu GNOME Wayland environment.

It is intentionally practical:

- what works
- what does not work
- how to run the supported validation flow
- how to interpret the results against the current roadmap

This guide now covers both:

- semantic interaction testing on the real GNOME Wayland session
- deterministic screenshot capture and Gemini review for visual inspection

Pointer-only divider drag/click validation now has a separate companion path under Xwayland:

- `python3 scripts/validate_divider_drag_x11.py`

## Supported automation path

The supported path is direct `AT-SPI` automation against the real desktop session with `pyatspi`.

Use this path when the goal is to:

- find Hydra by window/application name
- find named controls semantically
- press current buttons without screen-coordinate clicking
- validate filesystem, SQLite, and `tmux` side effects from real UI actions
- inspect control geometry to catch obvious visibility/layout regressions

Current prerequisites:

- GNOME Wayland session
- active session bus
- `QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1`
- `python3-pyatspi`
- Hydra desktop binary built at `build/debug/hydra`

## What the current UI exposes semantically

Hydra currently exposes enough accessible naming to drive the main Phase 1 / 1.5 / 2 shell controls:

- `Repository <repo name>`
- `Worktree <branch name>`
- `Worktree branch name`
- `Create worktree`
- `Launch tmux shell`
- `Refresh Hydra state`
- `Toggle navigation rail`
- `End session <session name>`

These names are sufficient for:

- repo selection
- worktree selection
- launch
- refresh
- end session

## Current automation boundary

The branch input is visible through AT-SPI and has usable geometry, but on this GNOME Wayland setup it is not currently exposed as a working semantic text-entry surface.

What this means in practice:

- the field can be found
- the field can be measured
- button actions around it can be found
- semantic text insertion into that field is not currently reliable through the toolchain we validated

As a result:

- UI-driven worktree creation is not yet covered by the supported automation script
- worktree creation must currently be validated either manually or through backend/bridge probing plus UI validation of the resulting worktree

## Explicitly unsupported or unreliable paths

These were tested and are not the recommended path today:

- `dogtail` as the primary driver
  - installed, but GNOME introspection access was denied for the input path we attempted
- `Xvfb + xdotool`
  - Hydra did not produce a usable visible X11 window in the tested virtual session
- `gnome-screenshot` / GNOME Shell screenshot DBus calls
  - screenshot access was denied in the tested non-interactive automation context
- `ydotool`
  - attempted as a keyboard-injection fallback, crashed on `uinput`, and was removed

## Recommended validation workflow

### 1. Build Hydra

```bash
cmake --preset debug
cmake --build build/debug --target hydra -j2
```

### 2. Run the supported Wayland validation script

```bash
python3 scripts/validate_wayland_ui.py
```

Optional:

```bash
python3 scripts/validate_wayland_ui.py --keep-temp
```

`--keep-temp` preserves:

- the temporary Git repo
- the pre-created linked worktree
- the temporary `XDG_DATA_HOME`
- the temporary SQLite database

Use that when investigating a failure.

### 3. Read the JSON output

The script reports:

- individual checks
- pass/fail status
- layout geometry for key named controls
- compact-window geometry for the current critical controls
- the validation temp paths
- current automation limits

## Visual review companion path

For layout and styling review, pair the Wayland validator with the screenshot/Gemini path.

### Capture representative UI states

```bash
python3 scripts/capture_ui_screenshots.py
```

Outputs land under:

- `.runtime/ui-captures/current/`

### Run the Gemini image review

```bash
python3 scripts/review_ui_with_gemini.py
```

Current output:

- `docs/validation/gemini-ui-review-2026-03-06.md`

### Why this matters

AT-SPI can tell us whether controls exist, can be pressed, and retain usable geometry.

It cannot by itself answer:

- whether the shell feels clunky
- whether spacing is excessive
- whether the responsive state looks intentional
- whether the current visual direction matches the intended Hydra identity

That is now covered by the screenshot + Gemini review loop.

## What the script currently validates

The script validates the implemented roadmap surface honestly.

Phase 1:

- app launches
- temp repo can be injected into isolated app data and selected from the UI
- session launch creates a real persisted SQLite row
- session launch creates a live `tmux` session
- restart reload surfaces the live session again
- refresh does not duplicate the visible session action
- end session removes the live `tmux` session and marks the row `exited`

Phase 1.5:

- main named controls are visible with non-zero geometry
- session board actions remain stable after refresh/restart
- the divider control remains materialized with usable geometry
- the previous blank-card runtime bug is indirectly guarded because the named controls remain materialized instead of failing out of the delegate path

Phase 2:

- selecting a repo causes `.hydra/` bootstrapping
- `.hydra/docs/` and `.hydra/local.json` are created
- `.git/info/exclude` is updated with `.hydra/`
- a linked worktree is visible in the UI and can be selected
- launching from the selected worktree persists the worktree path as the session working directory

## What still requires separate validation

The following current-product behavior is real, but not fully covered by the supported Wayland UI script:

- typing a new branch name into the field and creating a worktree through the UI
- pointer-only divider drag/click interactions

That specific gap is a limitation of the validated automation surface, not evidence that the feature is absent.

When needed, validate that path by:

1. direct backend/bridge probing of `WorktreeManager` / `AppState::createWorktree`
2. then UI validation of the resulting worktree selection and launch path
3. for divider behavior specifically, run `python3 scripts/validate_divider_drag_x11.py`

## Interpreting geometry output

The script records screen-space geometry for the currently critical controls.

Use that output to catch:

- zero-size controls
- obviously clipped controls
- controls that disappear after refresh/restart
- controls that survive the wide layout but disappear in the compact `960x700` launch
- regressions where a control exists semantically but is no longer materially visible

This is no longer the only replacement for screenshot-based checks.

Hydra now has a real screenshot path, but geometry output still matters because it catches semantic/material regressions even when the visual result looks acceptable.

## Roadmap interpretation

The guide validates the product only to the extent the roadmap claims is complete:

- no provider adapters
- no structured provider telemetry
- no embedded terminal surface
- no terminal attach surface
- no command palette

Do not use this guide to imply Phase 3, 4, 5, or 6 completion.
