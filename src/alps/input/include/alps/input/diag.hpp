// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#ifndef ALPS_INPUT_DIAG_HPP
#define ALPS_INPUT_DIAG_HPP

#include <alps/parameter.h>
#include <alps/config.h>

#include <optional>
#include <string>

namespace alps { namespace input {

    // The parameters the exact diagonalization applications (sparsediag,
    // fulldiag) read themselves, with their defaults. Lattice and model
    // parameters are read by the lattice and model libraries.
    struct diag_parameters {
        std::string conserved_quantumnumbers;    // CONSERVED_QUANTUMNUMBERS, empty when unset
        bool translation_symmetry;               // TRANSLATION_SYMMETRY, default true
        bool total_momentum;                     // TOTAL_MOMENTUM is set: one momentum sector
        std::optional<bool> complex;             // COMPLEX; unset, the lattice decides
        bool print_eigenvectors;                 // PRINT_EIGENVECTORS, default false
        bool save_eigenvectors;                  // SAVE_EIGENVECTORS, default false
        bool measure_energy;                     // MEASURE_ENERGY, default true
        std::optional<int> number_eigenvalues;   // NUMBER_EIGENVALUES
        std::optional<int> max_iterations;       // MAX_ITERATIONS of sparsediag's Lanczos
    };

    ALPS_DECL diag_parameters read_diag_parameters(Parameters const & parameters);

    // A temperature or field grid from MIN (default 0) to MAX in steps of DELTA.
    struct diag_range {
        double min;
        double max;
        double delta;
    };

    // The parameters of fulldiag_evaluate, read from the task's parameters
    // with the command-line options added.
    struct fulldiag_evaluation_parameters {
        bool couple_mu;                          // couple = mu or MU: field -mu N instead of -h Sz
        std::string field_name;                  // "mu" or "h"
        bool measure_properties;                 // MEASURE_CHARGE_PROPERTIES or MEASURE_MAGNETIC_PROPERTIES, default true
        bool densities;                          // DENSITIES, default true
        bool versus_field;                       // versus = the field name, in either case; otherwise versus T
        std::optional<diag_range> temperatures;  // T_MIN, T_MAX and DELTA_T, when T_MAX and DELTA_T are set
        std::optional<double> beta;              // 1/T, or beta
        double field;                            // the field (h or mu), default 0
        diag_range fields;                       // <FIELD>_MIN, _MAX and DELTA_<FIELD>; otherwise the one field
    };

    ALPS_DECL fulldiag_evaluation_parameters read_fulldiag_evaluation_parameters(Parameters const & parameters);

    // beta (1/T, or beta) and the field (H, or h, default 0) of a result
    // written with all its XML, when T or beta is set.
    struct fulldiag_output_point {
        double beta;
        double field;
    };

    ALPS_DECL std::optional<fulldiag_output_point> read_fulldiag_output_point(Parameters const & parameters);

} }

#endif
