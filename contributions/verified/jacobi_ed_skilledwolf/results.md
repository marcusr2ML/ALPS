# jacobi_ed by @skilledwolf

Accepted from PR #8 (commit 3b7ac209b9de) on 2026-10-09.
Threshold: 50% of judged cases. Times are from the filing run.

**Overall: 6 of 6 judged cases passed (100%), 134.3 us total.**

### By geometry

| geometry | passed | rate | N/A | time (us) |
|---|---|---|---|---|
| 1d | 3/3 | 100% | 3 | 25.0 |
| 2d | 3/3 | 100% | 1 | 109.3 |
| **overall** | **6/6** | **100%** | 4 | 134.3 |

### By quantity

| quantity | passed | rate | N/A | time (us) |
|---|---|---|---|---|
| correlation | 0/0 | - | 4 | 0.0 |
| energy | 6/6 | 100% | 0 | 134.3 |
| **overall** | **6/6** | **100%** | 4 | 134.3 |

### By case

| result | case | quantity | geometry | required | value | reference | time (us) |
|---|---|---|---|---|---|---|---|
| PASS | open chain 10 | energy | 1d | yes | E0 = -1.9189859472 +/- 0.0e+00 | -1.9189859472 | 13.2 |
| PASS | ring 10 N=5 | energy | 1d | yes | E0 = -6.4721359550 +/- 0.0e+00 | -6.4721359550 | 11.5 |
| PASS | biased dimer | energy | 1d | yes | E0 = -1.4142135624 +/- 0.0e+00 | -1.4142135624 | 0.3 |
| PASS | open 3x3 | energy | 2d | yes | E0 = -2.8284271247 +/- 0.0e+00 | -2.8284271247 | 7.7 |
| PASS | torus 4x4 N=8 | energy | 2d | yes | E0 = -12.0000000000 +/- 0.0e+00 | -12.0000000000 | 54.8 |
| PASS | open 4x4 | energy | 2d | no | E0 = -3.2360679775 +/- 0.0e+00 | -3.2360679775 | 46.8 |
| N/A  | open chain 10 | correlation | 1d | yes | quantity not declared |  |  |
| N/A  | ring 10 N=5 | correlation | 1d | yes | quantity not declared |  |  |
| N/A  | biased dimer | correlation | 1d | yes | quantity not declared |  |  |
| N/A  | open 3x3 | correlation | 2d | yes | quantity not declared |  |  |
