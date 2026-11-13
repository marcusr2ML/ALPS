# Agent guidelines for ALPS

Follow [CONTRIBUTING.md](CONTRIBUTING.md) for contribution requirements,
including its provenance and scientific credit policy.

When copying, translating, or substantially adapting external code, record
the source and nature of the adaptation near the affected code. Preserve
copyright and license notices, and flag uncertain provenance or licensing
for maintainer review. Do not invent attribution or claim independence
without evidence.

For scientific credit changes, update `CITATION.cff` and `CITATIONS.yaml`
using [the citation maintenance instructions](.github/scripts/citations/README.md).
`CITATION.md` is generated; do not edit it directly.

## Read order

1. This file.
2. [`REPOSITORY_RULES.md`](REPOSITORY_RULES.md): tiers, numerical rules,
   reference data, claiming, `hold:`.
3. Only the area docs the task needs (`src/alps/README.md`, `tests/README.md`,
   the relevant tutorial).

The implementation, its tests and the CI configuration define current behavior.
If a document disagrees with them, the document is the defect.

## Build and test

```bash
cmake --preset <preset>            # see CMakePresets.json
cmake --build --preset <preset>
ctest --test-dir <build-dir> --output-on-failure
python -m pytest tests -q                    # fast suite
```

Keep local checks proportional to the change. Hosted CI owns the full matrix
(Linux, macOS, MPI, debug, sanitizers, packaging).

## Before opening a PR

- Claim the issue: `python .github/scripts/agent/claim_issue.py claim <N>`.
- Re-check right before opening: `python .github/scripts/agent/claim_issue.py precheck <N>`.
- Fill the **Tier** and **Verification** sections of the PR template.
- Bug fixes: include a test that fails on `master` and passes on the branch,
  and paste both results in Verification.
- Worker agents never merge, never push to `master`, never edit `tests/physics/references/`.
