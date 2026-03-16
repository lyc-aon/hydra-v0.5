#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import pyatspi

from ui_fixture import BRANCH_NAME, REPO_NAME, ensure_git_repo, inject_repository_row, precreate_worktree


class ValidationError(RuntimeError):
    pass


class HydraRunner:
    def __init__(self,
                 hydra_bin: Path,
                 app_data: Path,
                 *,
                 width: int | None = None,
                 height: int | None = None) -> None:
        self.hydra_bin = hydra_bin
        self.app_data = app_data
        self.width = width
        self.height = height
        self.proc: subprocess.Popen[str] | None = None
        self.stdout_log = self.app_data / "hydra-validation.stdout.log"
        self.stderr_log = self.app_data / "hydra-validation.stderr.log"
        self._stdout_handle = None
        self._stderr_handle = None

    def start(self) -> None:
        env = os.environ.copy()
        env["QT_LINUX_ACCESSIBILITY_ALWAYS_ON"] = "1"
        env["HYDRA_DISABLE_SOUND"] = "1"
        env["XDG_DATA_HOME"] = str(self.app_data)
        env["XDG_CONFIG_HOME"] = str(self.app_data / "config")
        self.app_data.mkdir(parents=True, exist_ok=True)
        cmd = [str(self.hydra_bin), "--skip-boot"]
        if self.width is not None:
            cmd.extend(["--window-width", str(self.width)])
        if self.height is not None:
            cmd.extend(["--window-height", str(self.height)])
        self._stdout_handle = self.stdout_log.open("a", encoding="utf-8")
        self._stderr_handle = self.stderr_log.open("a", encoding="utf-8")
        self.proc = subprocess.Popen(
            cmd,
            env=env,
            stdout=self._stdout_handle,
            stderr=self._stderr_handle,
        )
        wait_for_app(self.pid)

    def stop(self) -> None:
        if self.proc is None:
            self._close_logs()
            return
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=5)
        self.proc = None
        self._close_logs()

    @property
    def pid(self) -> int | None:
        return self.proc.pid if self.proc is not None else None

    def _close_logs(self) -> None:
        if self._stdout_handle is not None:
            self._stdout_handle.close()
            self._stdout_handle = None
        if self._stderr_handle is not None:
            self._stderr_handle.close()
            self._stderr_handle = None


def wait_until(predicate, timeout: float = 10.0, step: float = 0.2):
    deadline = time.time() + timeout
    last = None
    while time.time() < deadline:
        try:
            value = predicate()
            if value:
                return value
            last = value
        except Exception as exc:  # pragma: no cover - diagnostics path
            last = exc
        time.sleep(step)
    raise TimeoutError(str(last))


def hydra_app(pid: int | None = None):
    desktop = pyatspi.Registry.getDesktop(0)
    for candidate in desktop:
        if (getattr(candidate, "name", "") or "").lower() != "hydra v2":
            continue
        if pid is not None and candidate.get_process_id() != pid:
            continue
        return candidate
    return None


def wait_for_app(pid: int | None = None):
    return wait_until(lambda: hydra_app(pid), timeout=15)


def walk(node):
    yield node
    try:
        count = node.childCount
    except Exception:
        count = 0
    for index in range(count):
        yield from walk(node[index])


def find_by_name(name: str, pid: int | None = None):
    app = hydra_app(pid)
    if app is None:
        return None
    for node in walk(app):
        if (getattr(node, "name", "") or "") == name:
            return node
    return None


def find_all_with_prefix(prefix: str, pid: int | None = None):
    app = hydra_app(pid)
    if app is None:
        return []
    matches = []
    for node in walk(app):
        if (getattr(node, "name", "") or "").startswith(prefix):
            matches.append(node)
    return matches


