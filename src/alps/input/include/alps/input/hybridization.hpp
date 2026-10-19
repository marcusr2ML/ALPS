// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#ifndef ALPS_INPUT_HYBRIDIZATION_HPP
#define ALPS_INPUT_HYBRIDIZATION_HPP

#include <alps/ngs/params.hpp>
#include <alps/config.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace alps { namespace input {

    // The parameters of the hybridization-expansion (CT-HYB) program. Its
    // parts are set up before the parameters are checked, so each part has
    // its own reader with only what it reads.

    // Throws, with a message naming the parameter, unless N_TAU, BETA,
    // N_MEAS, THERMALIZATION, SWEEPS, N_ORBITALS and the parameters of the
    // chosen measurements are set and consistent.
    ALPS_DECL void check_hybridization_parameters(params const & parameters);

    // MAX_TIME in seconds, which the program requires.
    ALPS_DECL long read_hybridization_max_time(params const & parameters);

    // N_TAU and N_ORBITALS.
    struct hybridization_sizes {
        int n_tau;
        int n_orbitals;
    };

    ALPS_DECL hybridization_sizes read_hybridization_sizes(params const & parameters);

    // The simulation and its measurements; read after the check.
    struct hybridization_parameters {
        bool verbose;                            // VERBOSE, default false
        int output_period;                       // OUTPUT_PERIOD, default 100000
        std::uint64_t thermalization_sweeps;     // THERMALIZATION
        std::uint64_t total_sweeps;              // SWEEPS
        std::size_t n_orbitals;                  // N_ORBITALS
        double beta;                             // BETA
        std::size_t n_meas;                      // N_MEAS: updates per measurement
        std::size_t n_hist_orders;               // N_HISTOGRAM_ORDERS, default 50
        std::size_t timeseries;                  // TIMESERIES, default 0: fixed instead of detailed binning
        std::size_t num_bins;                    // NUM_BINS, default 0
        bool spin_flip;                          // SPINFLIP, default false
        bool global_flip;                        // GLOBALFLIP, default false
        bool measure_nnt;                        // MEASURE_nnt, default false
        bool measure_nnw;                        // MEASURE_nnw, default false
        bool measure_nn;                         // MEASURE_nn, default false
        bool measure_g2w;                        // MEASURE_g2w, default false
        bool measure_h2w;                        // MEASURE_h2w, default false
        bool measure_time;                       // MEASURE_time, default true
        bool measure_freq;                       // MEASURE_freq, default false
        bool measure_legendre;                   // MEASURE_legendre, default false
        bool measure_sector_statistics;          // MEASURE_sector_statistics, default false
        bool compute_vertex;                     // COMPUTE_VERTEX, default false
        bool text_output;                        // TEXT_OUTPUT, default false
        bool accurate_covariance;                // ACCURATE_COVARIANCE, default true
        std::size_t n_w;                         // N_MATSUBARA, default 0
        std::size_t n_l;                         // N_LEGENDRE, default 0
        std::size_t n_t;                         // N_TAU
        std::size_t n_nn;                        // N_nn, default 0
        std::size_t n_w2;                        // N_w2, default 0
        std::size_t n_W;                         // N_W, default 0
        long max_time;                           // MAX_TIME
        std::string basepath;                    // BASEPATH, default empty: where results are written
        std::optional<std::string> ret_int_k;    // RET_INT_K
        std::optional<std::string> u_matrix;     // U_MATRIX
        std::optional<std::string> mu_vector;    // MU_VECTOR
    };

    ALPS_DECL hybridization_parameters read_hybridization_parameters(params const & parameters);

    // The interaction matrix: from the file U_MATRIX (HDF5 when
    // UMATRIX_IN_HDF5), or from U, J (default 0) and U' (default U-2J).
    // Throws when neither U_MATRIX nor U is set.
    struct hybridization_interaction {
        int n_orbitals;
        std::optional<std::string> u_matrix;
        bool u_matrix_in_hdf5;
        bool u_defined;
        double U;
        double J;
        double Uprime;
    };

    ALPS_DECL hybridization_interaction read_hybridization_interaction(params const & parameters);

    // The chemical potential: MU (default 0) for every orbital, or the file
    // MU_VECTOR (HDF5 when MU_IN_HDF5).
    struct hybridization_chemical_potential {
        int n_orbitals;
        double mu;
        bool mu_defined;
        std::optional<std::string> mu_vector;
        bool mu_in_hdf5;
    };

    ALPS_DECL hybridization_chemical_potential read_hybridization_chemical_potential(params const & parameters);

    // The hybridization function Delta(tau) from the file DELTA (HDF5 when
    // DELTA_IN_HDF5, as written by dmft when DMFT_FRAMEWORK). Throws for a
    // missing or zero N_TAU and a missing DELTA.
    struct hybridization_function_input {
        double beta;
        std::string file;
        bool in_hdf5;
        bool dmft_framework;
    };

    ALPS_DECL hybridization_function_input read_hybridization_function_input(params const & parameters);

    // The retarded interaction K(tau) from the file RET_INT_K (HDF5 when
    // K_IN_HDF5), when it is set. Throws for a missing or zero N_TAU then.
    struct retarded_interaction_input {
        bool used;
        double beta;
        std::string file;
        bool in_hdf5;
    };

    ALPS_DECL retarded_interaction_input read_retarded_interaction_input(params const & parameters);

    // What the local configuration and the hybridization matrix read.
    struct hybridization_configuration_parameters {
        double beta;                             // BETA
        int n_orbitals;                          // N_ORBITALS
        bool retarded_interaction;               // RET_INT_K is set
        int measure_g2w;                         // MEASURE_g2w, default 0
        int measure_h2w;                         // MEASURE_h2w, default 0
    };

    ALPS_DECL hybridization_configuration_parameters read_hybridization_configuration(params const & parameters);

} }

#endif
