#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import signal
import sqlite3
import subprocess
import tempfile
import time
import uuid
from pathlib import Path

import pyatspi


REPO_ROOT = Path(__file__).resolve().parents[1]
HYDRA_BIN = REPO_ROOT / "build" / "debug" / "hydra"


def wait_for(predicate, timeout=30.0, interval=0.25, message="condition"):
    deadline = time.time() + timeout
    last_error = None
    while time.time() < deadline:
        try:
            value = predicate()
            if value:
                return value
        except Exception as exc:  # pragma: no cover - best-effort diagnostics
            last_error = exc
        time.sleep(interval)
    raise RuntimeError(f"Timed out waiting for {message}: {last_error}")


def run(command, *, cwd=None, env=None, capture_output=False, check=True):
    return subprocess.run(
        command,
        cwd=cwd,
        env=env,
        check=check,
        text=True,
        capture_output=capture_output,
    )


def find_accessible(node, predicate):
    try:
        if predicate(node):
            return node
    except Exception:
        pass

    try:
        child_count = node.childCount
    except Exception:
        child_count = 0

    for index in range(child_count):
        found = find_accessible(node[index], predicate)
        if found is not None:
            return found

    return None


def wait_for_application(pid, timeout=30.0):
    desktop = pyatspi.Registry.getDesktop(0)

    def lookup():
        for candidate in desktop:
            try:
                if candidate.get_process_id() == pid:
                    return candidate
            except Exception:
                continue
        return None

    return wait_for(lookup, timeout=timeout, message=f"Hydra accessibility app for pid {pid}")


def button_names(application):
    names = []

    def collect(node):
        try:
            if node.getRoleName() == "button":
                name = getattr(node, "name", "") or ""
                if name:
                    names.append(name)
        except Exception:
            return

        try:
            child_count = node.childCount
        except Exception:
            child_count = 0

        for index in range(child_count):
            collect(node[index])

    collect(application)
    return sorted(set(names))


def invoke_button(application, name, timeout=15.0):
    button = wait_for(
        lambda: find_accessible(application, lambda node: (getattr(node, "name", "") or "") == name),
        timeout=timeout,
        message=f"button {name}",
    )
    button.queryAction().doAction(0)
    return button


def latest_session_row(database_path):
    connection = sqlite3.connect(database_path)
    try:
        return connection.execute(
            "SELECT name, state, working_directory, tmux_session_name "
            "FROM sessions ORDER BY created_at DESC LIMIT 1"
        ).fetchone()
    finally:
        connection.close()


def has_live_tmux_session(session_name):
    result = run(
        ["tmux", "list-sessions", "-F", "#{session_name}"],
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        return False
    return session_name in result.stdout.splitlines()


def initialize_database(xdg_data_home):
    env = os.environ.copy()
    env["XDG_DATA_HOME"] = str(xdg_data_home)
    run(
        ["timeout", "2", str(HYDRA_BIN), "-platform", "offscreen"],
        cwd=REPO_ROOT,
        env=env,
        check=False,
    )


def insert_repository(database_path, repository_path):
    connection = sqlite3.connect(database_path)
    try:
        repository_id = str(uuid.uuid4())
        timestamp = "2099-01-01T00:00:00.000Z"
        connection.execute(
            "INSERT INTO repos (id, name, path, description, accent_color, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            (
                repository_id,
                "UI Test Repo",
                str(repository_path),
                "Disposable repo for Wayland UI validation.",
                "#4d7a58",
                timestamp,
                timestamp,
            ),
        )
        connection.commit()
        return repository_id
    finally:
        connection.close()


def create_temp_repo(base_directory):
    repo_path = base_directory / "ui-test-repo"
    repo_path.mkdir(parents=True, exist_ok=True)
    run(["git", "init", "-b", "main"], cwd=repo_path, capture_output=True)
    run(["git", "config", "user.name", "Hydra Test"], cwd=repo_path)
    run(["git", "config", "user.email", "hydra-test@example.com"], cwd=repo_path)
    (repo_path / "README.md").write_text("ui validation\n", encoding="utf-8")
    run(["git", "add", "README.md"], cwd=repo_path)
    run(["git", "commit", "-m", "init"], cwd=repo_path, capture_output=True)

    worktree_branch = "ui-existing-worktree"
    worktree_path = repo_path.parent / f"{repo_path.name}-{worktree_branch}"
    run(["git", "worktree", "add", "-b", worktree_branch, str(worktree_path), "HEAD"], cwd=repo_path)

    return repo_path, worktree_branch, worktree_path


