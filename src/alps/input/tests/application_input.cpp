// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#include <alps/input/ctint.hpp>
#include <alps/input/diag.hpp>
#include <alps/input/dmft.hpp>
#include <alps/input/hirschfye.hpp>
#include <alps/input/dmrg.hpp>
#include <alps/parameter_input.hpp>
#include <gtest/gtest.h>
#include <alps/testing/temporary_directory.hpp>
#include <fstream>
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

// Typed params, read from a parameter file as the interaction program does.
alps::params typed(std::string const & parameters) {
    alps::testing::TemporaryDirectory dir;
    std::ofstream(dir.path() / "parm") << parameters;
    return alps::read_params_file((dir.path() / "parm").string());
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

TEST(DmftInput, RunDefaults) {
    alps::input::dmft_run_parameters const p = alps::input::read_dmft_run_parameters(text("SOLVER = \"hybridization\""));
    EXPECT_FALSE(p.cluster_loop);
    EXPECT_FALSE(p.omega_loop);
    EXPECT_EQ(p.solver, "hybridization");
    EXPECT_FALSE(p.sc_write_delta);
    EXPECT_FALSE(p.antiferromagnet);
    EXPECT_FALSE(p.symmetrization);
    EXPECT_FALSE(p.semicircle_hilbert);
    EXPECT_EQ(p.sizes.flavors, 2);
    EXPECT_EQ(p.sizes.sites, 1);
}

TEST(DmftInput, RunNeedsSolverUnlessClusterLoop) {
    EXPECT_THROW(alps::input::read_dmft_run_parameters(text("OMEGA_LOOP = 1")), std::runtime_error);
    EXPECT_TRUE(alps::input::read_dmft_run_parameters(text("CLUSTER_LOOP = 1")).cluster_loop);
}

TEST(DmftInput, LoopReadsWhatEachLoopNeeds) {
    std::string const common = "N = 16\nBETA = 10\nCONVERGED = 0.001\nSYMMETRIZATION = 1\nBASENAME = \"run\"\n";
    alps::input::dmft_loop_parameters const dressed =
        alps::input::read_dmft_loop_parameters(text(common), alps::input::dmft_loop::dressed_imaginary_time);
    EXPECT_EQ(dressed.n_tau, 16);
    EXPECT_EQ(dressed.beta, 10.);
    EXPECT_EQ(dressed.converged, 0.001);
    EXPECT_TRUE(dressed.symmetrization);
    EXPECT_EQ(dressed.max_it, 1000);
    EXPECT_EQ(dressed.h, 0.);
    EXPECT_EQ(dressed.basename, "run");
    EXPECT_EQ(dressed.g0_tau_output, "G0tau_output");
    EXPECT_FALSE(dressed.h_init);
    // MU is needed by the other loops, NMATSUBARA by the Matsubara loop.
    EXPECT_THROW(alps::input::read_dmft_loop_parameters(text(common), alps::input::dmft_loop::imaginary_time), std::runtime_error);
    EXPECT_THROW(alps::input::read_dmft_loop_parameters(text(common + "MU = 0.5"), alps::input::dmft_loop::matsubara), std::runtime_error);
    alps::input::dmft_loop_parameters const omega = alps::input::read_dmft_loop_parameters(
        text(common + "MU = 0.5\nNMATSUBARA = 64\nSITES = 1\nRELAX_RATE = 0.5\nH = 0.1\nH_INIT = 0.2\nG0TAU_input = \"g0\""),
        alps::input::dmft_loop::matsubara);
    EXPECT_EQ(omega.n_matsubara, 64);
    EXPECT_EQ(omega.mu, 0.5);
    EXPECT_EQ(omega.relax_rate, 0.5);
    EXPECT_EQ(omega.h, 0.1);
    EXPECT_EQ(omega.h_init, std::string("0.2"));
    EXPECT_EQ(omega.initial_g0_tau_output, std::string("g0"));
    EXPECT_EQ(omega.g0_omega_output, "G0omega_output");
}

TEST(DmftInput, InitialG0IgnoresEmptyInputFile) {
    alps::input::dmft_initial_g0_parameters const p =
        alps::input::read_dmft_initial_g0_tau(text("N = 8\nG0TAU_INPUT = \"\"\nG0TAU_input = \"out\""));
    EXPECT_EQ(p.n_tau, 8);
    EXPECT_FALSE(p.input);
    EXPECT_EQ(p.output, std::string("out"));
    alps::input::dmft_initial_g0_parameters const q = alps::input::read_dmft_initial_g0_omega(
        text("N = 8\nNMATSUBARA = 32\nBETA = 5\nMU = 1\nINSULATING = 1\nG0OMEGA_INPUT = \"g0\""));
    EXPECT_EQ(q.n_matsubara, 32);
    EXPECT_TRUE(q.insulating);
    EXPECT_EQ(q.input, std::string("g0"));
}

TEST(DmftInput, Bandstructures) {
    EXPECT_EQ(alps::input::read_dmft_bandstructure(text("t = 1")).kind, alps::input::dmft_bandstructure::semicircle);
    EXPECT_EQ(alps::input::read_dmft_bandstructure(text("TWODBS = square")).kind, alps::input::dmft_bandstructure::square);
    EXPECT_EQ(alps::input::read_dmft_bandstructure(text("TWODBS = hexagonal")).kind, alps::input::dmft_bandstructure::hexagonal);
    alps::input::dmft_bandstructure_choice const dos = alps::input::read_dmft_bandstructure(text("DOSFILE = \"dos.dat\"\nTWODBS = square"));
    EXPECT_EQ(dos.kind, alps::input::dmft_bandstructure::dos);
    EXPECT_EQ(dos.dosfile, "dos.dat");
    EXPECT_EQ(alps::input::read_semicircle_hoppings(text("FLAVORS = 4\nt = 1\nt1 = 0.5")), (std::vector<double>{1., 0.5}));
    EXPECT_THROW(alps::input::read_semicircle_hoppings(text("FLAVORS = 3\nt = 1")), std::logic_error);
    alps::input::dmft_lattice_hoppings const h = alps::input::read_lattice_hoppings(text("tprime = 0.2"));
    EXPECT_EQ(h.t, 1.);
    EXPECT_EQ(h.tprime, 0.2);
    EXPECT_EQ(h.L, 128);
}

TEST(DmftInput, FourierDefaultsTheMoments) {
    alps::input::dmft_fourier_parameters const p =
        alps::input::read_dmft_fourier_parameters(text("BETA = 2\nMU = 0.5\nU = 3\nEPS_1 = 0.25"), true, true);
    EXPECT_EQ(p.beta, 2.);
    EXPECT_EQ(p.mu, 0.5);
    EXPECT_EQ(p.U, 3.);
    EXPECT_EQ(p.eps, (std::vector<double>{0., 0.25}));
    EXPECT_EQ(p.epssq, (std::vector<double>{1., 1.}));
    alps::input::dmft_band_moments const m = alps::input::read_band_moments(text("EPSSQ_0 = 2"), 2);
    EXPECT_FALSE(m.eps[0]);
    EXPECT_EQ(m.epssq[0], 2.);
    EXPECT_FALSE(m.epssq[1]);
}

TEST(DmftInput, InteractionMatrix) {
    alps::input::dmft_interaction_parameters const two = alps::input::read_dmft_interaction(text("U = 4"));
    EXPECT_EQ(two.U, 4.);
    EXPECT_FALSE(two.u_matrix);
    alps::input::dmft_interaction_parameters const four = alps::input::read_dmft_interaction(text("FLAVORS = 4\nU = 4\nJ = 0.5"));
    EXPECT_EQ(four.Uprime, 3.);
    EXPECT_EQ(alps::input::read_dmft_interaction(text("FLAVORS = 4\nU = 4\nJ = 0.5\nU' = 2")).Uprime, 2.);
    EXPECT_THROW(alps::input::read_dmft_interaction(text("FLAVORS = 4\nU = 4")), std::runtime_error);
    EXPECT_EQ(alps::input::read_dmft_interaction(text("U_MATRIX = \"u.dat\"")).u_matrix, std::string("u.dat"));
}

TEST(DmftInput, ExternalSolverReadsHybridizationOnlyWhenWritten) {
    alps::input::dmft_external_solver_parameters const plain =
        alps::input::read_dmft_external_solver(text("N = 8\nTMPNAME = \"tmp\""), alps::input::dmft_loop::imaginary_time);
    EXPECT_EQ(plain.n_tau, 8);
    EXPECT_EQ(plain.tmpname, std::string("tmp"));
    EXPECT_FALSE(plain.sc_write_delta);
    alps::input::dmft_external_solver_parameters const delta = alps::input::read_dmft_external_solver(
        text("N = 8\nNMATSUBARA = 16\nSC_WRITE_DELTA = 1\nBETA = 4\nMU = 1\nH = 0.1"), alps::input::dmft_loop::matsubara);
    EXPECT_EQ(delta.n_matsubara, 16);
    EXPECT_EQ(delta.beta, 4.);
    EXPECT_EQ(delta.mu, 1.);
    EXPECT_EQ(delta.h, 0.1);
    EXPECT_EQ(alps::input::read_dmft_max_time(text("MAX_TIME = 30"), 60), 30u);
    EXPECT_EQ(alps::input::read_dmft_max_time(text("N = 1"), 60), 60u);
}

TEST(HirschFyeInput, ReadsSolverParameters) {
    std::string const required = "THERMALIZATION = 10\nSWEEPS = 100\nBETA = 8\nN = 16\nU = 2\nTOLERANCE = 1e-6\nNMATSUBARA = 32\n";
    alps::input::hirschfye_parameters const p = alps::input::read_hirschfye_parameters(text(required));
    EXPECT_EQ(p.thermalization, 10);
    EXPECT_EQ(p.sweeps, 100);
    EXPECT_EQ(p.beta, 8.);
    EXPECT_EQ(p.n_tau, 16);
    EXPECT_EQ(p.sites, 1);
    EXPECT_EQ(p.U, 2.);
    EXPECT_EQ(p.fourpoint_interval, 1000);
    EXPECT_FALSE(p.measure_fourpoint);
    EXPECT_EQ(p.tolerance, 1e-6);
    EXPECT_EQ(p.max_time, 86400);
    EXPECT_EQ(p.n_matsubara, 32);
    EXPECT_THROW(alps::input::read_hirschfye_parameters(text("BETA = 8")), std::runtime_error);
}

TEST(HirschFyeInput, ResultNeedsSites) {
    EXPECT_THROW(alps::input::read_hirschfye_result_parameters(text("N = 16\nNMATSUBARA = 32")), std::runtime_error);
    alps::input::hirschfye_result_parameters const p = alps::input::read_hirschfye_result_parameters(
        text("N = 16\nNMATSUBARA = 32\nSITES = 1\nOUTFILE = \"out.h5\""));
    EXPECT_EQ(p.sites, 1);
    EXPECT_FALSE(p.checkpoint);
    EXPECT_EQ(p.outfile, std::string("out.h5"));
    alps::input::hirschfye_input_sizes const sizes = alps::input::read_hirschfye_input_sizes(text("NMATSUBARA = 32"));
    EXPECT_EQ(sizes.n_matsubara, 32);
    EXPECT_EQ(sizes.sites, 1);
    EXPECT_EQ(sizes.flavors, 2);
}

std::string const ctint_required = "NMATSUBARA = 64\nN = 32\nSWEEPS = 100\nTHERMALIZATION = 10\nBETA = 5\nU = 2\nALPHA = -0.01\n";

TEST(CtintInput, DmftDefaults) {
    alps::input::ctint_parameters const p = alps::input::read_ctint_parameters(text(ctint_required));
    EXPECT_EQ(p.max_order, 2048u);
    EXPECT_EQ(p.flavors, 2u);
    EXPECT_EQ(p.sites, 1u);
    EXPECT_EQ(p.n_matsubara, 64u);
    EXPECT_EQ(p.n_matsubara_measurements, 64u);
    EXPECT_EQ(p.n_tau, 32u);
    EXPECT_EQ(p.n_self, 320u);
    EXPECT_EQ(p.mc_steps, 100u);
    EXPECT_EQ(p.therm_steps, 10ul);
    EXPECT_EQ(p.nruns, 1u);
    EXPECT_EQ(p.max_time, 86400.);
    EXPECT_EQ(p.recalc_period, 5000u);
    EXPECT_EQ(p.measurement_period, 200u);
    EXPECT_EQ(p.convergence_check_period, 5000u);
    EXPECT_EQ(p.almost_zero, 1e-16);
    EXPECT_EQ(p.seed, 0);
    EXPECT_FALSE(p.histogram_measurement);
    EXPECT_FALSE(p.atomic);
    EXPECT_FALSE(p.tau_discretization_for_exp);
    EXPECT_EQ(alps::input::read_ctint_parameters(text(ctint_required + "SWEEP_MULTIPLICATOR = 3\nRECALC_PERIOD = 10")).mc_steps, 300u);
    EXPECT_EQ(alps::input::read_ctint_parameters(text(ctint_required + "RECALC_PERIOD = 10")).convergence_check_period, 10u);
    EXPECT_THROW(alps::input::read_ctint_parameters(text("NMATSUBARA = 64\nN = 32")), std::runtime_error);
}

TEST(CtintInput, ProgramAcceptsHybridizationNames) {
    alps::input::ctint_parameters const p = alps::input::read_ctint_parameters(typed(
        "N_MATSUBARA = 64\nN_TAU = 32\nN_ORBITALS = 4\nN_MEAS = 50\nSWEEPS = 100\nTHERMALIZATION = 10\n"
        "BETA = 5\nU = 2\nALPHA = -0.01\nINFILE = \"in.h5\"\nDMFT_FRAMEWORK = 1\nMAX_TIME = 30"));
    EXPECT_EQ(p.n_matsubara, 64u);
    EXPECT_EQ(p.n_tau, 32u);
    EXPECT_EQ(p.flavors, 4u);
    EXPECT_EQ(p.measurement_period, 50u);
    EXPECT_EQ(p.max_time, 30.);
    EXPECT_EQ(p.infile, "in.h5");
    EXPECT_TRUE(p.dmft_framework);
    alps::input::ctint_result_parameters const r = alps::input::read_ctint_result_parameters(typed(
        "NMATSUBARA = 64\nN = 32\nBETA = 5\nINFILE = \"in.h5\"\nHISTOGRAM_MEASUREMENT = 1"));
    EXPECT_EQ(r.n_self, 320u);
    EXPECT_TRUE(r.histogram_measurement);
    EXPECT_FALSE(r.dmft_framework);
    EXPECT_EQ(alps::input::read_interaction_max_time(typed("MAX_TIME = 30")), 30);
    EXPECT_THROW(alps::input::read_interaction_max_time(typed("N = 1")), std::runtime_error);
}

TEST(CtintInput, DmftResult) {
    alps::input::ctint_result_parameters const r = alps::input::read_ctint_result_parameters(text(
        "NMATSUBARA = 64\nN = 32\nBETA = 5\nCHECKPOINT = \"cp\"\nGET_COMPACTED_MEASUREMENTS = false"));
    EXPECT_EQ(r.n_matsubara_measurements, 64u);
    EXPECT_EQ(r.checkpoint, std::string("cp"));
    EXPECT_FALSE(r.get_compacted_measurements);
}

}
