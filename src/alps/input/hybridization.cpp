// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#include <alps/input/hybridization.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>

#include <stdexcept>

namespace alps { namespace input {

    namespace {

        std::optional<std::string> optional_string(params const & parms, std::string const & name) {
            if (!parms.defined(name))
                return std::nullopt;
            return parms[name].cast<std::string>();
        }

        bool flag(params const & parms, std::string const & name) {
            return parms.defined(name) && parms[name].cast<bool>();
        }

        void check_time_slices(params const & p, std::string const & what) {
            if (!p.defined("N_TAU") || (int)(p["N_TAU"]) == 0)
                boost::throw_exception(std::invalid_argument("define parameter N_TAU, the number of " + what + " time slices!"));
        }

    }

    void check_hybridization_parameters(params const & parms) {
        // These checks are likely not complete: passing them does not
        // guarantee that all parameters are meaningful.
        if (!parms.defined("N_TAU")) boost::throw_exception(std::invalid_argument("please specify the parameter N_TAU"));
        if (!parms.defined("BETA")) boost::throw_exception(std::invalid_argument("please specify parameter BETA for inverse temperature"));
        if (!parms.defined("N_MEAS")) boost::throw_exception(std::invalid_argument("please specify parameter N_MEAS for measurement interval"));
        if (!parms.defined("THERMALIZATION") ||
            !parms.defined("SWEEPS") ||
            !parms.defined("N_ORBITALS")) boost::throw_exception(std::invalid_argument("please specify parameters THERMALIZATION, SWEEPS, and N_ORBITALS"));

        // parameters that the chosen measurements require
        if ((parms["MEASURE_freq"] | false) && !parms.defined("N_MATSUBARA")) boost::throw_exception(std::invalid_argument("please specify parameter N_MATSUBARA for # of Matsubara frequencies to be measured"));
        if ((parms["MEASURE_legendre"] | false) && !parms.defined("N_LEGENDRE")) boost::throw_exception(std::invalid_argument("please specify parameter N_LEGENDRE for # of Legendre coefficients to be measured"));
        if ((parms["MEASURE_legendre"] | false) && !parms.defined("N_MATSUBARA")) boost::throw_exception(std::invalid_argument("please specify parameter N_MATSUBARA for # of Matsubara frequencies"));
        if ((parms["MEASURE_nnt"] | false) && !parms.defined("N_nn")) boost::throw_exception(std::invalid_argument("please specify the parameter N_nn for # of imaginary time points for the density-density correlator"));
        if ((parms["MEASURE_nnw"] | false) && !parms.defined("N_W")) boost::throw_exception(std::invalid_argument("please specify the parameter N_W for # of bosonic frequencies for the density-density correlator"));
        if ((parms["MEASURE_g2w"] | false) || (parms["MEASURE_h2w"] | false)) {
            if (!parms.defined("N_w2")) boost::throw_exception(std::invalid_argument("please specify the parameter N_w2 for # of fermionic Matsubara frequencies for two-particle functions"));
            if (!parms.defined("N_W")) boost::throw_exception(std::invalid_argument("please specify the parameter N_W for # of bosonic Matsubara frequencies for two-particle functions"));
            if ((int)parms["N_w2"] % 2 != 0) boost::throw_exception(std::invalid_argument("parameter N_w2 must be even"));
        }
        if (parms["COMPUTE_VERTEX"] | false) {
            if (!(parms["MEASURE_freq"] | false)) boost::throw_exception(std::invalid_argument("frequency measurement is required for computing the vertex, please set MEASURE_freq=1"));
            if (!((parms["MEASURE_g2w"] | false) || (parms["MEASURE_h2w"] | false))) boost::throw_exception(std::invalid_argument("at least one two-particle quantity is required for computing the vertex, set MEASURE_g2w=1 or MEASURE_h2w=1"));
            if ((int)parms["N_MATSUBARA"] < ((int)parms["N_w2"] / 2 + (int)parms["N_W"] - 1)) boost::throw_exception(std::invalid_argument("for computing the vertex, N_MATSUBARA must be at least N_w2/2+N_W-1"));
        }
    }

    long read_hybridization_max_time(params const & parms) {
        if (!parms.defined("MAX_TIME"))
            boost::throw_exception(std::runtime_error("parameter MAX_TIME is not defined. How long do you want to run the code for? (in seconds)"));
        return (long)parms["MAX_TIME"];
    }

    hybridization_sizes read_hybridization_sizes(params const & p) {
        hybridization_sizes result;
        result.n_tau = p["N_TAU"];
        result.n_orbitals = p["N_ORBITALS"];
        return result;
    }

