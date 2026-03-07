#!/usr/bin/env python3

from __future__ import annotations

import sqlite3
import subprocess
import uuid
from datetime import datetime, timedelta, timezone
from pathlib import Path


REPO_NAME = "Validation Repo"
BRANCH_NAME = "ui-precreated"


def iso_now(offset_seconds: int = 0) -> str:
    value = datetime.now(timezone.utc) + timedelta(seconds=offset_seconds)
    return value.isoformat(timespec="milliseconds").replace("+00:00", "Z")


def ensure_git_repo(repo_path: Path) -> None:
    repo_path.mkdir(parents=True, exist_ok=True)
    subprocess.run(["git", "init", "-q"], cwd=repo_path, check=True)
    subprocess.run(["git", "config", "user.email", "hydra-test@example.com"], cwd=repo_path, check=True)
    subprocess.run(["git", "config", "user.name", "Hydra Test"], cwd=repo_path, check=True)
    (repo_path / "README.md").write_text("seed\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_path, check=True)
    subprocess.run(["git", "commit", "-q", "-m", "seed"], cwd=repo_path, check=True)


def precreate_worktree(repo_path: Path, worktree_path: Path, branch_name: str = BRANCH_NAME) -> None:
    subprocess.run(
        ["git", "worktree", "add", "-q", "-b", branch_name, str(worktree_path), "HEAD"],
        cwd=repo_path,
        check=True,
    )


def inject_repository_row(
    db_path: Path,
    repo_path: Path,
    *,
    repo_name: str = REPO_NAME,
    description: str = "Temporary validation repository",
    accent_color: str = "#b37748",
    updated_at: str | None = None,
    created_at: str | None = None,
) -> str:
    repo_id = str(uuid.uuid4())
    created = created_at or iso_now()
    updated = updated_at or created

    conn = sqlite3.connect(db_path)
    conn.execute(
        "INSERT INTO repos (id, name, path, description, accent_color, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)",
        (
            repo_id,
            repo_name,
            str(repo_path),
            description,
            accent_color,
            created,
            updated,
        ),
    )
    conn.commit()
    conn.close()
    return repo_id


def insert_session_row(
    db_path: Path,
    *,
    repo_id: str,
    name: str,
    provider_key: str,
    state: str,
    tmux_session_name: str,
    tmux_pane_id: str,
    working_directory: Path,
    last_error: str = "",
    updated_at: str | None = None,
    created_at: str | None = None,
) -> str:
    session_id = str(uuid.uuid4())
    created = created_at or iso_now()
    updated = updated_at or created

    conn = sqlite3.connect(db_path)
    conn.execute(
        "INSERT INTO sessions (id, repo_id, name, provider_key, state, tmux_session_name, "
        "tmux_pane_id, working_directory, last_error, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        (
            session_id,
            repo_id,
            name,
            provider_key,
            state,
            tmux_session_name,
            tmux_pane_id,
            str(working_directory),
            last_error,
            created,
            updated,
        ),
    )
    conn.commit()
    conn.close()
    return session_id


def create_tmux_session(session_name: str, working_directory: Path) -> str:
    subprocess.run(
        ["tmux", "new-session", "-d", "-s", session_name, "-c", str(working_directory)],
        check=True,
    )
    pane_id = subprocess.check_output(
        ["tmux", "list-panes", "-t", session_name, "-F", "#{pane_id}"],
        text=True,
    ).strip()
    return pane_id


def kill_tmux_session(session_name: str) -> None:
    subprocess.run(["tmux", "kill-session", "-t", session_name], check=False)
