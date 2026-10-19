// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#include <alps/input/dmrg.hpp>

#include <alps/expression.h>

#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <boost/tokenizer.hpp>

#include <cstddef>
#include <stdexcept>

namespace alps { namespace input {

    dmrg_parameters read_dmrg_parameters(Parameters const & parms) {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(" ,");
        dmrg_parameters result;

        result.number_eigenvalues = parms.value_or_default("NUMBER_EIGENVALUES", 1);
        result.sweeps = parms.value_or_default("SWEEPS", 4);
        result.start_sweep = parms.value_or_default("START_SWEEP", 0);
        result.start_dir = parms.value_or_default("START_DIR", 0);
        if (result.start_dir != 0 && result.start_dir != 1)
            boost::throw_exception(std::runtime_error("START_DIR can assume the values 0 (left-to-right) or 1 (right-to-left)"));
        result.start_iter = parms.value_or_default("START_ITER", 1);
        result.verbose = parms.value_or_default("VERBOSE", 0);

        result.warmup_states = 20;
        if (parms.defined("NUM_WARMUP_STATES"))
            result.warmup_states = static_cast<int>(parms["NUM_WARMUP_STATES"]);
        result.truncation_error = -1.;
        if (parms.defined("TRUNCATION_ERROR"))
            result.truncation_error = static_cast<double>(parms["TRUNCATION_ERROR"]);
        result.lanczos_tolerance = -1.;
        if (parms.defined("LANCZOS_TOLERANCE"))
            result.lanczos_tolerance = static_cast<double>(parms["LANCZOS_TOLERANCE"]);

        int const half_sweeps = 2 * result.sweeps;
        result.maxstates = -1;
        if (parms.defined("STATES")) {
            std::string states_string = parms["STATES"];
            tokenizer state_tokens(states_string, sep);
            for (tokenizer::const_iterator it = state_tokens.begin(); it != state_tokens.end(); ++it)
                result.states.push_back(boost::lexical_cast<int>(*it));
            if (result.states.size() < static_cast<std::size_t>(half_sweeps))
                boost::throw_exception(std::runtime_error("Need to specify either 2*SWEEPS different values in STATES, or one MAXSTATES value"));
        } else if (parms.defined("MAXSTATES")) {
            // MAXSTATES grows the states linearly up to the maximum.
            result.maxstates = static_cast<int>(parms["MAXSTATES"]);
            for (int i = 0; i < half_sweeps; ++i)
                result.states.push_back((i + 1) * result.maxstates / half_sweeps);
        } else if (parms.defined("NUMSTATES")) {
            result.maxstates = static_cast<int>(parms["NUMSTATES"]);
            for (int i = 0; i < half_sweeps; ++i)
                result.states.push_back(result.maxstates);
        } else
            boost::throw_exception(std::runtime_error("Need to specify either 2*SWEEPS different values in STATES, or one MAXSTATES value"));

        if (parms.defined("CONSERVED_QUANTUMNUMBERS")) {
            std::string qn_string = parms["CONSERVED_QUANTUMNUMBERS"];
            tokenizer qn_tokens(qn_string, sep);
            for (tokenizer::const_iterator it = qn_tokens.begin(); it != qn_tokens.end(); ++it)
                if (parms.defined(*it + "_total"))
                    result.conserved_totals.push_back(std::make_pair(*it,
                        alps::evaluate<double>(static_cast<std::string>(parms[*it + "_total"]), parms)));
        }

        if (parms.defined("TEMP_DIRECTORY"))
            result.temp_directory = static_cast<std::string>(parms["TEMP_DIRECTORY"]);
        return result;
    }

    bool dmrg_uses_complex(Parameters const & parms) {
        return parms.value_or_default("COMPLEX", false);
    }

} }
