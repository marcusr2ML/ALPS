/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
*
*****************************************************************************/

// linear_timeseries_fit fits y_k = a + b x_k with x_k = 1, 2, ..., n.
// On exact data the least-squares fit returns (a, b).

#include <alps/numeric/regression.hpp>

#include <gtest/gtest.h>
#include <cmath>
#include <vector>

TEST(Regression, LinearFitRecoversInterceptAndSlope)
{
    // y = 3 + 0.5 x for x = 1..5
    const std::vector<double> y = {3.5, 4.0, 4.5, 5.0, 5.5};
    const std::pair<double, double> fit = alps::numeric::linear_timeseries_fit(y.begin(), y.end());
    EXPECT_NEAR(fit.first, 3.0, 1e-12) << "abs err " << std::abs(fit.first - 3.0);
    EXPECT_NEAR(fit.second, 0.5, 1e-12) << "abs err " << std::abs(fit.second - 0.5);
}

TEST(Regression, LinearFitOfConstant)
{
    const std::vector<double> y(4, 1.0);
    const std::pair<double, double> fit = alps::numeric::linear_timeseries_fit(y.begin(), y.end());
    EXPECT_NEAR(fit.first, 1.0, 1e-12) << "abs err " << std::abs(fit.first - 1.0);
    EXPECT_NEAR(fit.second, 0.0, 1e-12) << "abs err " << std::abs(fit.second);
}

TEST(Regression, ExponentialFitRecoversAmplitudeAndRate)
{
    // y = 2 exp(-0.25 x) for x = 1..10
    std::vector<double> y;
    for (int x = 1; x <= 10; ++x)
        y.push_back(2.0 * std::exp(-0.25 * x));
    const std::pair<double, double> fit = alps::numeric::exponential_timeseries_fit(y.begin(), y.end());
    EXPECT_NEAR(fit.first, 2.0, 1e-12) << "rel err " << std::abs(fit.first - 2.0) / 2.0;
    EXPECT_NEAR(fit.second, -0.25, 1e-12) << "abs err " << std::abs(fit.second + 0.25);
}
