"""
File: scripts/check_repo.py
Version: 0.1.0
Date: 2026-09-21
Purpose: Repository quality gate for file size, version consistency and source headers.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
MAX_LINES = 800
SOURCE_SUFFIXES = {".cpp", ".h", ".py"}
SKIP_PARTS = {".git", ".pio", ".venv"}


def iter_files():
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if any(part in SKIP_PARTS for part in path.parts):
            continue
        yield path


def check_line_limits(errors: list[str]) -> None:
    for path in iter_files():
        try:
            count = len(path.read_text(encoding="utf-8").splitlines())
        except UnicodeDecodeError:
            continue
        if count > MAX_LINES:
            errors.append(f"{path.relative_to(ROOT)} has {count} lines (> {MAX_LINES})")


def check_source_headers(errors: list[str]) -> None:
    exempt = {ROOT / "VERSION"}
    for path in iter_files():
        if path in exempt or path.suffix not in SOURCE_SUFFIXES:
            continue
        text = path.read_text(encoding="utf-8")
        head = "\n".join(text.splitlines()[:12])
        if "Version:" not in head or "Date:" not in head or "Purpose:" not in head:
            errors.append(f"{path.relative_to(ROOT)} is missing Version/Date/Purpose header")


def check_version(errors: list[str]) -> None:
    version = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        errors.append(f"VERSION is not SemVer: {version!r}")
        return

    platformio = (ROOT / "platformio.ini").read_text(encoding="utf-8")
    if f'GA_FIRMWARE_VERSION=\\\"{version}\\\"' not in platformio:
        errors.append("platformio.ini firmware version differs from VERSION")

    changelog = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8")
    if f"## [{version}]" not in changelog:
        errors.append("CHANGELOG.md has no entry for current VERSION")


def main() -> int:
    errors: list[str] = []
    check_line_limits(errors)
    check_source_headers(errors)
    check_version(errors)

    if errors:
        print("Quality gate failed:")
        for error in errors:
            print(f" - {error}")
        return 1

    print("Quality gate passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
