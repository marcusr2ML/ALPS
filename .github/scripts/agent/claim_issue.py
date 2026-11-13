#!/usr/bin/env python3
"""Claim an issue and re-check ownership before opening a PR (draft).

  claim_issue.py claim <N>     assign @me if unassigned, read back, refuse if someone else holds it
  claim_issue.py precheck <N>  before `gh pr create`: still mine? any open PR already closing #N?

Exit codes: 0 ok, 2 held by someone else, 3 claim did not stick / contended,
            4 issue not found, 5 an open PR already implements it.
Adapted from tensor4all-agent-rules rules/common/assignment.md.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys

REPO = os.environ.get("ALPS_REPO", "ALPSim/ALPS")


def gh(*args: str) -> str:
    p = subprocess.run(["gh", *args], capture_output=True, text=True)
    return p.stdout if p.returncode == 0 else p.stdout or p.stderr


def me() -> str:
    return gh("api", "user", "--jq", ".login").strip()


def issue(n: int) -> dict | None:
    try:
        data = json.loads(gh("api", f"repos/{REPO}/issues/{n}"))
    except json.JSONDecodeError:
        return None
    if "number" not in data or "assignees" not in data:
        return None
    return data


def assignees(n: int) -> list[str] | None:
    data = issue(n)
    return None if data is None else [a["login"] for a in data["assignees"]]


def claim(n: int) -> int:
    who = me()
    cur = assignees(n)
    if cur is None:
        print(f"#{n}: not found"); return 4
    if cur and who not in cur:
        print(f"#{n}: held by {', '.join(cur)}"); return 2
    if who not in cur:
        gh("issue", "edit", str(n), "-R", REPO, "--add-assignee", "@me")
    cur = assignees(n) or []
    if who not in cur:
        print(f"#{n}: claim did not stick"); return 3
    if len(cur) > 1:
        print(f"#{n}: contended ({', '.join(cur)}); resolve by earliest current assignment, see rules")
        return 3
    print(f"#{n}: claimed by {who}"); return 0


def precheck(n: int) -> int:
    who = me()
    cur = assignees(n)
    if cur is None:
        print(f"#{n}: not found"); return 4
    if who not in cur:
        print(f"#{n}: no longer assigned to you ({', '.join(cur) or 'nobody'})"); return 2
    prs = json.loads(gh("pr", "list", "-R", REPO, "--state", "open",
                        "--json", "number,title,body,headRefName,author"))
    pat = re.compile(rf"(#{n}\b)|(-{n}$)")
    for pr in prs:
        text = f"{pr['title']}\n{pr.get('body') or ''}"
        if pat.search(text) or pat.search(pr["headRefName"]):
            if pr["author"]["login"] != who:
                print(f"#{n}: open PR #{pr['number']} by {pr['author']['login']} already references it")
                return 5
    print(f"#{n}: ok to open PR"); return 0


if __name__ == "__main__":
    if len(sys.argv) != 3 or sys.argv[1] not in ("claim", "precheck"):
        print(__doc__); sys.exit(64)
    sys.exit({"claim": claim, "precheck": precheck}[sys.argv[1]](int(sys.argv[2])))
