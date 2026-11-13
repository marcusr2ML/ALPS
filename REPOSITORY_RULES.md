# ALPS repository rules (agents and humans)

Each preventable mistake becomes a rule here. Keep rules short, say what
failure they prevent, and delete rules that CI now enforces.

## 1. Tiers

CI assigns the tier from the diff (`.github/agent/tiers.json`) and the measured
effect on outputs. A PR's tier is the highest tier of any file it touches.
The tier written in the PR body is a claim; the gate's tier wins.

- **T0 — cannot change compiled output.** Markdown, comments-only diffs,
  issue templates, CI-helper text.
- **T1 — refactor/cleanup.** Compiled code changes, but the physics suite must
  show identical outputs (fixed seeds) and all existing tests pass unchanged.
  Sanitizer jobs required.
- **T2 — non-numeric bug fix.** Parsers, IO, CLI, error messages, build system.
  Needs a fail-before/pass-after test.
- **T3 — can change numbers.** Anything under the numeric/app paths in
  `tiers.json` whose outputs move, any tolerance change, any edit under
  `tests/physics/references/`. Always a human decision.

## 2. Numerical rules

- Tests assert values, identities or residuals. Shape-only, finite-only or
  "it ran" checks are not correctness tests.
- Never relax a tolerance, skip a test, or delete a check without a stated
  reason in the PR body. Doing so makes the PR T3.
- Approximate comparisons report the residual (abs and rel error) on failure.
- MC comparisons use error bars: |a − b| / sqrt(σa² + σb²) below the agreed z
  threshold. Never compare MC results to a fixed absolute tolerance.
- Couplings and parameters must round-trip at full double precision
  (regression: couplings were cut to 6 significant digits).
- DMRG tests and examples use open chains only.
- Call the DMFT solvers CT-INT / Hirsch-Fye, not "Monte Carlo"; "MC" means
  the MC apps (loop, worm, sse, spinmc, …).

## 3. Reference data

- References live in `tests/physics/references/` with provenance
  (commit, date, command, app versions). They are produced only by
  `tests/physics/freeze_references.py`, never typed by hand.
- Changing a reference is its own PR, T3, with the physics reason stated.

## 4. Bug fixes

- Include a test that fails on `master` and passes on the branch; paste both
  outputs in the Verification section.
- After fixing, check the neighboring code paths (other apps, other lattices,
  float/complex variants) once for the same bug; report findings as issues,
  not as extra changes in the same PR.

## 5. Claiming work (matches the board: Bucket → claimed → PR)

- The issue assignee list is the lock. Claim before starting, read it back,
  re-check assignees and open PRs (`Closes #N`) right before opening a PR.
- Never take an issue assigned to someone else.
- New findings during a PR go to new issues, not into the open PR.

## 6. Holds and merging

- A line `hold: <reason>` in a PR body blocks any automated merge even when CI
  is green. Only a human removes it.
- Worker agents can push branches and open PRs. They hold no merge rights,
  release tokens or write access to references.
- Any revert of a gate-passed PR demotes that tier (see README rollout).

## 7. CI time

- The per-PR pipeline has one total time budget. Tests that need more are
  marked `heavy` and run in `physics-nightly.yml`; a nightly failure opens an
  issue labelled `nightly-physics`.
