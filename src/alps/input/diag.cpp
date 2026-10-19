// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
#include <alps/input/diag.hpp>

#include <boost/algorithm/string/case_conv.hpp>

namespace alps { namespace input {

    namespace {

        bool equals(Parameters const & parms, std::string const & name, std::string const & value) {
            return parms.defined(name) && parms[name] == value;
        }

    }

    diag_parameters read_diag_parameters(Parameters const & parms) {
        diag_parameters result;
        if (parms.defined("CONSERVED_QUANTUMNUMBERS"))
            result.conserved_quantumnumbers = static_cast<std::string>(parms["CONSERVED_QUANTUMNUMBERS"]);
        result.translation_symmetry = parms.value_or_default("TRANSLATION_SYMMETRY", true);
        result.total_momentum = parms.defined("TOTAL_MOMENTUM");
        if (parms.defined("COMPLEX"))
            result.complex = static_cast<bool>(parms["COMPLEX"]);
        result.print_eigenvectors = parms.value_or_default("PRINT_EIGENVECTORS", false);
        result.save_eigenvectors = parms.value_or_default("SAVE_EIGENVECTORS", false);
        result.measure_energy = parms.value_or_default("MEASURE_ENERGY", true);
        if (parms.defined("NUMBER_EIGENVALUES"))
            result.number_eigenvalues = static_cast<int>(parms["NUMBER_EIGENVALUES"]);
        if (parms.defined("MAX_ITERATIONS"))
            result.max_iterations = static_cast<int>(parms["MAX_ITERATIONS"]);
        return result;
    }

    fulldiag_evaluation_parameters read_fulldiag_evaluation_parameters(Parameters const & p) {
        fulldiag_evaluation_parameters result;
        result.couple_mu = equals(p, "couple", "mu") || equals(p, "couple", "MU");
        result.field_name = result.couple_mu ? "mu" : "h";
        std::string const field_name_uc = boost::algorithm::to_upper_copy(result.field_name);

        result.measure_properties = p.value_or_default(result.couple_mu ? "MEASURE_CHARGE_PROPERTIES"
                                                                        : "MEASURE_MAGNETIC_PROPERTIES", 1);
        result.densities = p.value_or_default("DENSITIES", 1);
        result.versus_field = equals(p, "versus", result.field_name) || equals(p, "versus", field_name_uc);

        if (p.defined("T_MAX") && p.defined("DELTA_T"))
            result.temperatures = diag_range{p.value_or_default("T_MIN", 0.), p["T_MAX"], p["DELTA_T"]};
        if (p.defined("T"))
            result.beta = 1. / double(p["T"]);
        else if (p.defined("beta"))
            result.beta = static_cast<double>(p["beta"]);

        result.field = p.value_or_default(result.field_name, 0.);
        if (p.defined(field_name_uc + "_MAX") && p.defined("DELTA_" + field_name_uc))
            result.fields = diag_range{p.value_or_default(field_name_uc + "_MIN", 0.),
                                       p[field_name_uc + "_MAX"], p["DELTA_" + field_name_uc]};
        else
            result.fields = diag_range{result.field, result.field, 1.};
        return result;
    }

    std::optional<fulldiag_output_point> read_fulldiag_output_point(Parameters const & pa) {
        if (!pa.defined("T") && !pa.defined("beta"))
            return std::nullopt;
        return fulldiag_output_point{pa.defined("T") ? 1. / double(pa["T"]) : double(pa["beta"]),
                                     pa.value_or_default("H", pa.value_or_default("h", 0.))};
    }

} }
