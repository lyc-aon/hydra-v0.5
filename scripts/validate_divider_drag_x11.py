#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import validate_wayland_ui as harness
from ui_fixture import BRANCH_NAME, REPO_NAME, ensure_git_repo, inject_repository_row, precreate_worktree


class X11ValidationError(RuntimeError):
    pass


class HydraRunnerX11:
    def __init__(self, hydra_bin: Path, app_data: Path) -> None:
        self.hydra_bin = hydra_bin
        self.app_data = app_data
        self.proc: subprocess.Popen[str] | None = None

    def start(self) -> None:
        env = os.environ.copy()
        env["QT_LINUX_ACCESSIBILITY_ALWAYS_ON"] = "1"
        env["QT_QPA_PLATFORM"] = "xcb"
        env["XDG_DATA_HOME"] = str(self.app_data)
        self.proc = subprocess.Popen(
            [str(self.hydra_bin)],
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        harness.wait_for_app()
        time.sleep(1.0)

    def stop(self) -> None:
        if self.proc is None:
            return
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=5)
        self.proc = None

    @property
    def pid(self) -> int | None:
        return self.proc.pid if self.proc is not None else None

    def window_id(self) -> str:
        if self.proc is None:
            raise X11ValidationError("Hydra is not running")
        output = subprocess.check_output(
            [
                "xdotool",
                "search",
                "--sync",
                "--onlyvisible",
                "--pid",
                str(self.proc.pid),
                "--name",
                "Hydra V2",
            ],
            text=True,
        )
        window_ids = [line.strip() for line in output.splitlines() if line.strip()]
        if not window_ids:
            raise X11ValidationError("Could not find Hydra X11 window")
        return window_ids[-1]


def xdotool(*args: str) -> None:
    subprocess.run(["xdotool", *args], check=True, capture_output=True, text=True)


def capture_window(window_id: str, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["import", "-window", window_id, str(output_path)], check=True, capture_output=True, text=True)


