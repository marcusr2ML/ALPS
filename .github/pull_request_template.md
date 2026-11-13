## Summary

<!-- What does this PR do and why? One or two sentences. -->

## Tier

<!-- One line, e.g. "Tier: T0". The agent gate computes the tier from the diff; if yours is lower, the gate blocks. See REPOSITORY_RULES.md §1. -->

Tier: T?

<!-- To block automated merging even when CI is green, add a line:  hold: <reason> -->

## Verification

<!-- Required. What did you run and what did it show?
     Bug fix: the new test's output on master (failing) and on this branch (passing).
     T1 refactor: confirm physics outputs are identical (fixed seeds).
     T3: which physics checks moved, by how much, and why that is correct. -->

## Type of change

<!-- Check all that apply -->

- [ ] Bug fix
- [ ] New feature or algorithm
- [ ] Documentation or tutorial improvement
- [ ] Build system / dependency update
- [ ] Refactor (no behaviour change)
- [ ] Other: <!-- describe -->

## Testing

<!-- How was this tested? Check all that apply and describe any manual steps. -->

- [ ] Existing tests pass (`ctest --output-on-failure`), full suite
- [ ] Only the neighbouring tests were run (list them in Verification)
- [ ] New tests added for changed behaviour
- [ ] Results verified against known reference values or published results
- [ ] Tested on Linux
- [ ] Tested on macOS

## For simulation code changes

<!-- If this PR adds or modifies a simulation method, please complete this section. Skip if not applicable. -->

- [ ] Physical results match expected behaviour (describe briefly below)
- [ ] Parameter files / input format changes are documented
- [ ] Relevant tutorial or documentation updated

<!-- Brief description of physical validation, if applicable: -->

## Checklist

- [ ] CI passes (build + tests on all platforms)
- [ ] No new compiler warnings (`-Wall -Wextra`)
- [ ] Code follows the style of the surrounding code
- [ ] Commit messages are descriptive and in the imperative mood
- [ ] Written or substantially written by an AI agent (name the agent/model): <!-- e.g. Claude Opus 5.5 -->

## Related issues

<!-- Link any related issues: "Closes #...", "Related to #..." -->