    hybridization_parameters read_hybridization_parameters(params const & parms) {
        hybridization_parameters result;
        result.verbose = (parms["VERBOSE"] | false);
        result.output_period = parms["OUTPUT_PERIOD"] | 100000;
        result.thermalization_sweeps = parms["THERMALIZATION"];
        result.total_sweeps = parms["SWEEPS"];
        result.n_orbitals = parms["N_ORBITALS"];
        result.beta = parms["BETA"];
        result.n_meas = parms["N_MEAS"];
        result.n_hist_orders = parms["N_HISTOGRAM_ORDERS"] | 50;
        result.timeseries = parms["TIMESERIES"] | 0;
        result.num_bins = parms["NUM_BINS"] | 0;
        result.spin_flip = parms["SPINFLIP"] | 0;
        result.global_flip = parms["GLOBALFLIP"] | 0;
        result.measure_nnt = parms["MEASURE_nnt"] | 0;
        result.measure_nnw = parms["MEASURE_nnw"] | 0;
        result.measure_nn = parms["MEASURE_nn"] | 0;
        result.measure_g2w = parms["MEASURE_g2w"] | 0;
        result.measure_h2w = parms["MEASURE_h2w"] | 0;
        result.measure_time = parms["MEASURE_time"] | 1;
        result.measure_freq = parms["MEASURE_freq"] | 0;
        result.measure_legendre = parms["MEASURE_legendre"] | 0;
        result.measure_sector_statistics = parms["MEASURE_sector_statistics"] | 0;
        result.compute_vertex = parms["COMPUTE_VERTEX"] | false;
        result.text_output = parms["TEXT_OUTPUT"] | false;
        result.accurate_covariance = parms["ACCURATE_COVARIANCE"] | true;
        result.n_w = parms["N_MATSUBARA"] | 0;
        result.n_l = parms["N_LEGENDRE"] | 0;
        result.n_t = parms["N_TAU"];
        result.n_nn = parms["N_nn"] | 0;
        result.n_w2 = parms["N_w2"] | 0;
        result.n_W = parms["N_W"] | 0;
        result.max_time = (long)parms["MAX_TIME"];
        result.basepath = boost::lexical_cast<std::string>(parms["BASEPATH"] | "");
        result.ret_int_k = optional_string(parms, "RET_INT_K");
        result.u_matrix = optional_string(parms, "U_MATRIX");
        result.mu_vector = optional_string(parms, "MU_VECTOR");
        return result;
    }

    hybridization_interaction read_hybridization_interaction(params const & p) {
        hybridization_interaction result;
        result.n_orbitals = p["N_ORBITALS"];
        result.u_defined = p.defined("U");
        result.u_matrix = optional_string(p, "U_MATRIX");
        result.u_matrix_in_hdf5 = result.u_matrix && flag(p, "UMATRIX_IN_HDF5");
        result.U = result.J = result.Uprime = 0.;
        if (!result.u_matrix) {
            if (!result.u_defined)
                boost::throw_exception(std::invalid_argument("please specify either U (and, optionally, U' and J) or a file U_MATRIX in your parameter file"));
            result.U = (double)(p["U"]);
            result.J = (double)(p["J"] | 0.);
            result.Uprime = (p["U'"] | (result.U - 2 * result.J));
        }
        return result;
    }

    hybridization_chemical_potential read_hybridization_chemical_potential(params const & p) {
        hybridization_chemical_potential result;
        result.n_orbitals = p["N_ORBITALS"];
        result.mu = p["MU"] | 0.;
        result.mu_defined = p.defined("MU");
        result.mu_vector = optional_string(p, "MU_VECTOR");
        result.mu_in_hdf5 = result.mu_vector && flag(p, "MU_IN_HDF5");
        return result;
    }

    hybridization_function_input read_hybridization_function_input(params const & p) {
        check_time_slices(p, "hybridization");
        hybridization_function_input result;
        result.beta = p["BETA"];
        if (!p.defined("DELTA"))
            boost::throw_exception(std::invalid_argument(std::string("Parameter DELTA missing, filename for hybridization function not specified.")));
        result.file = p["DELTA"].cast<std::string>();
        result.in_hdf5 = flag(p, "DELTA_IN_HDF5");
        result.dmft_framework = result.in_hdf5 && flag(p, "DMFT_FRAMEWORK");
        return result;
    }

    retarded_interaction_input read_retarded_interaction_input(params const & p) {
        retarded_interaction_input result;
        result.used = p.defined("RET_INT_K");
        result.beta = 0.;
        result.in_hdf5 = false;
        if (!result.used)
            return result;
        check_time_slices(p, "retarded interaction");
        result.beta = p["BETA"];
        result.file = p["RET_INT_K"].cast<std::string>();
        result.in_hdf5 = flag(p, "K_IN_HDF5");
        return result;
    }

    hybridization_configuration_parameters read_hybridization_configuration(params const & p) {
        hybridization_configuration_parameters result;
        result.beta = p["BETA"];
        result.n_orbitals = p["N_ORBITALS"];
        result.retarded_interaction = p.defined("RET_INT_K");
        result.measure_g2w = p["MEASURE_g2w"] | 0;
        result.measure_h2w = p["MEASURE_h2w"] | 0;
        return result;
    }

} }