def node_extents(node) -> dict[str, int]:
    try:
        extents = node.queryComponent().getExtents(pyatspi.DESKTOP_COORDS)
        return {
            "x": extents.x,
            "y": extents.y,
            "width": extents.width,
            "height": extents.height,
        }
    except Exception:
        return {
            "x": 0,
            "y": 0,
            "width": 0,
            "height": 0,
        }


def geometry_visible(geometry: dict[str, int]) -> bool:
    return geometry["width"] > 0 and geometry["height"] > 0


def named_node_hidden(name: str, pid: int | None = None) -> bool:
    node = find_by_name(name, pid)
    if node is None:
        return True
    return not geometry_visible(node_extents(node))


def press(node) -> None:
    if node is None:
        raise ValidationError("expected UI node but found none before press()")
    node.queryAction().doAction(0)


def refresh_cycle_completed(refresh_name: str, pid: int | None = None) -> bool:
    refreshing = find_by_name("Refreshing Hydra state", pid)
    if refreshing is not None:
        return True
    return find_by_name(refresh_name, pid) is not None


def tmux_sessions() -> set[str]:
    try:
        output = subprocess.check_output(
            ["tmux", "list-sessions", "-F", "#{session_name}"], text=True
        )
    except subprocess.CalledProcessError:
        return set()
    return {line.strip() for line in output.splitlines() if line.strip()}


def session_rows(db_path: Path):
    conn = sqlite3.connect(db_path)
    rows = list(
        conn.execute(
            "SELECT name, state, tmux_session_name, working_directory, status_detail "
            "FROM sessions ORDER BY created_at DESC"
        )
    )
    conn.close()
    return rows


def repository_rows(db_path: Path):
    conn = sqlite3.connect(db_path)
    rows = list(
        conn.execute(
            "SELECT id, name, path FROM repos ORDER BY updated_at DESC, name ASC"
        )
    )
    conn.close()
    return rows


def record_check(results: dict, name: str, ok: bool, detail) -> None:
    results["checks"].append({"name": name, "ok": bool(ok), "detail": detail})
    if not ok:
        raise ValidationError(f"{name}: {detail}")


