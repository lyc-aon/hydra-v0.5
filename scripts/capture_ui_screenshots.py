#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import math
import os
import shutil
import subprocess
import tempfile
import uuid
from pathlib import Path

from ui_fixture import (
    BRANCH_NAME,
    REPO_NAME,
    create_tmux_session,
    ensure_git_repo,
    inject_repository_row,
    insert_session_row,
    iso_now,
    kill_tmux_session,
    precreate_worktree,
)


class CaptureError(RuntimeError):
    pass


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def hydra_database_path(app_data: Path) -> Path:
    return app_data / "Hydra" / "Hydra V2" / "hydra.sqlite3"


def run_hydra_capture(
    hydra_bin: Path,
    app_data: Path,
    output_path: Path,
    *,
    width: int,
    height: int,
    start_sidebar_collapsed: bool = False,
    start_sidebar_width: int | None = None,
    open_quick_help_topic: str | None = None,
    open_help_topic: str | None = None,
    start_theme: str | None = None,
    open_session_trace_name: str | None = None,
    timeout: int = 30,
) -> None:
    env = os.environ.copy()
    env["QT_LINUX_ACCESSIBILITY_ALWAYS_ON"] = "1"
    env["HYDRA_DISABLE_SOUND"] = "1"
    env["XDG_DATA_HOME"] = str(app_data)
    env["XDG_CONFIG_HOME"] = str(app_data / "config")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(hydra_bin),
        "--skip-boot",
        "-platform",
        "offscreen",
        "--screenshot",
        str(output_path),
        "--window-width",
        str(width),
        "--window-height",
        str(height),
        "--screenshot-delay-ms",
        "2500",
        "--quit-after-screenshot",
    ]
    if start_sidebar_collapsed:
        cmd.append("--start-sidebar-collapsed")
    if start_sidebar_width is not None and start_sidebar_width > 0:
        cmd.extend(["--start-sidebar-width", str(start_sidebar_width)])
    if open_quick_help_topic:
        cmd.extend(["--open-quick-help-topic", open_quick_help_topic])
    if open_help_topic:
        cmd.extend(["--open-help-topic", open_help_topic])
    if start_theme:
        cmd.extend(["--start-theme", start_theme])
    if open_session_trace_name:
        cmd.extend(["--open-session-trace-name", open_session_trace_name])
    proc = subprocess.run(
        cmd,
        env=env,
        text=True,
        capture_output=True,
        timeout=timeout,
    )
    if proc.returncode != 0 and not output_path.exists():
        raise CaptureError(
            f"Hydra capture failed for {output_path.name}: {proc.stderr.strip() or proc.stdout.strip()}"
        )
    if not output_path.exists():
        raise CaptureError(f"Hydra did not produce screenshot: {output_path}")


def bootstrap_database(hydra_bin: Path, app_data: Path, scratch_dir: Path) -> Path:
    bootstrap_capture = scratch_dir / "bootstrap.png"
    run_hydra_capture(hydra_bin, app_data, bootstrap_capture, width=1500, height=920)
    return hydra_database_path(app_data)


