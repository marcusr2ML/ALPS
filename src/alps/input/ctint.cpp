// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#include <alps/input/ctint.hpp>

#include <boost/throw_exception.hpp>

#include <stdexcept>

namespace alps { namespace input {

    ctint_parameters read_ctint_parameters(Parameters const & parms) {
        ctint_parameters result;
        result.max_order = parms.value_or_default("MAX_ORDER", 2048);
        result.flavors = parms.value_or_default("FLAVORS", 2);
        result.sites = parms.value_or_default("SITES", 1);
        result.n_matsubara = (int)parms["NMATSUBARA"];
        result.n_matsubara_measurements = parms.value_or_default("NMATSUBARA_MEASUREMENTS", result.n_matsubara);
        result.n_tau = (int)parms["N"];
        result.n_self = parms.value_or_default("NSELF", 10 * result.n_tau);
        result.mc_steps = (std::uint64_t)parms["SWEEPS"] * (std::uint64_t)parms.value_or_default("SWEEP_MULTIPLICATOR", 1L);
        result.therm_steps = (unsigned int)parms["THERMALIZATION"];
        result.nruns = parms.value_or_default("NRUNS", 1);
        result.max_time = parms.value_or_default("MAX_TIME", 86400);
        result.beta = (double)parms["BETA"];
        result.U = (double)parms["U"];
        result.alpha = (double)parms["ALPHA"];
        result.recalc_period = parms.value_or_default("RECALC_PERIOD", 5000);
        result.measurement_period = parms.value_or_default("MEASUREMENT_PERIOD", 200);
        result.convergence_check_period = parms.value_or_default("CONVERGENCE_CHECK_PERIOD", result.recalc_period);
        result.almost_zero = parms.value_or_default("ALMOSTZERO", 1.e-16);
        result.seed = parms.value_or_default("SEED", 0);
        result.histogram_measurement = parms.value_or_default("HISTOGRAM_MEASUREMENT", false);
        result.atomic = parms.defined("ATOMIC");
        if (parms.defined("TAU_DISCRETIZATION_FOR_EXP"))
            result.tau_discretization_for_exp = static_cast<unsigned int>(parms["TAU_DISCRETIZATION_FOR_EXP"]);
        result.dmft_framework = false;
        return result;
    }

    ctint_parameters read_ctint_parameters(params const & parms) {
        ctint_parameters result;
        result.max_order = parms["MAX_ORDER"] | 2048;
        result.flavors = parms["FLAVORS"] | (parms["N_ORBITALS"] | 2);
        result.sites = parms["SITES"] | 1;
        result.n_matsubara = (int)(parms["NMATSUBARA"] | parms["N_MATSUBARA"]);
        result.n_matsubara_measurements = parms["NMATSUBARA_MEASUREMENTS"] | (int)result.n_matsubara;
        result.n_tau = (int)(parms["N"] | parms["N_TAU"]);
        result.n_self = parms["NSELF"] | (int)(10 * result.n_tau);
        result.mc_steps = (std::uint64_t)parms["SWEEPS"];
        result.therm_steps = (unsigned int)parms["THERMALIZATION"];
        result.nruns = 1;
        result.max_time = parms["MAX_TIME"] | 86400;
        result.beta = (double)parms["BETA"];
        result.U = (double)parms["U"];
        result.alpha = (double)parms["ALPHA"];
        result.recalc_period = parms["RECALC_PERIOD"] | 5000;
        result.measurement_period = parms["MEASUREMENT_PERIOD"] | (parms["N_MEAS"] | 200);
        result.convergence_check_period = parms["CONVERGENCE_CHECK_PERIOD"] | (int)result.recalc_period;
        result.almost_zero = parms["ALMOSTZERO"] | 1.e-16;
        result.seed = parms["SEED"] | 0;
        result.histogram_measurement = parms["HISTOGRAM_MEASUREMENT"] | false;
        result.atomic = parms.defined("ATOMIC");
        if (parms.defined("TAU_DISCRETIZATION_FOR_EXP"))
            result.tau_discretization_for_exp = static_cast<unsigned int>(parms["TAU_DISCRETIZATION_FOR_EXP"]);
        result.dmft_framework = false;
        if (!result.atomic) {
            result.infile = parms["INFILE"].cast<std::string>();
            result.dmft_framework = parms.defined("DMFT_FRAMEWORK") && parms["DMFT_FRAMEWORK"].cast<bool>();
        }
        return result;
    }

    ctint_result_parameters read_ctint_result_parameters(Parameters const & parms) {
        ctint_result_parameters result;
        result.n_matsubara = parms["NMATSUBARA"];
        result.n_matsubara_measurements = parms.value_or_default("NMATSUBARA_MEASUREMENTS", result.n_matsubara);
        result.n_tau = parms["N"];
        result.n_self = parms.value_or_default("NSELF", 10 * result.n_tau);
        result.flavors = parms.value_or_default("FLAVORS", 2);
        result.sites = parms.value_or_default("SITES", 1);
        result.beta = parms["BETA"];
        result.histogram_measurement = parms.value_or_default("HISTOGRAM_MEASUREMENT", false);
        result.get_compacted_measurements = parms.value_or_default("GET_COMPACTED_MEASUREMENTS", true);
        if (parms.defined("CHECKPOINT"))
            result.checkpoint = static_cast<std::string>(parms["CHECKPOINT"]);
        result.dmft_framework = false;
        return result;
    }

    ctint_result_parameters read_ctint_result_parameters(params const & parms) {
        ctint_result_parameters result;
        result.n_matsubara = parms["NMATSUBARA"] | parms["N_MATSUBARA"];
        result.n_matsubara_measurements = parms["NMATSUBARA_MEASUREMENTS"] | (int)result.n_matsubara;
        result.n_tau = parms["N"] | parms["N_TAU"];
        result.n_self = parms["NSELF"] | (int)(10 * result.n_tau);
        result.flavors = parms["FLAVORS"] | (parms["N_ORBITALS"] | 2);
        result.sites = parms["SITES"] | 1;
        result.beta = parms["BETA"];
        result.histogram_measurement = parms["HISTOGRAM_MEASUREMENT"] | false;
        result.get_compacted_measurements = true;
        result.infile = parms["INFILE"].cast<std::string>();
        result.dmft_framework = parms.defined("DMFT_FRAMEWORK") && static_cast<bool>(parms["DMFT_FRAMEWORK"]);
        return result;
    }

    int read_interaction_max_time(params const & parms) {
        if (!parms.defined("MAX_TIME"))
            boost::throw_exception(std::runtime_error("parameter MAX_TIME is not defined. How long do you want to run the code for? (in seconds)"));
        return (int)parms["MAX_TIME"];
    }

} }