def validate_ui(results: dict, hydra_bin: Path, root: Path) -> None:
    repo_path = root / "repo"
    worktree_path = root / f"{repo_path.name}-{BRANCH_NAME}"
    app_data = root / "data"
    db_path = app_data / "Hydra" / "Hydra V2" / "hydra.sqlite3"

    ensure_git_repo(repo_path)
    precreate_worktree(repo_path, worktree_path)

    results["repo"] = str(repo_path)
    results["precreated_worktree"] = str(worktree_path)
    results["db_path"] = str(db_path)

    runner = HydraRunner(hydra_bin, app_data)
    results["stdout_log"] = str(runner.stdout_log)
    results["stderr_log"] = str(runner.stderr_log)
    repo_button_name = f"Repository {REPO_NAME}"
    worktree_button_name = f"Worktree {BRANCH_NAME}"
    launch_name = "Launch tmux shell"
    refresh_name = "Refresh Hydra state"
    theme_default_name = "Theme cycle STEAM"
    theme_alt_name = "Theme cycle HERMES"
    empty_branch_message = "Branch name is required to create a worktree."
    launched_session_name = f"{REPO_NAME} [Codex]"
    end_name = f"End session {launched_session_name}"
    trace_name = f"Show trace for {launched_session_name}"
    trace_hide_name = f"Hide trace for {launched_session_name}"
    remove_repo_name = f"Remove repository {REPO_NAME}"
    remove_worktree_name = f"Remove worktree {BRANCH_NAME}"
    worktree_toggle_name = "Toggle worktrees"
    toggle_name = "Toggle navigation rail"
    target_help_name = "Explain target map"
    quick_help_open_name = "Open detailed help"
    quick_help_dismiss_name = "Dismiss quick help"
    detail_help_close_name = "Close detailed help"

    runner.start()
    record_check(results, "bootstrap app launch", db_path.exists(), str(db_path))
    runner.stop()

    inject_repository_row(db_path, repo_path)
    record_check(results, "temp repo inserted", True, REPO_NAME)

    runner.start()
    repo_button = wait_until(lambda: find_by_name(repo_button_name, runner.pid), timeout=10)
    record_check(results, "temp repo visible in UI", repo_button is not None, repo_button_name)
    press(repo_button)
    time.sleep(1.0)

    hydra_dir = repo_path / ".hydra"
    record_check(results, "repo local state directory exists", wait_until(hydra_dir.is_dir, timeout=5), str(hydra_dir))
    record_check(results, "repo docs directory exists", (hydra_dir / "docs").is_dir(), str(hydra_dir / "docs"))
    record_check(results, "repo local config exists", (hydra_dir / "local.json").is_file(), str(hydra_dir / "local.json"))
    exclude_text = (repo_path / ".git" / "info" / "exclude").read_text(encoding="utf-8")
    record_check(results, "git exclude wired", ".hydra/" in exclude_text, ".hydra/ present")

    for name in [
        repo_button_name,
        remove_repo_name,
        launch_name,
        refresh_name,
        theme_default_name,
        toggle_name,
        worktree_toggle_name,
        target_help_name,
    ]:
        node = wait_until(lambda target=name: find_by_name(target, runner.pid), timeout=10)
        geometry = node_extents(node)
        record_check(
            results,
            f"{name} has geometry",
            geometry_visible(geometry),
            geometry,
        )
        results["layout"][name] = geometry

    press(wait_until(lambda: find_by_name(theme_default_name, runner.pid), timeout=10))
    wait_until(lambda: find_by_name(theme_alt_name, runner.pid), timeout=8)
    record_check(results, "theme cycles to alternate palette", True, theme_alt_name)

    press(wait_until(lambda: find_by_name(worktree_toggle_name, runner.pid), timeout=10))
    for name in [
        worktree_button_name,
        remove_worktree_name,
        "Create worktree",
        "Worktree branch name",
    ]:
        node = wait_until(lambda target=name: find_by_name(target, runner.pid), timeout=10)
        geometry = node_extents(node)
        record_check(
            results,
            f"{name} has geometry after expanding worktrees",
            geometry_visible(geometry),
            geometry,
        )
        results["layout"][name] = geometry

    press(wait_until(lambda: find_by_name("Create worktree", runner.pid), timeout=10))
    wait_until(lambda: find_by_name(empty_branch_message, runner.pid), timeout=10)
    record_check(results, "empty worktree warning becomes visible", True, empty_branch_message)
    wait_until(lambda: find_by_name(empty_branch_message, runner.pid) is None, timeout=8)
    record_check(results, "empty worktree warning clears", True, empty_branch_message)

    press(wait_until(lambda: find_by_name(target_help_name, runner.pid), timeout=10))
    wait_until(lambda: find_by_name(quick_help_open_name, runner.pid), timeout=10)
    wait_until(lambda: find_by_name(quick_help_dismiss_name, runner.pid), timeout=10)
    record_check(results, "quick help opens from section info button", True, target_help_name)
    press(wait_until(lambda: find_by_name(quick_help_open_name, runner.pid), timeout=10))
    wait_until(lambda: find_by_name(detail_help_close_name, runner.pid), timeout=10)
    record_check(results, "detailed help opens from quick help", True, detail_help_close_name)
    press(wait_until(lambda: find_by_name(detail_help_close_name, runner.pid), timeout=10))
    wait_until(lambda: named_node_hidden(detail_help_close_name, runner.pid), timeout=6)
    record_check(results, "detailed help closes cleanly", True, detail_help_close_name)

    press(wait_until(lambda: find_by_name(worktree_button_name, runner.pid), timeout=10))
    time.sleep(0.5)
    press(wait_until(lambda: find_by_name(launch_name, runner.pid), timeout=10))
    latest = wait_until(lambda: session_rows(db_path)[:1] or None, timeout=10)[0]
    record_check(results, "session row created", latest[0] == launched_session_name and latest[1] == "idle", latest)
    record_check(results, "worktree-bound working directory persisted", latest[3] == str(worktree_path), latest)
    record_check(results, "tmux session live after launch", latest[2] in tmux_sessions(), latest[2])

    matching = [node for node in wait_until(lambda: find_all_with_prefix("End session ", runner.pid), timeout=10)
                if (getattr(node, "name", "") or "") == end_name]
    record_check(results, "single end button after launch", len(matching) == 1, [getattr(node, "name", "") for node in find_all_with_prefix("End session ", runner.pid)])
    trace_button = wait_until(lambda: find_by_name(trace_name, runner.pid), timeout=10)
    record_check(results, "trace button visible after launch", trace_button is not None, trace_name)
    press(trace_button)
    wait_until(lambda: find_by_name(trace_hide_name, runner.pid), timeout=10)
    record_check(results, "trace expands after launch", True, trace_hide_name)
    press(wait_until(lambda: find_by_name(trace_hide_name, runner.pid), timeout=10))
    wait_until(lambda: find_by_name(trace_name, runner.pid), timeout=10)
    record_check(results, "trace collapses after launch", True, trace_name)
    press(wait_until(lambda: find_by_name(refresh_name, runner.pid), timeout=10))
    wait_until(lambda: refresh_cycle_completed(refresh_name, runner.pid), timeout=10)
    record_check(results, "refresh action acknowledged", True, refresh_name)
    wait_until(lambda: find_by_name(refresh_name, runner.pid), timeout=10)
    record_check(results, "refresh returns to idle state", True, refresh_name)
    matching = wait_until(
        lambda: [node for node in find_all_with_prefix("End session ", runner.pid)
                 if (getattr(node, "name", "") or "") == end_name] or None,
        timeout=10,
    )
    record_check(results, "single end button after refresh", len(matching) == 1, [getattr(node, "name", "") for node in find_all_with_prefix("End session ", runner.pid)])

    press(wait_until(lambda: find_by_name(end_name, runner.pid), timeout=10))
    wait_until(
        lambda: runner.proc is not None
        and runner.proc.poll() is None
        and find_by_name(end_name, runner.pid) is None,
        timeout=10,
    )
    record_check(results,
                 "ending a live session keeps Hydra running",
                 runner.proc is not None and runner.proc.poll() is None,
                 runner.proc.poll() if runner.proc is not None else None)
    latest = wait_until(lambda: session_rows(db_path)[:1] or None, timeout=10)[0]
    record_check(results,
                 "end action marks the live session exited",
                 latest[0] == launched_session_name and latest[1] == "exited",
                 latest)
    record_check(results,
                 "tmux session removed after end action",
                 latest[2] not in tmux_sessions(),
                 latest[2])

    press(wait_until(lambda: find_by_name(launch_name, runner.pid), timeout=10))
    latest = wait_until(lambda: session_rows(db_path)[:1] or None, timeout=10)[0]
    record_check(results,
                 "session can relaunch after end action",
                 latest[0] == launched_session_name and latest[1] == "idle",
                 latest)
    record_check(results,
                 "relaunch restores a live tmux session",
                 latest[2] in tmux_sessions(),
                 latest[2])

    runner.stop()

    runner.start()
    wait_until(lambda: find_by_name(theme_alt_name, runner.pid), timeout=8)
    record_check(results, "theme persists across restart", True, theme_alt_name)
    matching = [node for node in find_all_with_prefix("End session ", runner.pid)
                if (getattr(node, "name", "") or "") == end_name]
    record_check(results, "no live end button after restart reload", len(matching) == 0, [getattr(node, "name", "") for node in find_all_with_prefix("End session ", runner.pid)])
    latest = wait_until(lambda: session_rows(db_path)[:1] or None, timeout=10)[0]
    record_check(results,
                 "restart session row marked exited",
                 latest[1] == "exited",
                 latest)
    record_check(results,
                 "restart session detail stays descriptive",
                 latest[4].startswith("Hydra closed this provider session when the app exited"),
                 latest[4])
    record_check(results,
                 "tmux session removed after restart",
                 latest[2] not in tmux_sessions(),
                 latest[2])
    press(wait_until(lambda: find_by_name(refresh_name, runner.pid), timeout=10))
    wait_until(lambda: refresh_cycle_completed(refresh_name, runner.pid), timeout=10)
    wait_until(lambda: find_by_name(refresh_name, runner.pid), timeout=10)
    matching = [node for node in find_all_with_prefix("End session ", runner.pid)
                if (getattr(node, "name", "") or "") == end_name]
    record_check(results, "no live end button after restart refresh", len(matching) == 0, [getattr(node, "name", "") for node in matching])
    runner.stop()

    compact_runner = HydraRunner(hydra_bin, app_data, width=960, height=700)
    compact_runner.start()
    press(wait_until(lambda: find_by_name(repo_button_name, compact_runner.pid), timeout=10))
    compact_targets = [
        repo_button_name,
        worktree_button_name,
        launch_name,
        refresh_name,
        toggle_name,
    ]
    for name in compact_targets:
        node = wait_until(lambda target=name: find_by_name(target, compact_runner.pid), timeout=10)
        geometry = node_extents(node)
        record_check(
            results,
            f"{name} remains visible in compact window",
            geometry_visible(geometry),
            geometry,
        )
        results["layout"][f"compact::{name}"] = geometry
    compact_runner.stop()

    runner.start()
    press(wait_until(lambda: find_by_name(repo_button_name, runner.pid), timeout=10))
    press(wait_until(lambda: find_by_name(worktree_toggle_name, runner.pid), timeout=10))
    remove_worktree_button = wait_until(lambda: find_by_name(remove_worktree_name, runner.pid), timeout=10)
    press(remove_worktree_button)
    record_check(results, "linked worktree removed from disk", wait_until(lambda: not worktree_path.exists(), timeout=10), str(worktree_path))
    record_check(results,
                 "linked worktree disappears from UI",
                 wait_until(lambda: find_by_name(worktree_button_name, runner.pid) is None, timeout=10) is True,
                 worktree_button_name)

    remove_repo_button = wait_until(lambda: find_by_name(remove_repo_name, runner.pid), timeout=10)
    press(remove_repo_button)
    remaining_repos = wait_until(
        lambda: [row for row in repository_rows(db_path) if row[1] == REPO_NAME] == [],
        timeout=10,
    )
    record_check(results, "repository removed from registry", remaining_repos, REPO_NAME)
    record_check(results,
                 "repository disappears from UI",
                 wait_until(lambda: find_by_name(repo_button_name, runner.pid) is None, timeout=10) is True,
                 repo_button_name)
    runner.stop()

    results["limits"].append("AT-SPI reliably drove button interactions on GNOME Wayland.")
    results["limits"].append(
        "The branch text field is visible and measurable, but text entry is still not exposed as a working semantic automation path through AT-SPI on this setup."
    )
    results["limits"].append(
        "UI worktree creation therefore still needs either manual typing or a future stronger text-input automation route."
    )
    results["limits"].append(
        "Divider drag is not semantically automated on GNOME Wayland because the compositor blocks the synthetic pointer stream needed for pointer-only interactions."
    )


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Validate the current Hydra Wayland UI path.")
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
        raise ValidationError(f"Hydra binary not found: {args.hydra_bin}")

    root = Path(tempfile.mkdtemp(prefix="hydra-wayland-validation-"))
    results = {
        "tmpdir": str(root),
        "checks": [],
        "layout": {},
        "limits": [],
    }

    try:
        validate_ui(results, args.hydra_bin, root)
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
    sys.exit(main())
