# Hydra v0.5 Guide

## What Hydra Does

Hydra is a Linux desktop shell for running multiple AI CLI sessions side by side over `tmux`.
It gives you:

- a workbench for launching worker sessions
- a master/router control plane for orchestrated multi-agent work
- session previews, resume history, and embedded terminals

## Prerequisites

- Linux desktop with Qt 6.5+ build deps
- `tmux`
- locally installed provider CLIs you want to use:
  - `codex`
  - `gemini`
  - `claude`
  - `hermes`
  - `opencode`
- valid local auth/config for any provider you intend to launch

## Build

From the repo root:

```bash
tools/prepare_qmltermwidget.sh
cmake --preset debug
cmake --build build/debug --target hydra -j8
```

## Launch

Standard launch:

```bash
./build/debug/hydra
```

Skip the boot animation:

```bash
./build/debug/hydra --skip-boot
```

## Basic Use

### Workbench

1. Add or select a repo/worktree in the left rail.
2. Pick a provider.
3. Launch a worker session.
4. Use the embedded terminal normally:
   - wheel scroll
   - drag selection
   - right-click Copy / Paste / Select All

### Master / Router

1. Open the `Master` view.
2. Launch `Master` and `Router`.
3. Send a natural-language request to the master.
4. Master forwards to router.
5. Router dispatches workers asynchronously.
6. Workers report back to router.
7. Router forwards reviewed summaries to master.

## Session Lifecycle

- Closing a live session should move it to `Resume`, not silently relaunch it on next boot.
- Closing Hydra should terminate Hydra-owned live sessions and preserve resumable session data.
- Deleting a stored resume entry should remove it from the UI immediately.

## Safe Verification

Fast local checks:

```bash
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_core_smoke
./build/debug/hydra_provider_launch_overrides_smoke
./build/debug/hydra_session_order_smoke
./build/debug/hydra_terminal_clipboard_routing_smoke
./build/debug/hydra_shutdown_resume_smoke
QT_QPA_PLATFORM=offscreen ./build/debug/hydra_boot_probe --skip-boot --wait-ms 4000
```

Longer desktop audit:

```bash
python3 scripts/validate_wayland_ui.py
```
