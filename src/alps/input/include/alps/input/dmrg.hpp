// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#ifndef ALPS_INPUT_DMRG_HPP
#define ALPS_INPUT_DMRG_HPP

#include <alps/parameter.h>
#include <alps/config.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace alps { namespace input {

    // The parameters the dmrg application reads itself, with their defaults.
    // Lattice and model parameters are read by the lattice and model
    // libraries.
    struct dmrg_parameters {
        int number_eigenvalues;              // NUMBER_EIGENVALUES, default 1
        int sweeps;                          // SWEEPS, default 4
        int start_sweep;                     // START_SWEEP, default 0
        int start_dir;                       // START_DIR, 0 (left to right, default) or 1
        int start_iter;                      // START_ITER, default 1
        int verbose;                         // VERBOSE, default 0
        int warmup_states;                   // NUM_WARMUP_STATES, default 20
        double truncation_error;             // TRUNCATION_ERROR, -1 when unset
        double lanczos_tolerance;            // LANCZOS_TOLERANCE, -1 when unset
        int maxstates;                       // MAXSTATES or NUMSTATES, -1 when STATES is given
        std::vector<int> states;             // states kept in each half sweep
        // CONSERVED_QUANTUMNUMBERS that have a <name>_total, in the order
        // listed, with the evaluated total.
        std::vector<std::pair<std::string, double> > conserved_totals;
        std::optional<std::string> temp_directory;   // TEMP_DIRECTORY
    };

    // Throws for an invalid START_DIR and when none of STATES (2*SWEEPS
    // values), MAXSTATES or NUMSTATES is given.
    ALPS_DECL dmrg_parameters read_dmrg_parameters(Parameters const & parameters);

    // COMPLEX, default false.
    ALPS_DECL bool dmrg_uses_complex(Parameters const & parameters);

} }

#endif