def click_at(window_id: str, geometry: dict[str, int]) -> None:
    x = geometry["x"] + max(1, geometry["width"] // 2)
    y = geometry["y"] + max(1, geometry["height"] // 2)
    xdotool("windowactivate", "--sync", window_id)
    xdotool("mousemove", "--sync", str(x), str(y))
    time.sleep(0.15)
    xdotool("mousedown", "1")
    time.sleep(0.12)
    xdotool("mouseup", "1")
    time.sleep(0.7)


def drag_at(window_id: str, geometry: dict[str, int], delta: int) -> None:
    start_x = geometry["x"] + max(1, geometry["width"] // 2)
    end_x = start_x + delta
    y = geometry["y"] + max(1, geometry["height"] // 2)
    xdotool("windowactivate", "--sync", window_id)
    xdotool("mousemove", "--sync", str(start_x), str(y))
    time.sleep(0.15)
    xdotool("mousedown", "1")
    time.sleep(0.15)
    steps = 6
    for step in range(1, steps + 1):
        current_x = start_x + round(delta * step / steps)
        xdotool("mousemove", "--sync", str(current_x), str(y))
        time.sleep(0.12)
    xdotool("mouseup", "1")
    time.sleep(0.7)


def validate(results: dict, hydra_bin: Path, root: Path) -> None:
    repo_path = root / "repo"
    worktree_path = root / f"{repo_path.name}-{BRANCH_NAME}"
    app_data = root / "data"
    db_path = app_data / "Hydra" / "Hydra V2" / "hydra.sqlite3"
    capture_dir = root / "captures"

    ensure_git_repo(repo_path)
    precreate_worktree(repo_path, worktree_path)

    results["repo"] = str(repo_path)
    results["precreated_worktree"] = str(worktree_path)
    results["db_path"] = str(db_path)

    repo_button_name = f"Repository {REPO_NAME}"
    launch_name = "Launch tmux shell"
    toggle_name = "Toggle navigation rail"

    def wait_for_geometry(name: str, predicate=None, timeout: float = 10.0) -> dict[str, int]:
        def locate() -> dict[str, int] | None:
            node = harness.find_by_name(name, runner.pid)
            if node is None:
                return None
            geometry = harness.node_extents(node)
            if predicate is not None and not predicate(geometry):
                return None
            return geometry

        return harness.wait_until(locate, timeout=timeout)

    runner = HydraRunnerX11(hydra_bin, app_data)
    try:
        runner.start()
        harness.record_check(results, "bootstrap app launch", db_path.exists(), str(db_path))
        runner.stop()

        inject_repository_row(db_path, repo_path)
        harness.record_check(results, "temp repo inserted", True, REPO_NAME)

        runner.start()
        window_id = runner.window_id()
        results["window_id"] = window_id

        repo_button = harness.wait_until(lambda: harness.find_by_name(repo_button_name, runner.pid), timeout=10)
        harness.press(repo_button)
        time.sleep(1.0)

        launch_geometry = wait_for_geometry(launch_name, predicate=harness.geometry_visible)
        toggle_geometry = wait_for_geometry(toggle_name, predicate=harness.geometry_visible)
        harness.record_check(results, "launch control visible before resize", harness.geometry_visible(launch_geometry), launch_geometry)
        harness.record_check(results, "divider control visible before resize", harness.geometry_visible(toggle_geometry), toggle_geometry)
        results["layout"][f"initial::{launch_name}"] = launch_geometry
        results["layout"][f"initial::{toggle_name}"] = toggle_geometry
        capture_window(window_id, capture_dir / "initial.png")

        drag_at(window_id, toggle_geometry, delta=96)
        resized_launch = wait_for_geometry(
            launch_name,
            predicate=lambda geometry: harness.geometry_visible(geometry) and geometry["width"] > launch_geometry["width"],
        )
        resized_toggle = wait_for_geometry(
            toggle_name,
            predicate=lambda geometry: harness.geometry_visible(geometry) and geometry["x"] > toggle_geometry["x"],
        )
        harness.record_check(
            results,
            "launch control widens when the divider is dragged right",
            resized_launch["width"] > launch_geometry["width"],
            {"initial": launch_geometry, "resized": resized_launch},
        )
        harness.record_check(
            results,
            "divider control moves right when the rail widens",
            resized_toggle["x"] > toggle_geometry["x"],
            {"initial": toggle_geometry, "resized": resized_toggle},
        )
        results["layout"][f"resized::{launch_name}"] = resized_launch
        results["layout"][f"resized::{toggle_name}"] = resized_toggle
        capture_window(window_id, capture_dir / "resized.png")

        drag_at(window_id, resized_toggle, delta=-72)
        narrowed_launch = wait_for_geometry(
            launch_name,
            predicate=lambda geometry: harness.geometry_visible(geometry) and geometry["width"] < resized_launch["width"],
        )
        narrowed_toggle = wait_for_geometry(
            toggle_name,
            predicate=lambda geometry: harness.geometry_visible(geometry) and geometry["x"] < resized_toggle["x"],
        )
        harness.record_check(
            results,
            "launch control narrows when the divider is dragged left",
            narrowed_launch["width"] < resized_launch["width"],
            {"resized": resized_launch, "narrowed": narrowed_launch},
        )
        harness.record_check(
            results,
            "divider control moves left when the rail narrows",
            narrowed_toggle["x"] < resized_toggle["x"],
            {"resized": resized_toggle, "narrowed": narrowed_toggle},
        )
        results["layout"][f"narrowed::{launch_name}"] = narrowed_launch
        results["layout"][f"narrowed::{toggle_name}"] = narrowed_toggle
        capture_window(window_id, capture_dir / "narrowed.png")
        results["captures"] = {
            "initial": str(capture_dir / "initial.png"),
            "resized": str(capture_dir / "resized.png"),
            "narrowed": str(capture_dir / "narrowed.png"),
        }

        results["limits"].append(
            "Pointer drag validation used Hydra under Xwayland with xdotool because GNOME Wayland blocks the synthetic pointer stream needed for resize automation."
        )
        results["limits"].append(
            "Collapse and restore are validated semantically in scripts/validate_wayland_ui.py; this X11 harness is now limited to pointer drag resizing because that is the interaction GNOME Wayland blocks."
        )
    finally:
        runner.stop()


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Validate Hydra divider drag under Xwayland with xdotool.")
    parser.add_argument(
        "--hydra-bin",
        type=Path,
        default=repo_root / "build" / "debug" / "hydra",
        help="Path to the hydra desktop binary.",
    )
    parser.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep the temporary repo and XDG_DATA_HOME for inspection.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if not args.hydra_bin.exists():
        raise X11ValidationError(f"Hydra binary not found: {args.hydra_bin}")

    root = Path(tempfile.mkdtemp(prefix="hydra-x11-divider-validation-"))
    results = {
        "tmpdir": str(root),
        "checks": [],
        "layout": {},
        "limits": [],
    }

    try:
        validate(results, args.hydra_bin, root)
        print(json.dumps(results, indent=2))
        if not args.keep_temp:
            shutil.rmtree(root)
        return 0
    except Exception as exc:
        print(json.dumps(results, indent=2))
        print(f"validation failed: {exc}", file=sys.stderr)
        print(f"temporary artifacts preserved at: {root}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
