#!/usr/bin/env python3

import argparse
import json
import shutil
import tempfile
import time
from pathlib import Path
import subprocess

import validate_wayland_ui as harness
from ui_fixture import BRANCH_NAME, REPO_NAME, ensure_git_repo, inject_repository_row, precreate_worktree
from validate_divider_drag_x11 import HydraRunnerX11, capture_window, click_at, xdotool

HOVER_HINT_DELAY_SECONDS = 1.7


class CaptureError(RuntimeError):
    pass


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Capture actual X11 help and hover states for Hydra.")
    parser.add_argument(
        "--hydra-bin",
        type=Path,
        default=repo_root / "build" / "debug" / "hydra",
        help="Path to the hydra desktop binary.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=repo_root / ".runtime" / "ui-captures" / "guidance-x11",
        help="Directory to write window captures and manifest data.",
    )
    return parser.parse_args()


def wait_for_geometry(name: str, pid: int, timeout: float = 10.0) -> dict[str, int]:
    def locate():
        node = harness.find_by_name(name, pid)
        if node is None:
            return None
        geometry = harness.node_extents(node)
        if not harness.geometry_visible(geometry):
            return None
        return geometry

    return harness.wait_until(locate, timeout=timeout)


def main() -> int:
    args = parse_args()
    if not args.hydra_bin.exists():
        raise CaptureError(f"Hydra binary not found: {args.hydra_bin}")

    root = Path(tempfile.mkdtemp(prefix="hydra-guidance-x11-"))
    repo_path = root / "repo"
    worktree_path = root / f"{repo_path.name}-{BRANCH_NAME}"
    app_data = root / "data"
    db_path = app_data / "Hydra" / "Hydra V2" / "hydra.sqlite3"
    output_dir = args.output_dir

    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    try:
        ensure_git_repo(repo_path)
        precreate_worktree(repo_path, worktree_path)

        runner = HydraRunnerX11(args.hydra_bin, app_data)
        try:
            runner.start()
            runner.stop()

            inject_repository_row(db_path, repo_path)

            runner.start()
            window_id = runner.window_id()
            repo_button_name = f"Repository {REPO_NAME}"
            launch_name = "Launch tmux shell"
            create_name = "Create worktree"
            empty_branch_message = "Branch name is required to create a worktree."
            target_help_name = "Explain target map"
            quick_detail_name = "Open detailed help"
            detail_close_name = "Close detailed help"

            repo_button = harness.wait_until(lambda: harness.find_by_name(repo_button_name, runner.pid), timeout=10)
            harness.press(repo_button)
            time.sleep(1.0)

            launch_geometry = wait_for_geometry(launch_name, runner.pid)
            create_geometry = wait_for_geometry(create_name, runner.pid)
            target_help_geometry = wait_for_geometry(target_help_name, runner.pid)

            click_at(window_id, create_geometry)
            harness.wait_until(lambda: harness.find_by_name(empty_branch_message, runner.pid), timeout=10)
            time.sleep(0.5)
            capture_window(window_id, output_dir / "empty-worktree-warning.png")
            harness.wait_until(lambda: harness.find_by_name(empty_branch_message, runner.pid) is None, timeout=8)

            xdotool("windowactivate", "--sync", window_id)
            xdotool(
                "mousemove",
                "--sync",
                str(launch_geometry["x"] + max(1, launch_geometry["width"] // 2)),
                str(launch_geometry["y"] + max(1, launch_geometry["height"] // 2)),
            )
            time.sleep(HOVER_HINT_DELAY_SECONDS)
            capture_window(window_id, output_dir / "launch-hover.png")

            xdotool(
                "mousemove",
                "--sync",
                str(target_help_geometry["x"] + max(1, target_help_geometry["width"] // 2)),
                str(target_help_geometry["y"] + max(1, target_help_geometry["height"] // 2)),
            )
            time.sleep(HOVER_HINT_DELAY_SECONDS)
            capture_window(window_id, output_dir / "target-help-hover.png")

            click_at(window_id, target_help_geometry)
            harness.wait_until(lambda: harness.find_by_name(quick_detail_name, runner.pid), timeout=10)
            time.sleep(0.5)
            capture_window(window_id, output_dir / "target-help-quick.png")

            quick_detail_button = harness.find_by_name(quick_detail_name, runner.pid)
            if quick_detail_button is None:
                raise CaptureError("Detailed help button not found after opening quick help")
            harness.press(quick_detail_button)
            harness.wait_until(lambda: harness.find_by_name(detail_close_name, runner.pid), timeout=10)
            time.sleep(0.5)
            capture_window(window_id, output_dir / "target-help-detail.png")

            detail_close_button = harness.find_by_name(detail_close_name, runner.pid)
            if detail_close_button is None:
                raise CaptureError("Close detailed help button not found")
            harness.press(detail_close_button)
            harness.wait_until(lambda: harness.named_node_hidden(detail_close_name, runner.pid), timeout=6)

            contact_sheet = output_dir / "contact-sheet.png"
            montage = shutil.which("montage")
            if montage is not None:
                subprocess.run(
                    [
                        montage,
                        str(output_dir / "empty-worktree-warning.png"),
                        str(output_dir / "launch-hover.png"),
                        str(output_dir / "target-help-hover.png"),
                        str(output_dir / "target-help-quick.png"),
                        str(output_dir / "target-help-detail.png"),
                        "-tile",
                        "3x2",
                        "-geometry",
                        "+20+20",
                        "-background",
                        "#06080a",
                        str(contact_sheet),
                    ],
                    check=True,
                    capture_output=True,
                    text=True,
                )

            manifest = {
                "tmpdir": str(root),
                "repo": str(repo_path),
                "worktree": str(worktree_path),
                "db_path": str(db_path),
                "window_id": window_id,
                "captures": {
                    "empty_worktree_warning": str(output_dir / "empty-worktree-warning.png"),
                    "launch_hover": str(output_dir / "launch-hover.png"),
                    "target_help_hover": str(output_dir / "target-help-hover.png"),
                    "target_help_quick": str(output_dir / "target-help-quick.png"),
                    "target_help_detail": str(output_dir / "target-help-detail.png"),
                },
            }
            if contact_sheet.exists():
                manifest["contact_sheet"] = str(contact_sheet)
            (output_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
            return 0
        finally:
            runner.stop()
    finally:
        shutil.rmtree(root, ignore_errors=True)


if __name__ == "__main__":
    raise SystemExit(main())
