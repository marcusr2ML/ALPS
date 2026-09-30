// Copyright (C) 2026 by the ALPS collaboration
// SPDX-License-Identifier: MIT
//
// pyalps.maxent_c: runs Maxent (tool/maxent) on a Python parameter dict.
//
//   AnalyticContinuation(parms)
//
// takes the parameters of the maxent program (see `maxent --help`) and
// writes <BASENAME>.out.h5, plus the text files unless TEXT_OUTPUT is false.
// Values are handed to Maxent the way a parameter file would supply them,
// so each is parsed with the type Maxent defines for it.
#include "dict_to_params.hpp"
#include "maxent.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <cstdio>
#include <string>

namespace nb = nanobind;

namespace {

std::string raw_value(nb::handle value, std::string const & key) {
    using pyalps::detail::scalar_kind;
    switch (pyalps::detail::classify_scalar(value)) {
        case scalar_kind::boolean: {
            int const truth = PyObject_IsTrue(value.ptr());
            if (truth < 0)
                throw nb::python_error();
            return truth ? "true" : "false";
        }
        case scalar_kind::integer:
            return std::to_string(pyalps::detail::integer_value(value, key));
        case scalar_kind::real: {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%.17g", pyalps::detail::real_value(value, key));
            return buffer;
        }
        case scalar_kind::string:
            return pyalps::detail::string_value(value);
        default:
            throw nb::type_error(("parameter '" + key
                + "' must be a bool, integer, float or string").c_str());
    }
}

void analytic_continuation(nb::dict const & parms_in) {
    maxent::params parms;
    for (auto item : parms_in) {
        std::string const key = nb::cast<std::string>(nb::str(item.first));
        parms.supply(key, raw_value(item.second, key));
    }
    if (!parms.supplied("BASENAME"))
        parms.supply("BASENAME", "results");
    MaxEntSimulation::define_parameters(parms);

    // the same checks as the maxent program
    if (!parms.exists("BETA"))
        throw nb::value_error("Please supply BETA");
    if (!parms.exists("NDAT"))
        throw nb::value_error("Please supply NDAT");
    if (parms["DATA"].as<std::string>().empty() && !parms.exists("X_0")
        && parms["DATA_IN_HDF5"] != true)
        throw nb::value_error("Please supply input data");

    MaxEntSimulation sim(parms);
    sim.run();
    sim.evaluate();
}

} // namespace

NB_MODULE(maxent_c, m) {
    m.def("AnalyticContinuation", &analytic_continuation, nb::arg("parms"),
          "Run Maxent analytic continuation with the given parameters.");
}
