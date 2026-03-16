#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
import uuid
from pathlib import Path


def resolve_message(args: argparse.Namespace, parser: argparse.ArgumentParser) -> str:
    inline_message = getattr(args, "message", None)
    use_stdin = bool(getattr(args, "stdin", False))
    message_file = getattr(args, "message_file", None)

    source_count = int(bool(inline_message)) + int(use_stdin) + int(bool(message_file))
    if source_count != 1:
        parser.error("provide exactly one of: inline message, --stdin, or --message-file")

    if inline_message:
        message = str(inline_message)
    elif use_stdin:
        message = sys.stdin.read()
    else:
        try:
            message = Path(message_file).expanduser().read_text(encoding="utf-8")
        except OSError as exc:
            raise SystemExit(f"hydra-router: could not read message file: {message_file}: {exc}") from exc

    normalized = message.strip()
    if not normalized:
        raise SystemExit("hydra-router: message is empty")
    return normalized


def load_manifest(path: Path) -> dict:
    try:
        return json.loads(path.read_text())
    except FileNotFoundError as exc:
        raise SystemExit(f"hydra-router: manifest not found: {path}") from exc
    except json.JSONDecodeError as exc:
        raise SystemExit(f"hydra-router: manifest is not valid JSON: {path}") from exc


def normalize_alias(value: str) -> str:
    return value.strip().casefold()


def find_session(manifest: dict, alias: str) -> dict:
    target = normalize_alias(alias)
    for session in manifest.get("sessions", []):
        if normalize_alias(session.get("alias", "")) == target:
            return session
    raise SystemExit(f"hydra-router: unknown worker alias: {alias}")


def maybe_find_session(manifest: dict, alias: str) -> dict | None:
    target = normalize_alias(alias)
    for session in manifest.get("sessions", []):
        if normalize_alias(session.get("alias", "")) == target:
            return session
    return None


