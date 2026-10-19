// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#include <alps/input/diag.hpp>
#include <alps/input/dmrg.hpp>
#include <alps/parameter_input.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

alps::Parameters text(std::string const & parameters) {
    std::istringstream in(parameters);
    return alps::read_parameter_text(in);
}

TEST(DmrgInput, Defaults) {
    alps::input::dmrg_parameters const p = alps::input::read_dmrg_parameters(text("NUMSTATES = 10"));
    EXPECT_EQ(p.number_eigenvalues, 1);
    EXPECT_EQ(p.sweeps, 4);
    EXPECT_EQ(p.start_sweep, 0);
    EXPECT_EQ(p.start_dir, 0);
    EXPECT_EQ(p.start_iter, 1);
    EXPECT_EQ(p.verbose, 0);
    EXPECT_EQ(p.warmup_states, 20);
    EXPECT_EQ(p.truncation_error, -1.);
    EXPECT_EQ(p.lanczos_tolerance, -1.);
    EXPECT_EQ(p.maxstates, 10);
    EXPECT_EQ(p.states, std::vector<int>(8, 10));
    EXPECT_TRUE(p.conserved_totals.empty());
    EXPECT_FALSE(p.temp_directory);
    EXPECT_FALSE(alps::input::dmrg_uses_complex(text("NUMSTATES = 10")));
}

TEST(DmrgInput, ReadsEveryParameter) {
    alps::input::dmrg_parameters const p = alps::input::read_dmrg_parameters(text(
        "NUMBER_EIGENVALUES = 2\nSWEEPS = 3\nSTART_SWEEP = 1\nSTART_DIR = 1\nSTART_ITER = 5\n"
        "VERBOSE = 1\nNUM_WARMUP_STATES = 40\nTRUNCATION_ERROR = 1e-7\nLANCZOS_TOLERANCE = 1e-9\n"
        "NUMSTATES = 50\nTEMP_DIRECTORY = \"/tmp/x\"\nCOMPLEX = true"));
    EXPECT_EQ(p.number_eigenvalues, 2);
    EXPECT_EQ(p.sweeps, 3);
    EXPECT_EQ(p.start_sweep, 1);
    EXPECT_EQ(p.start_dir, 1);
    EXPECT_EQ(p.start_iter, 5);
    EXPECT_EQ(p.verbose, 1);
    EXPECT_EQ(p.warmup_states, 40);
    EXPECT_EQ(p.truncation_error, 1e-7);
    EXPECT_EQ(p.lanczos_tolerance, 1e-9);
    EXPECT_EQ(p.states, std::vector<int>(6, 50));
    EXPECT_EQ(p.temp_directory, std::string("/tmp/x"));
}

TEST(DmrgInput, MaxstatesGrowsToTheMaximum) {
    alps::input::dmrg_parameters const p = alps::input::read_dmrg_parameters(text("SWEEPS = 2\nMAXSTATES = 100"));
    EXPECT_EQ(p.maxstates, 100);
    EXPECT_EQ(p.states, (std::vector<int>{25, 50, 75, 100}));
}

TEST(DmrgInput, StatesListsEachHalfSweep) {
    alps::input::dmrg_parameters const p = alps::input::read_dmrg_parameters(text("SWEEPS = 2\nSTATES = \"10,20 30, 40\""));
    EXPECT_EQ(p.maxstates, -1);
    EXPECT_EQ(p.states, (std::vector<int>{10, 20, 30, 40}));
    EXPECT_THROW(alps::input::read_dmrg_parameters(text("SWEEPS = 2\nSTATES = \"10,20,30\"")), std::runtime_error);
}

TEST(DmrgInput, StatesTakePrecedence) {
    alps::input::dmrg_parameters const p = alps::input::read_dmrg_parameters(
        text("SWEEPS = 1\nSTATES = \"5,6\"\nMAXSTATES = 100\nNUMSTATES = 7"));
    EXPECT_EQ(p.states, (std::vector<int>{5, 6}));
}

TEST(DmrgInput, RejectsMissingStates) {
    try {
        alps::input::read_dmrg_parameters(text("SWEEPS = 2"));
        FAIL();
    } catch (std::runtime_error const & error) {
        EXPECT_NE(std::string(error.what()).find("MAXSTATES"), std::string::npos);
    }
}

TEST(DmrgInput, RejectsStartDir) {
    EXPECT_THROW(alps::input::read_dmrg_parameters(text("NUMSTATES = 10\nSTART_DIR = 2")), std::runtime_error);
}

TEST(DmrgInput, EvaluatesConservedTotals) {
    alps::input::dmrg_parameters const p = alps::input::read_dmrg_parameters(text(
        "NUMSTATES = 10\nL = 8\nCONSERVED_QUANTUMNUMBERS = \"N, Sz\"\nN_total = L/2\nSz_total = 0"));
    typedef std::pair<std::string, double> total;
    EXPECT_EQ(p.conserved_totals, (std::vector<total>{total("N", 4.), total("Sz", 0.)}));
    // Quantum numbers without a total are not fixed.
    alps::input::dmrg_parameters const q = alps::input::read_dmrg_parameters(text(
        "NUMSTATES = 10\nCONSERVED_QUANTUMNUMBERS = \"N,Sz\"\nSz_total = 1"));
    EXPECT_EQ(q.conserved_totals, (std::vector<total>{total("Sz", 1.)}));
}

