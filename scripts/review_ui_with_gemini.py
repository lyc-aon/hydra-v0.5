#!/usr/bin/env python3

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


class GeminiReviewError(RuntimeError):
    pass


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def relative_workspace_path(path: Path, root: Path) -> str:
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(root))
    except ValueError as exc:
        raise GeminiReviewError(f"Image must be inside the workspace for headless Gemini review: {path}") from exc


def build_prompt(image_paths: list[str]) -> str:
    image_refs = " ".join(f"@{path}" for path in image_paths)
    return (
        "You are reviewing Hydra V2 desktop application screenshots.\n\n"
        "Target aesthetic:\n"
        "- modern, highly responsive Qt desktop workbench\n"
        "- nu-oldtech, not sterile SaaS minimalism\n"
        "- matrix/phosphor undertones without becoming neon parody\n"
        "- old technical control-room and dialog-screen discipline, with Neon Genesis Evangelion influence\n"
        "- mythic Hydra undertones, but subtle and grown-up\n\n"
        "Constraints:\n"
        "- This is still a Phase 1/2 shell, not the final provider/terminal product.\n"
        "- Do not invent fake features.\n"
        "- Focus on shell composition, hierarchy, density, tokens, motion, responsiveness, and component styling.\n\n"
        "Deliverables:\n"
        "1. Top 10 problems in the current UI, ranked by impact.\n"
        "2. What already works and should be preserved.\n"
        "3. Where the UI misses the target aesthetic.\n"
        "4. A concrete Shell V2 direction: layout, color, typography, spacing, interaction, motion, responsive rules.\n"
        "5. Specific rewrite directives suitable for QML implementation next.\n\n"
        f"Screenshots: {image_refs}"
    )


def parse_args() -> argparse.Namespace:
    root = repo_root()
    current = root / ".runtime" / "ui-captures" / "current"
    parser = argparse.ArgumentParser(description="Run a Gemini review over captured Hydra UI screenshots.")
    parser.add_argument(
        "images",
        nargs="*",
        type=Path,
        default=[
            current / "phase2-baseline-wide.png",
            current / "phase2-baseline-resized-wide.png",
            current / "phase2-baseline-collapsed-wide.png",
            current / "phase2-live-wide.png",
            current / "phase2-live-resized-wide.png",
            current / "phase2-live-collapsed-wide.png",
            current / "phase2-live-tight.png",
            current / "phase2-live-compact.png",
        ],
        help="Workspace-local screenshot paths to include in the review.",
    )
    parser.add_argument(
        "--model",
        default="gemini-2.5-pro",
        help="Gemini model to use.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=root / "docs" / "validation" / "gemini-ui-review-2026-03-06.md",
        help="Markdown file to write with the review output.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = repo_root()

    images = [path.resolve() for path in args.images]
    missing = [str(path) for path in images if not path.exists()]
    if missing:
        raise GeminiReviewError(f"Missing screenshot(s): {', '.join(missing)}")

    workspace_paths = [relative_workspace_path(path, root) for path in images]
    prompt = build_prompt(workspace_paths)
    result = subprocess.run(
        [
            "gemini",
            "-p",
            prompt,
            "-m",
            args.model,
            "--approval-mode",
            "default",
            "--output-format",
            "text",
            "--sandbox",
            "false",
        ],
        cwd=root,
        text=True,
        capture_output=True,
        timeout=180,
    )
    if result.returncode != 0:
        raise GeminiReviewError(result.stderr.strip() or result.stdout.strip() or "Gemini review failed")

    output = result.stdout.strip()
    if not output:
        raise GeminiReviewError("Gemini returned empty review output")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    body = (
        "# Gemini UI Review\n\n"
        f"Model: `{args.model}`\n\n"
        "Screenshots:\n"
        + "\n".join(f"- `{path}`" for path in workspace_paths)
        + "\n\n"
        + output
        + "\n"
    )
    args.output.write_text(body, encoding="utf-8")
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
