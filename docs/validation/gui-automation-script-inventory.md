# GUI Automation Inventory

Only four scripts remain in the active tree.

## Kept scripts

- `scripts/validate_wayland_ui.py`: default semantic UI regression gate; writes Hydra stdout/stderr to temp log files, presses the live `End session` action, verifies relaunch/restart behavior on GNOME Wayland, and now uses explicit node waits for its cleanup/removal path
- `scripts/capture_ui_screenshots.py`: deterministic offscreen screenshot capture
- `scripts/ui_fixture.py`: shared repo, database, and tmux fixture helpers
- `scripts/hydra_router_control.py`: product helper staged into the master/router workspace

## Rule

Generated `__pycache__` artifacts are not part of the repo surface. They may appear locally after script validation, but they should stay ignored and untracked.
