// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#ifndef ALPS_INPUT_CTINT_HPP
#define ALPS_INPUT_CTINT_HPP

#include <alps/ngs/params.hpp>
#include <alps/parameter.h>
#include <alps/config.h>

#include <cstdint>
#include <optional>
#include <string>

namespace alps { namespace input {

    // The parameters of the interaction-expansion (CT-INT) impurity solver.
    // It is built into dmft (SOLVER = Interaction Expansion), reading
    // Parameters, and is the interaction program, reading params; the
    // program also accepts the hybridization solver's names N_ORBITALS,
    // N_MATSUBARA, N_TAU and N_MEAS.
    struct ctint_parameters {
        unsigned int max_order;                  // MAX_ORDER, default 2048
        unsigned int flavors;                    // FLAVORS, default 2
        unsigned int sites;                      // SITES, default 1
        unsigned int n_matsubara;                // NMATSUBARA
        unsigned int n_matsubara_measurements;   // NMATSUBARA_MEASUREMENTS, default NMATSUBARA
        unsigned int n_tau;                      // N
        unsigned int n_self;                     // NSELF: self-energy bins, default 10 N
        std::uint64_t mc_steps;                  // SWEEPS (times SWEEP_MULTIPLICATOR, default 1, in dmft)
        unsigned long therm_steps;               // THERMALIZATION
        unsigned int nruns;                      // NRUNS, default 1 (dmft)
        double max_time;                         // MAX_TIME in seconds, default 86400
        double beta;                             // BETA
        double U;                                // U
        double alpha;                            // ALPHA: shift of the auxiliary field
        unsigned int recalc_period;              // RECALC_PERIOD, default 5000
        unsigned int measurement_period;         // MEASUREMENT_PERIOD, default 200
        unsigned int convergence_check_period;   // CONVERGENCE_CHECK_PERIOD, default RECALC_PERIOD
        double almost_zero;                      // ALMOSTZERO, default 1e-16
        int seed;                                // SEED, default 0
        bool histogram_measurement;              // HISTOGRAM_MEASUREMENT, default false: measure in imaginary time
        bool atomic;                             // ATOMIC is set: start from the atomic limit
        std::optional<unsigned int> tau_discretization_for_exp;  // TAU_DISCRETIZATION_FOR_EXP: tabulate exp(i omega tau)
        std::string infile;                      // INFILE, the program's input with G0, unless ATOMIC
        bool dmft_framework;                     // DMFT_FRAMEWORK: G0 written by dmft (program)
    };

    ALPS_DECL ctint_parameters read_ctint_parameters(Parameters const & parameters);
    ALPS_DECL ctint_parameters read_ctint_parameters(params const & parameters);

    // What the solver reads when it collects its result.
    struct ctint_result_parameters {
        unsigned int n_matsubara;
        unsigned int n_matsubara_measurements;
        unsigned int n_tau;
        unsigned int n_self;
        unsigned int flavors;
        unsigned int sites;
        double beta;
        bool histogram_measurement;
        bool get_compacted_measurements;         // GET_COMPACTED_MEASUREMENTS, default true (dmft)
        std::optional<std::string> checkpoint;   // CHECKPOINT: write a checkpoint (dmft)
        std::string infile;                      // INFILE (program)
        bool dmft_framework;                     // DMFT_FRAMEWORK (program)
    };

    ALPS_DECL ctint_result_parameters read_ctint_result_parameters(Parameters const & parameters);
    ALPS_DECL ctint_result_parameters read_ctint_result_parameters(params const & parameters);

    // The interaction program's MAX_TIME, which it requires.
    ALPS_DECL int read_interaction_max_time(params const & parameters);

} }

#endif