def launch_hydra(xdg_data_home):
    env = os.environ.copy()
    env["XDG_DATA_HOME"] = str(xdg_data_home)
    env["QT_LINUX_ACCESSIBILITY_ALWAYS_ON"] = "1"
    return subprocess.Popen(
        [str(HYDRA_BIN)],
        cwd=REPO_ROOT,
        env=env,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def stop_process(process):
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def cleanup_repo(repo_path, branch_name, worktree_path):
    if worktree_path.exists():
        run(
            ["git", "-C", str(repo_path), "worktree", "remove", "--force", str(worktree_path)],
            check=False,
        )
    run(["git", "-C", str(repo_path), "branch", "-D", branch_name], check=False)


def main():
    parser = argparse.ArgumentParser(description="Validate the Hydra Wayland UI via AT-SPI.")
    parser.add_argument("--keep-temp", action="store_true", help="Preserve temporary files after the run.")
    parser.add_argument("--json", action="store_true", help="Emit the final report as JSON only.")
    args = parser.parse_args()

    base_directory = Path(tempfile.mkdtemp(prefix="hydra-ui-wayland-"))
    xdg_data_home = base_directory / "data"
    xdg_data_home.mkdir(parents=True, exist_ok=True)
    database_path = xdg_data_home / "Hydra" / "Hydra V2" / "hydra.sqlite3"

    report = {
        "temp_root": str(base_directory),
        "xdg_data_home": str(xdg_data_home),
        "database_path": str(database_path),
        "checks": [],
    }

    hydra_process = None
    repo_path = None
    worktree_path = None
    worktree_branch = None
    tmux_session_name = None

    try:
        repo_path, worktree_branch, worktree_path = create_temp_repo(base_directory)
        report["repo_path"] = str(repo_path)
        report["precreated_worktree_path"] = str(worktree_path)
        report["precreated_worktree_branch"] = worktree_branch

        initialize_database(xdg_data_home)
        insert_repository(database_path, repo_path)

        hydra_process = launch_hydra(xdg_data_home)
        application = wait_for_application(hydra_process.pid)

        buttons = button_names(application)
        report["accessible_buttons_initial"] = buttons
        report["checks"].append(
            {"name": "atspi_button_inventory", "ok": True, "details": buttons}
        )

        invoke_button(application, "Repository UI Test Repo")
        wait_for(lambda: (repo_path / ".hydra").exists(), timeout=10.0, message=".hydra creation")
        exclude_text = (repo_path / ".git" / "info" / "exclude").read_text(encoding="utf-8")
        report["checks"].append(
            {
                "name": "repo_selection_triggers_workspace_bootstrap",
                "ok": ".hydra/" in exclude_text,
                "details": {
                    "hydra_directory_exists": (repo_path / ".hydra").exists(),
                    "git_exclude_contains_hydra": ".hydra/" in exclude_text,
                },
            }
        )

        invoke_button(application, f"Worktree {worktree_branch}")
        report["checks"].append(
            {"name": "worktree_button_present_and_invokable", "ok": True, "details": worktree_branch}
        )

        invoke_button(application, "Launch tmux shell")
        wait_for(lambda: latest_session_row(database_path) is not None, message="session row")
        wait_for(
            lambda: latest_session_row(database_path)[1] == "idle",
            message="idle session state",
        )
        session_row = latest_session_row(database_path)
        tmux_session_name = session_row[3]
        report["checks"].append(
            {
                "name": "ui_launch_persists_idle_session",
                "ok": session_row[2] == str(worktree_path),
                "details": {
                    "session_name": session_row[0],
                    "state": session_row[1],
                    "working_directory": session_row[2],
                    "tmux_session_name": tmux_session_name,
                },
            }
        )

        stop_process(hydra_process)
        hydra_process = launch_hydra(xdg_data_home)
        application = wait_for_application(hydra_process.pid)
        restarted_buttons = button_names(application)
        report["accessible_buttons_after_restart"] = restarted_buttons
        report["checks"].append(
            {
                "name": "restart_reload_surfaces_active_session",
                "ok": any(name.startswith("End session ") for name in restarted_buttons),
                "details": restarted_buttons,
            }
        )

        invoke_button(application, "Refresh Hydra state")
        report["checks"].append(
            {
                "name": "refresh_button_invokable",
                "ok": latest_session_row(database_path)[1] == "idle",
                "details": latest_session_row(database_path)[1],
            }
        )

        end_button_name = next(
            name for name in button_names(application) if name.startswith("End session ")
        )
        invoke_button(application, end_button_name)
        wait_for(
            lambda: latest_session_row(database_path)[1] == "exited",
            message="exited session state",
        )
        report["checks"].append(
            {
                "name": "ui_end_marks_session_exited_and_kills_tmux",
                "ok": not has_live_tmux_session(tmux_session_name),
                "details": {
                    "final_state": latest_session_row(database_path)[1],
                    "tmux_session_live": has_live_tmux_session(tmux_session_name),
                },
            }
        )

        report["checks"].append(
            {
                "name": "ui_create_worktree_text_entry",
                "ok": False,
                "details": (
                    "Blocked in current automation. The branch-name field is discoverable and focusable "
                    "through AT-SPI, but text injection did not succeed via AT-SPI, dogtail rawinput, "
                    "or ydotool on this GNOME Wayland stack."
                ),
            }
        )

    finally:
        if hydra_process is not None:
            stop_process(hydra_process)
        if tmux_session_name:
            run(["tmux", "kill-session", "-t", tmux_session_name], check=False)
        if repo_path is not None and worktree_branch is not None and worktree_path is not None:
            cleanup_repo(repo_path, worktree_branch, worktree_path)
        if not args.keep_temp:
            shutil.rmtree(base_directory, ignore_errors=True)

    overall_ok = all(check["ok"] for check in report["checks"] if check["name"] != "ui_create_worktree_text_entry")
    report["overall_ok"] = overall_ok

    if args.json:
        print(json.dumps(report, indent=2))
        return

    print(f"Temp root: {report['temp_root']}")
    for check in report["checks"]:
        status = "PASS" if check["ok"] else "BLOCKED"
        print(f"{status} {check['name']}")
        print(json.dumps(check["details"], indent=2, sort_keys=True))
    print(f"OVERALL {'PASS' if overall_ok else 'FAIL'}")


if __name__ == "__main__":
    main()
