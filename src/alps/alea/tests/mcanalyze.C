/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* Copyright (C) 2011-2012 by Lukas Gamper <gamperl@gmail.com>,
*                            Matthias Troyer <troyer@itp.phys.ethz.ch>,
*                            Maximilian Poprawe <poprawem@ethz.ch>
*
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
*
*****************************************************************************/

#include <alps/alea.h>
#include <alps/alea/mcanalyze.hpp>
#include <alps/utilities/size.hpp>
#include <alps/hdf5.hpp>
#include <alps/testing/temporary_directory.hpp>
#include <boost/random.hpp>
#include <gtest/gtest.h>
#include <cmath>
#include <numeric>
#include <vector>

TEST(McAnalyze, LoadsRecordedScalarStatistics)
{
    alps::testing::TemporaryDirectory temporary;
    const auto filename = (temporary.path() / "test.h5").string();
    boost::minstd_rand0 engine(1u);
    boost::uniform_01<boost::minstd_rand0> random(engine);
    alps::ObservableSet measurement;
    measurement << alps::RealObservable("Scalar") << alps::RealVectorObservable("Vector");
    const double initial = random();
    double sum = 0.;
    std::vector<double> samples;
    for (int i = 0; i < 10000; ++i) {
        const double sample = 0.9 * initial + random();
        sum += sample;
        samples.push_back(sample);
        measurement["Scalar"] << sample;
    }
    const auto& scalar = measurement.get<alps::RealObservable>("Scalar");
    const double expected_mean = scalar.mean();
    const double expected_error = scalar.error();
    {
        alps::hdf5::archive archive(filename, "a");
        archive << alps::make_pvp("/test/result", measurement);
    }
    alps::alea::mcdata<double> restored;
    restored.load(filename, "/test/result/Scalar");
    EXPECT_EQ(restored.count(), 10000u);
    EXPECT_NEAR(restored.mean(), sum / 10000., 1e-12);
    EXPECT_DOUBLE_EQ(restored.mean(), expected_mean);
    EXPECT_DOUBLE_EQ(restored.error(), expected_error);
    // mcanalyze treats mcdata as a timeseries of completed bins. The partial
    // final bin contributes to restored.mean(), but not to this free function.
    EXPECT_EQ(restored.bin_size(), 128u);
    EXPECT_EQ(restored.bin_number(), 78u);
    const double completed_mean = std::accumulate(samples.begin(), samples.begin() + 78 * 128, 0.) / (78 * 128);
    EXPECT_NEAR(alps::alea::mean(restored), completed_mean, 1e-12);
    EXPECT_EQ(alps::size(restored), 78u);
}

// Exact autocorrelation rho(t) = A exp(b t), stored as rho(1), ..., rho(N).
TEST(McAnalyze, ExponentialAutocorrelationTimeOfExactExponential)
{
    const double A = 0.8;
    const double b = -0.2;
    const int N = 20;
    std::vector<double> rho;
    for (int t = 1; t <= N; ++t)
        rho.push_back(A * std::exp(b * t));
    const alps::alea::mctimeseries<double> autocorrelation(rho);

    const std::pair<double, double> fit =
        alps::alea::exponential_autocorrelation_time_distance(autocorrelation, 3, 10);
    EXPECT_NEAR(fit.first, A, 1e-12) << "rel err " << std::abs(fit.first - A) / A;
    EXPECT_NEAR(fit.second, b, 1e-12) << "abs err " << std::abs(fit.second - b);

    // sum_{t=1}^{N} rho(t) + integral_{N+1/2}^{inf} A exp(b t) dt
    const double expected = std::accumulate(rho.begin(), rho.end(), 0.) - A / b * std::exp(b * (N + 0.5));
    const double tau = alps::alea::integrated_autocorrelation_time(autocorrelation, fit);
    EXPECT_NEAR(tau, expected, 1e-12 * expected) << "rel err " << std::abs(tau - expected) / expected;
}