TEST(DiagInput, Defaults) {
    alps::input::diag_parameters const p = alps::input::read_diag_parameters(text("L = 4"));
    EXPECT_EQ(p.conserved_quantumnumbers, "");
    EXPECT_TRUE(p.translation_symmetry);
    EXPECT_FALSE(p.total_momentum);
    EXPECT_FALSE(p.complex);
    EXPECT_FALSE(p.print_eigenvectors);
    EXPECT_FALSE(p.save_eigenvectors);
    EXPECT_TRUE(p.measure_energy);
    EXPECT_FALSE(p.number_eigenvalues);
    EXPECT_FALSE(p.max_iterations);
}

TEST(DiagInput, ReadsEveryParameter) {
    alps::input::diag_parameters const p = alps::input::read_diag_parameters(text(
        "CONSERVED_QUANTUMNUMBERS = \"N,Sz\"\nTRANSLATION_SYMMETRY = false\nTOTAL_MOMENTUM = \"0 0\"\n"
        "COMPLEX = 0\nPRINT_EIGENVECTORS = 1\nSAVE_EIGENVECTORS = true\nMEASURE_ENERGY = false\n"
        "NUMBER_EIGENVALUES = 3\nMAX_ITERATIONS = 200"));
    EXPECT_EQ(p.conserved_quantumnumbers, "N,Sz");
    EXPECT_FALSE(p.translation_symmetry);
    EXPECT_TRUE(p.total_momentum);
    EXPECT_EQ(p.complex, false);
    EXPECT_TRUE(p.print_eigenvectors);
    EXPECT_TRUE(p.save_eigenvectors);
    EXPECT_FALSE(p.measure_energy);
    EXPECT_EQ(p.number_eigenvalues, 3);
    EXPECT_EQ(p.max_iterations, 200);
}

TEST(FulldiagEvaluationInput, MagneticFieldByDefault) {
    alps::input::fulldiag_evaluation_parameters const p =
        alps::input::read_fulldiag_evaluation_parameters(text("T = 0.5\nh = 0.25"));
    EXPECT_FALSE(p.couple_mu);
    EXPECT_EQ(p.field_name, "h");
    EXPECT_TRUE(p.measure_properties);
    EXPECT_TRUE(p.densities);
    EXPECT_FALSE(p.versus_field);
    EXPECT_FALSE(p.temperatures);
    EXPECT_EQ(p.beta, 2.);
    EXPECT_EQ(p.field, 0.25);
    EXPECT_EQ(p.fields.min, 0.25);
    EXPECT_EQ(p.fields.max, 0.25);
    EXPECT_EQ(p.fields.delta, 1.);
}

TEST(FulldiagEvaluationInput, ChemicalPotentialRanges) {
    alps::input::fulldiag_evaluation_parameters const p = alps::input::read_fulldiag_evaluation_parameters(text(
        "couple = MU\nversus = mu\nMEASURE_CHARGE_PROPERTIES = 0\nMEASURE_MAGNETIC_PROPERTIES = 1\n"
        "DENSITIES = 0\nT_MAX = 2\nDELTA_T = 0.1\nMU_MIN = -1\nMU_MAX = 1\nDELTA_MU = 0.5\nmu = 0.3"));
    EXPECT_TRUE(p.couple_mu);
    EXPECT_EQ(p.field_name, "mu");
    EXPECT_FALSE(p.measure_properties);
    EXPECT_FALSE(p.densities);
    EXPECT_TRUE(p.versus_field);
    ASSERT_TRUE(p.temperatures);
    EXPECT_EQ(p.temperatures->min, 0.);
    EXPECT_EQ(p.temperatures->max, 2.);
    EXPECT_EQ(p.temperatures->delta, 0.1);
    EXPECT_FALSE(p.beta);
    EXPECT_EQ(p.field, 0.3);
    EXPECT_EQ(p.fields.min, -1.);
    EXPECT_EQ(p.fields.max, 1.);
    EXPECT_EQ(p.fields.delta, 0.5);
}

TEST(FulldiagEvaluationInput, VersusOtherFieldIsTemperature) {
    EXPECT_FALSE(alps::input::read_fulldiag_evaluation_parameters(text("beta = 4\nversus = mu")).versus_field);
    EXPECT_TRUE(alps::input::read_fulldiag_evaluation_parameters(text("beta = 4\nversus = H")).versus_field);
    EXPECT_EQ(alps::input::read_fulldiag_evaluation_parameters(text("beta = 4")).beta, 4.);
}

TEST(FulldiagOutputPoint, ReadsTemperatureAndField) {
    EXPECT_FALSE(alps::input::read_fulldiag_output_point(text("H = 1")));
    std::optional<alps::input::fulldiag_output_point> const t = alps::input::read_fulldiag_output_point(text("T = 0.5\nh = 2"));
    ASSERT_TRUE(t);
    EXPECT_EQ(t->beta, 2.);
    EXPECT_EQ(t->field, 2.);
    std::optional<alps::input::fulldiag_output_point> const b = alps::input::read_fulldiag_output_point(text("beta = 3\nH = 1\nh = 2"));
    ASSERT_TRUE(b);
    EXPECT_EQ(b->beta, 3.);
    EXPECT_EQ(b->field, 1.);
}

}
