#!/usr/bin/env python3
"""Deterministic PR gate for ALPS (no LLM).

Reads the diff between --base and --head plus the PR body, and reports:
  * tier (from .github/agent/tiers.json, max over changed files)
  * blast radius (CI areas from .github/scripts/ci/areas.json, file/line counts)
  * whether the PR body has Tier and Verification sections
  * for bug fixes: whether tests changed (fail-before/pass-after evidence)
  * tolerance/skip edits in tests (forces T3)
  * a `hold:` line

Writes a Markdown report to stdout. Exit 1 if any BLOCK finding.

Draft. Usage:
  python review_report.py --base <sha> --head <sha> --body-file pr_body.md
"""
from __future__ import annotations

import argparse
import fnmatch
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
TIERS = ROOT / ".github/agent/tiers.json"
AREAS = ROOT / ".github/scripts/ci/areas.json"

# Assertion tolerances and skips only. Plain words like "tolerance" or a
# TOLERANCE simulation parameter are not test-tolerance edits.
TOLERANCE_PAT = re.compile(
    r"^[-+](?![-+]).*("
    r"\b(EXPECT|ASSERT)_(NEAR|DOUBLE_EQ|FLOAT_EQ)\b|\bBOOST_(CHECK|REQUIRE)_CLOSE\w*\b"
    r"|\b(np\.)?(allclose|isclose)\s*\(|\bapprox\s*\(|\bassert_allclose\b"
    r"|\b[ar]tol\s*="
    r"|pytest\.mark\.(skip|xfail)|\bskipif\b|\bGTEST_SKIP\b|\bDISABLED_\w+"
    r")"
)


@dataclass
class Report:
    block: list[str] = field(default_factory=list)
    warn: list[str] = field(default_factory=list)
    info: list[str] = field(default_factory=list)


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True,
                          capture_output=True, text=True).stdout


def changed_files(base: str, head: str) -> list[tuple[str, int, int]]:
    out = git("diff", "--numstat", f"{base}...{head}")
    rows = []
    for line in out.splitlines():
        a, d, path = line.split("\t", 2)
        rows.append((path, int(a) if a != "-" else 0, int(d) if d != "-" else 0))
    return rows


def match(path: str, glob: str) -> bool:
    # fnmatch's '*' already crosses '/', so '**' behaves like '*'.
    return fnmatch.fnmatch(path, glob.replace("**/", "*").replace("**", "*")) or \
        fnmatch.fnmatch(path, glob)


def tier_of(path: str, cfg: dict) -> tuple[int, str]:
    for rule in cfg["rules"]:
        if match(path, rule["glob"]):
            return rule["tier"], rule.get("why", rule["glob"])
    return cfg.get("default_tier", 2), "default"


def areas_of(paths: list[str]) -> list[str]:
    if not AREAS.exists():
        return []
    cfg = json.loads(AREAS.read_text())
    hit = []
    for name, area in cfg.get("areas", {}).items():
        if any(match(p, g) for p in paths for g in area.get("paths", [])):
            hit.append(name)
    return hit


def section(body: str, title: str) -> str | None:
    m = re.search(rf"^##\s+{re.escape(title)}\s*$(.*?)(?=^##\s|\Z)", body, re.M | re.S)
    if not m:
        return None
    text = re.sub(r"<!--.*?-->", "", m.group(1), flags=re.S).strip()
    return text


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", required=True)
    ap.add_argument("--head", required=True)
    ap.add_argument("--body-file", required=True)
    args = ap.parse_args()

    body = Path(args.body_file).read_text() if Path(args.body_file).exists() else ""
    cfg = json.loads(TIERS.read_text())
    files = changed_files(args.base, args.head)
    paths = [p for p, _, _ in files]
    r = Report()

    # --- tier
    per_file = [(p, *tier_of(p, cfg)) for p in paths]
    gate_tier = max((t for _, t, _ in per_file), default=0)

    # tolerance / skip edits force T3
    diff = git("diff", "-U0", f"{args.base}...{args.head}", "--", "tests", "src")
    tol_lines = [l for l in diff.splitlines() if TOLERANCE_PAT.search(l)]
    if tol_lines:
        gate_tier = 3
        r.warn.append(f"{len(tol_lines)} added/removed line(s) touch tolerances or skips → T3")

    declared = re.search(r"Tier:\s*T([0-3])", body)
    declared_t = int(declared.group(1)) if declared else None
    if declared_t is None:
        r.block.append("PR body has no `Tier: T0..T3` line.")
    elif declared_t < gate_tier and not (declared_t == 1 and gate_tier == 2):
        r.block.append(f"Declared T{declared_t} but changed paths require T{gate_tier}.")
    elif declared_t == 1 and gate_tier == 2:
        r.info.append("Declared T1 on T2 paths: needs the physics invariance job to report identical outputs.")

    # --- verification section
    ver = section(body, "Verification")
    if not ver:
        r.block.append("Missing or empty `## Verification` section.")

    # --- bug fix needs a test
    is_bugfix = bool(re.search(r"\[x\]\s*Bug fix", body, re.I)) or ("Closes #" in body and "bug" in body.lower())
    touched_tests = any(p.startswith(("tests/", "python/pyalps/tests/")) or "/tests/" in p for p in paths)
    if is_bugfix and gate_tier >= 2 and not touched_tests:
        r.block.append("Bug fix without a test change (need fail-before/pass-after).")
    if is_bugfix and ver and not re.search(r"fail", ver, re.I):
        r.warn.append("Verification does not show the test failing on master.")

    # --- references
    if any(p.startswith("tests/physics/references/") for p in paths):
        r.block.append("Reference data changed: human (T3) decision required.")

    # --- hold
    hold = re.search(r"^hold:\s*(.+)$", body, re.M | re.I)
    if hold:
        r.block.append(f"hold: {hold.group(1).strip()}")

    # --- blast radius
    added = sum(a for _, a, _ in files)
    deleted = sum(d for _, _, d in files)
    areas = areas_of(paths)

    # --- output
    out = [f"### Agent gate report (deterministic, no LLM)", "",
           f"**Gate tier: T{gate_tier}**" + (f" (declared T{declared_t})" if declared_t is not None else ""),
           f"Blast radius: {len(files)} files, +{added}/−{deleted} lines; CI areas: {', '.join(areas) or 'none'}", ""]
    for label, items in (("BLOCK", r.block), ("WARN", r.warn), ("INFO", r.info)):
        for it in items:
            out.append(f"- **{label}** {it}")
    out += ["", "<details><summary>Per-file tier</summary>", ""]
    out += [f"- T{t} `{p}` ({why})" for p, t, why in sorted(per_file, key=lambda x: -x[1])]
    out += ["", "</details>"]
    print("\n".join(out))
    return 1 if r.block else 0


if __name__ == "__main__":
    sys.exit(main())