def maybe_write_contact_sheet(output_dir: Path, images: list[Path]) -> Path | None:
    montage = shutil.which("montage")
    if montage is None:
        return None

    contact_sheet = output_dir / "contact-sheet.png"
    columns = 4
    rows = max(1, math.ceil(len(images) / columns))
    subprocess.run(
        [
            montage,
            *[str(image) for image in images],
            "-tile",
            f"{columns}x{rows}",
            "-geometry",
            "+24+24",
            "-background",
            "#06080a",
            str(contact_sheet),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    return contact_sheet


def parse_args() -> argparse.Namespace:
    root = repo_root()
    parser = argparse.ArgumentParser(
        description="Capture deterministic Hydra UI screenshots for visual review."
    )
    parser.add_argument(
        "--hydra-bin",
        type=Path,
        default=root / "build" / "debug" / "hydra",
        help="Path to the hydra desktop binary.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=root / ".runtime" / "ui-captures" / "current",
        help="Directory to write PNG captures and manifest data.",
    )
    parser.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep the temporary repo and XDG_DATA_HOME fixture for inspection.",
    )
    parser.add_argument(
        "--resized-rail-width",
        type=int,
        default=408,
        help="Optional sidebar width in pixels for the resized-rail capture set.",
    )
    parser.add_argument(
        "--alternate-theme",
        default="hermes_veil",
        help="Named alternate theme id to capture in addition to the default.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if not args.hydra_bin.exists():
        raise CaptureError(f"Hydra binary not found: {args.hydra_bin}")

    fixture_root = Path(tempfile.mkdtemp(prefix="hydra-ui-capture-"))
    repo_path = fixture_root / "repo"
    worktree_path = fixture_root / f"{repo_path.name}-{BRANCH_NAME}"
    app_data = fixture_root / "data"
    db_path = hydra_database_path(app_data)
    session_name = f"hydra-ui-capture-{uuid.uuid4().hex[:10]}"

    output_dir = args.output_dir
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    manifest = {
        "fixture_root": str(fixture_root),
        "repo": str(repo_path),
        "worktree": str(worktree_path),
        "db_path": str(db_path),
        "captures": {},
    }

    try:
        ensure_git_repo(repo_path)
        precreate_worktree(repo_path, worktree_path)
        bootstrap_database(args.hydra_bin, app_data, output_dir)

        repo_id = inject_repository_row(
            db_path,
            repo_path,
            repo_name=REPO_NAME,
            updated_at=iso_now(offset_seconds=30),
        )

        baseline_wide = output_dir / "phase2-baseline-wide.png"
        baseline_tight = output_dir / "phase2-baseline-tight.png"
        baseline_narrow = output_dir / "phase2-baseline-narrow.png"
        baseline_compact = output_dir / "phase2-baseline-compact.png"
        baseline_collapsed_wide = output_dir / "phase2-baseline-collapsed-wide.png"
        baseline_resized_wide = output_dir / "phase2-baseline-resized-wide.png"
        alt_baseline_wide = output_dir / "phase2-alt-baseline-wide.png"
        run_hydra_capture(args.hydra_bin, app_data, baseline_wide, width=1500, height=920)
        run_hydra_capture(args.hydra_bin, app_data, baseline_tight, width=1180, height=760)
        run_hydra_capture(args.hydra_bin, app_data, baseline_narrow, width=1040, height=720)
        run_hydra_capture(args.hydra_bin, app_data, baseline_compact, width=960, height=700)
        run_hydra_capture(
            args.hydra_bin,
            app_data,
            baseline_collapsed_wide,
            width=1500,
            height=920,
            start_sidebar_collapsed=True,
        )
        run_hydra_capture(
            args.hydra_bin,
            app_data,
            baseline_resized_wide,
            width=1500,
            height=920,
            start_sidebar_width=args.resized_rail_width,
        )
        run_hydra_capture(
            args.hydra_bin,
            app_data,
            alt_baseline_wide,
            width=1500,
            height=920,
            start_theme=args.alternate_theme,
        )

        pane_id = create_tmux_session(session_name, worktree_path)
        insert_session_row(
            db_path,
            repo_id=repo_id,
            name=f"{REPO_NAME} [Codex]",
            provider_key="codex",
            state="idle",
            tmux_session_name=session_name,
            tmux_pane_id=pane_id,
            working_directory=worktree_path,
            updated_at=iso_now(offset_seconds=60),
        )

        live_wide = output_dir / "phase2-live-wide.png"
        live_tight = output_dir / "phase2-live-tight.png"
        live_narrow = output_dir / "phase2-live-narrow.png"
        live_compact = output_dir / "phase2-live-compact.png"
        live_collapsed_wide = output_dir / "phase2-live-collapsed-wide.png"
        live_resized_wide = output_dir / "phase2-live-resized-wide.png"
        alt_live_wide = output_dir / "phase2-alt-live-wide.png"
        run_hydra_capture(args.hydra_bin, app_data, live_wide, width=1500, height=920)
        run_hydra_capture(args.hydra_bin, app_data, live_tight, width=1180, height=760)
        run_hydra_capture(args.hydra_bin, app_data, live_narrow, width=1040, height=720)
        run_hydra_capture(args.hydra_bin, app_data, live_compact, width=960, height=700)
        run_hydra_capture(
            args.hydra_bin,
            app_data,
            live_collapsed_wide,
            width=1500,
            height=920,
            start_sidebar_collapsed=True,
        )
        run_hydra_capture(
            args.hydra_bin,
            app_data,
            live_resized_wide,
            width=1500,
            height=920,
            start_sidebar_width=args.resized_rail_width,
        )
        run_hydra_capture(
            args.hydra_bin,
            app_data,
            alt_live_wide,
            width=1500,
            height=920,
            start_theme=args.alternate_theme,
        )

        captures = [
            baseline_wide,
            baseline_resized_wide,
            baseline_collapsed_wide,
            live_wide,
            live_resized_wide,
            live_collapsed_wide,
            baseline_tight,
            live_tight,
            baseline_narrow,
            live_narrow,
            baseline_compact,
            live_compact,
            alt_baseline_wide,
            alt_live_wide,
        ]
        contact_sheet = maybe_write_contact_sheet(output_dir, captures)

        manifest["captures"] = {
            "baseline_wide": str(baseline_wide),
            "baseline_tight": str(baseline_tight),
            "baseline_narrow": str(baseline_narrow),
            "baseline_compact": str(baseline_compact),
            "baseline_collapsed_wide": str(baseline_collapsed_wide),
            "baseline_resized_wide": str(baseline_resized_wide),
            "live_wide": str(live_wide),
            "live_tight": str(live_tight),
            "live_narrow": str(live_narrow),
            "live_compact": str(live_compact),
            "live_collapsed_wide": str(live_collapsed_wide),
            "live_resized_wide": str(live_resized_wide),
            "alt_baseline_wide": str(alt_baseline_wide),
            "alt_live_wide": str(alt_live_wide),
        }
        if contact_sheet is not None:
            manifest["contact_sheet"] = str(contact_sheet)
        (output_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
        return 0
    finally:
        kill_tmux_session(session_name)
        if not args.keep_temp:
            shutil.rmtree(fixture_root, ignore_errors=True)


if __name__ == "__main__":
    raise SystemExit(main())