def run_tmux(*args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    command = ["tmux"]
    socket_path = os.environ.get("HYDRA_TMUX_SOCKET_PATH", "").strip()
    socket_name = os.environ.get("HYDRA_TMUX_SOCKET_NAME", "").strip()
    if socket_path:
        command.extend(["-S", socket_path])
    elif socket_name:
        command.extend(["-L", socket_name])
    command.extend(args)
    return subprocess.run(
        command,
        text=True,
        capture_output=True,
        check=check,
    )


def paste_and_submit(pane_id: str, text: str, provider_key: str = "") -> None:
    buffer_name = f"hydra-router-{uuid.uuid4().hex}"
    try:
        run_tmux("set-buffer", "-b", buffer_name, text)
        run_tmux("paste-buffer", "-d", "-b", buffer_name, "-t", pane_id)
        normalized_provider = provider_key.strip().casefold()
        if normalized_provider == "codex" and ("\n" in text or len(text) >= 400):
            time.sleep(1.0)
            run_tmux("send-keys", "-t", pane_id, "Enter")
            time.sleep(0.6)
            run_tmux("send-keys", "-t", pane_id, "Enter")
        else:
            time.sleep(0.15)
            run_tmux("send-keys", "-t", pane_id, "Enter")
    except subprocess.CalledProcessError as exc:
        stderr = exc.stderr.strip()
        raise SystemExit(f"hydra-router: tmux command failed: {stderr or exc}") from exc
    finally:
        run_tmux("delete-buffer", "-b", buffer_name, check=False)


def capture_pane(pane_id: str, lines: int) -> str:
    try:
        result = run_tmux("capture-pane", "-p", "-t", pane_id, "-S", f"-{max(lines, 1)}")
    except subprocess.CalledProcessError as exc:
        stderr = exc.stderr.strip()
        raise SystemExit(f"hydra-router: capture failed: {stderr or exc}") from exc

    return result.stdout.rstrip()


def reports_dir(base_dir: Path) -> Path:
    path = base_dir / "reports"
    path.mkdir(parents=True, exist_ok=True)
    return path


def clip_text(value: str, max_lines: int = 80, max_chars: int = 6000) -> str:
    lines = value.splitlines()
    if len(lines) > max_lines:
        lines = lines[-max_lines:]
    clipped = "\n".join(lines).strip()
    if len(clipped) > max_chars:
        clipped = clipped[-max_chars:]
    return clipped


def resolved_alias(alias: str | None) -> str:
    candidate = (alias or "").strip()
    if candidate:
        return candidate

    candidate = os.environ.get("HYDRA_SESSION_ALIAS", "").strip()
    if candidate:
        return candidate

    raise SystemExit("hydra-router: worker alias not provided and HYDRA_SESSION_ALIAS is not set")


def derive_summary_from_report(report_text: str, report_path: Path) -> str:
    for raw_line in report_text.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        lower = line.casefold()
        if lower.startswith("summary:"):
            summary = line.split(":", 1)[1].strip()
            if summary:
                return summary[:240]
        if line.startswith("#"):
            continue
        if line.startswith("```"):
            continue
        return line[:240]

    return f"Worker completed. See report {report_path.name}."


def resolve_summary(args: argparse.Namespace, report_path: Path, report_text: str) -> str:
    summary = str(getattr(args, "summary", "") or "").strip()
    summary_file = getattr(args, "summary_file", None)

    if summary and summary_file:
        raise SystemExit("hydra-router: provide at most one of --summary or --summary-file")

    if summary_file:
        try:
            summary = Path(summary_file).expanduser().read_text(encoding="utf-8").strip()
        except OSError as exc:
            raise SystemExit(f"hydra-router: could not read summary file: {summary_file}: {exc}") from exc

    if summary:
        return summary[:240]

    return derive_summary_from_report(report_text, report_path)


def control_script_path() -> Path:
    return Path(__file__).resolve()


def reports_root_for_manifest(manifest_path: Path) -> Path:
    resolved_manifest = manifest_path.resolve()
    if resolved_manifest.parent.name == "router":
        return resolved_manifest.parent.parent / "workspaces" / "router" / "reports"
    return resolved_manifest.parent / "reports"


def report_delivery_marker(report_path: Path) -> Path:
    return report_path.with_name(report_path.name + ".reported")


def build_master_request_block(message: str) -> tuple[str, str]:
    request_id = uuid.uuid4().hex
    return request_id, (
        "HYDRA MASTER REQUEST\n"
        f"request_id: {request_id}\n"
        "source: master\n"
        "router_action: route_then_review_then_report_master\n"
        "completion_target: master_via_router\n"
        "request:\n"
        f"{message.strip()}\n"
    )


def build_route_request_block(alias: str,
                              message: str,
                              manifest_path: Path,
                              provider_key: str) -> tuple[str, str, Path]:
    route_id = uuid.uuid4().hex
    report_file_name = f"worker-{normalize_alias(alias)}-{route_id}.md"
    report_path = reports_root_for_manifest(manifest_path) / report_file_name
    report_path.parent.mkdir(parents=True, exist_ok=True)
    normalized_provider = provider_key.strip().casefold()
    if normalized_provider in {"codex", "hermes"}:
        compact_message = " ".join(message.strip().split())
        payload = (
            "HYDRA ROUTE REQUEST"
            f" | route_id: {route_id}"
            f" | worker_alias: {alias}"
            f" | when the task is truly finished, write a concise markdown report to {report_path}"
            " | Hydra watches that file automatically, so do not notify the router manually and do not write the report early"
            f" | task: {compact_message}"
        )
        return route_id, payload, report_path

    return (
        route_id,
        (
            "HYDRA ROUTE REQUEST\n"
            f"route_id: {route_id}\n"
            "source: router\n"
            f"worker_alias: {alias}\n"
            "\n"
            "When the task is actually finished, write a concise markdown report to this exact file:\n"
            f"{report_path}\n"
            "\n"
            "Hydra is already watching that file and will return the completed report to the router automatically.\n"
            "Do not search for environment variables.\n"
            "Do not try to notify the router manually.\n"
            "Do not write the report early.\n"
            "If you are still working, keep working.\n"
            "\n"
            "TASK:\n"
            f"{message.strip()}\n"
        ),
        report_path,
    )


def spawn_report_watcher(manifest_path: Path, alias: str, route_id: str, report_path: Path) -> None:
    marker_path = report_delivery_marker(report_path)
    if marker_path.exists():
        return

    with open(os.devnull, "w", encoding="utf-8") as devnull:
        subprocess.Popen(
            [
                sys.executable,
                str(control_script_path()),
                "--manifest",
                str(manifest_path.resolve()),
                "watch-report",
                alias,
                "--route-id",
                route_id,
                "--report-file",
                str(report_path),
            ],
            start_new_session=True,
            stdin=subprocess.DEVNULL,
            stdout=devnull,
            stderr=devnull,
        )


def format_recent_events(session: dict | None) -> str:
    if not session:
        return "- none"

    events = session.get("recent_events", []) or []
    if not events:
        return "- none"

    lines = []
    for event in events:
        state = str(event.get("state", "")).strip()
        summary = str(event.get("summary", "")).strip()
        detail = str(event.get("detail", "")).strip()
        created_at = str(event.get("created_at", "")).strip()
        parts = [part for part in (state, summary) if part]
        head = " :: ".join(parts) if parts else "event"
        if detail:
            head += f" :: {detail}"
        if created_at:
            head += f" [{created_at}]"
        lines.append(f"- {head}")
    return "\n".join(lines)


def print_status(manifest: dict) -> None:
    master_provider = manifest.get("master_provider") or "-"
    router_provider = manifest.get("router_provider") or "-"
    print(f"master: {master_provider} pane={manifest.get('master_pane_id') or '-'}")
    print(f"router: {router_provider} pane={manifest.get('router_pane_id') or '-'}")
    print("")

    sessions = manifest.get("sessions", [])
    if not sessions:
        print("workers: none")
        return

    print("workers:")
    for session in sessions:
        alias = session.get("alias") or "?"
        name = session.get("name") or "Unnamed"
        provider = session.get("provider") or "-"
        state = session.get("state") or "-"
        detail = session.get("status_detail") or ""
        line = f"  {alias}: {name} [{provider}] {state}"
        if detail:
            line += f" :: {detail}"
        print(line)


def capture_output(manifest: dict, alias: str, lines: int) -> None:
    session = find_session(manifest, alias)
    pane_id = session.get("tmux_pane", "").strip()
    if not pane_id:
        raise SystemExit(f"hydra-router: worker {alias} has no tmux pane")
    print(capture_pane(pane_id, lines))


def ask_router(manifest: dict, message: str) -> None:
    pane_id = str(manifest.get("router_pane_id", "")).strip()
    if not pane_id:
        raise SystemExit("hydra-router: router pane is not available")
    request_id, block = build_master_request_block(message)
    paste_and_submit(pane_id, block, str(manifest.get("router_provider", "")))
    print(f"sent to router request_id={request_id}")


def route_worker(manifest_path: Path, manifest: dict, alias: str, message: str) -> None:
    session = find_session(manifest, alias)
    pane_id = session.get("tmux_pane", "").strip()
    if not pane_id:
        raise SystemExit(f"hydra-router: worker {alias} has no tmux pane")

    alias_label = session.get("alias") or alias
    route_id, payload, report_path = build_route_request_block(alias_label,
                                                               message,
                                                               manifest_path,
                                                               str(session.get("provider", "")))
    paste_and_submit(pane_id, payload, str(session.get("provider", "")))
    spawn_report_watcher(manifest_path, alias_label, route_id, report_path)
    print(f"routed @{alias_label} route_id={route_id} completion=async")


def build_worker_completion_block(alias: str,
                                  route_id: str,
                                  report_file: Path,
                                  report_text: str,
                                  summary: str,
                                  session: dict | None) -> str:
    worker_name = session.get("name", alias) if session else alias
    provider = session.get("provider", "-") if session else "-"
    state = session.get("state", "Unknown") if session else "Unknown"
    status_detail = session.get("status_detail", "") if session else ""
    updated_at = session.get("updated_at", "") if session else ""
    excerpt = clip_text(report_text)

    block = [
        f"HYDRA WORKER REPORT @{alias}:",
        "source: worker",
        f"worker_alias: {alias}",
        "router_action: verify_then_report_master",
        "forward_to: master_via_router",
        f"route_id: {route_id or '-'}",
        f"worker_name: {worker_name}",
        f"provider: {provider}",
        f"state: {state}",
        f"summary: {summary}",
        f"report_file: {report_file}",
    ]
    if status_detail:
        block.append(f"status_detail: {status_detail}")
    if updated_at:
        block.append(f"updated_at: {updated_at}")
    block.extend(
        [
            "recent_events:",
            format_recent_events(session),
            "worker_report:",
            excerpt,
            "",
        ]
    )
    return "\n".join(block)


def worker_complete(manifest: dict,
                    alias: str | None,
                    route_id: str,
                    report_file: str,
                    summary: str | None,
                    summary_file: str | None) -> None:
    router_pane_id = str(manifest.get("router_pane_id", "")).strip()
    if not router_pane_id:
        raise SystemExit("hydra-router: router pane is not available")

    alias_value = resolved_alias(alias)
    session = maybe_find_session(manifest, alias_value)
    report_path = Path(report_file).expanduser()
    if not report_path.is_absolute():
        report_path = Path.cwd() / report_path
    try:
        report_text = report_path.read_text(encoding="utf-8").strip()
    except OSError as exc:
        raise SystemExit(f"hydra-router: could not read report file: {report_path}: {exc}") from exc

    report_args = argparse.Namespace(summary=summary, summary_file=summary_file)
    summary_text = resolve_summary(report_args, report_path, report_text)
    paste_and_submit(
        router_pane_id,
        build_worker_completion_block(alias_value, route_id.strip(), report_path, report_text, summary_text, session),
        str(manifest.get("router_provider", "")),
    )
    report_delivery_marker(report_path).write_text("reported\n", encoding="utf-8")
    print(f"reported @{alias_value} to router")


def watch_report(manifest_path: Path, alias: str, route_id: str, report_file: str) -> None:
    report_path = Path(report_file).expanduser()
    if not report_path.is_absolute():
        report_path = Path.cwd() / report_path

    marker_path = report_delivery_marker(report_path)
    if marker_path.exists():
        return

    last_signature: tuple[int, int] | None = None
    stable_observations = 0
    deadline = time.monotonic() + 60.0 * 60.0 * 12.0

    while time.monotonic() < deadline:
        if marker_path.exists():
            return
        if report_path.exists():
            try:
                stat = report_path.stat()
            except OSError:
                time.sleep(1.0)
                continue

            if stat.st_size > 0:
                signature = (stat.st_size, int(stat.st_mtime_ns))
                if signature == last_signature:
                    stable_observations += 1
                else:
                    last_signature = signature
                    stable_observations = 1

                if stable_observations >= 2:
                    manifest = load_manifest(manifest_path)
                    worker_complete(manifest, alias, route_id, str(report_path), None, None)
                    return
        time.sleep(1.0)


def build_report_block(alias: str, summary: str, report_file: Path) -> str:
    report_excerpt = ""
    if report_file.exists():
        report_excerpt = report_file.read_text().strip()
    else:
        report_excerpt = f"(report file missing: {report_file})"

    return (
        f"ROUTER REPORT @{alias}:\n"
        f"router_summary: {summary.strip()}\n"
        f"report_file: {report_file}\n"
        "worker_report:\n"
        f"{report_excerpt}\n"
    )


def report_master(manifest: dict, alias: str, summary: str, report_file: str) -> None:
    pane_id = str(manifest.get("master_pane_id", "")).strip()
    if not pane_id:
        raise SystemExit("hydra-router: master pane is not available")

    report_path = Path(report_file).expanduser()
    if not report_path.is_absolute():
        report_path = Path.cwd() / report_path

    block = build_report_block(alias, summary, report_path)
    paste_and_submit(pane_id, block, str(manifest.get("master_provider", "")))
    print(f"reported @{alias} to master")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="hydra-router")
    parser.add_argument("--manifest", required=True, help="Path to manifest.json")
    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("status")

    capture_parser = subparsers.add_parser("capture")
    capture_parser.add_argument("alias")
    capture_parser.add_argument("--lines", type=int, default=120)

    ask_parser = subparsers.add_parser("ask-router")
    ask_parser.add_argument("message", nargs="?")
    ask_parser.add_argument("--stdin", action="store_true")
    ask_parser.add_argument("--message-file")

    route_parser = subparsers.add_parser("route")
    route_parser.add_argument("alias")
    route_parser.add_argument("message", nargs="?")
    route_parser.add_argument("--stdin", action="store_true")
    route_parser.add_argument("--message-file")

    worker_complete_parser = subparsers.add_parser("worker-complete")
    worker_complete_parser.add_argument("alias", nargs="?")
    worker_complete_parser.add_argument("--route-id", default="")
    worker_complete_parser.add_argument("--report-file", required=True)
    worker_complete_parser.add_argument("--summary")
    worker_complete_parser.add_argument("--summary-file")

    watch_parser = subparsers.add_parser("watch-report")
    watch_parser.add_argument("alias")
    watch_parser.add_argument("--route-id", required=True)
    watch_parser.add_argument("--report-file", required=True)

    report_parser = subparsers.add_parser("report-master")
    report_parser.add_argument("alias")
    report_parser.add_argument("--summary", required=True)
    report_parser.add_argument("--report-file", required=True)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    manifest_path = Path(args.manifest).expanduser()
    manifest = load_manifest(manifest_path)

    if args.command == "status":
        print_status(manifest)
        return 0
    if args.command == "capture":
        capture_output(manifest, args.alias, args.lines)
        return 0
    if args.command == "ask-router":
        ask_router(manifest, resolve_message(args, parser))
        return 0
    if args.command == "route":
        route_worker(manifest_path, manifest, args.alias, resolve_message(args, parser))
        return 0
    if args.command == "worker-complete":
        worker_complete(
            manifest,
            args.alias,
            args.route_id,
            args.report_file,
            args.summary,
            args.summary_file,
        )
        return 0
    if args.command == "watch-report":
        watch_report(manifest_path, args.alias, args.route_id, args.report_file)
        return 0
    if args.command == "report-master":
        report_master(manifest, args.alias, args.summary, args.report_file)
        return 0

    parser.error(f"unsupported command: {args.command}")
    return 2


if __name__ == "__main__":
    sys.exit(main())
