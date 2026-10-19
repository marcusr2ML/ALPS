/****************************************************************************
 *
 * ALPS DMFT Project
 *
 * Copyright (C) 2012 by Emanuel Gull <gull@pks.mpg.de>,
 *                       Hartmut Hafermann <hafermann@cpht.polytechnique.fr>
 *
 *  based on an earlier version by Philipp Werner and Emanuel Gull
 *
 *
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
 *
 *****************************************************************************/

#include <alps/input/hybridization.hpp>
#include <alps/solvers.hpp>
#include <alps/ngs.hpp>
#include <alps/parameter_input.hpp>
#ifdef ALPS_HAVE_MPI
#include <boost/mpi/environment.hpp>
#endif

int main(int argc, char** argv) {
  try {
    alps::mcoptions options(argc, argv, "hybridization");
    if (!options.valid) return 0;
#ifdef ALPS_HAVE_MPI
    boost::mpi::environment env(argc, argv);
#endif
    alps::params parms = alps::read_params_file(options.input_file);
    if (options.time_limit != 0)
      throw std::invalid_argument("time limit is passed in the parameter file!");
    alps::input::read_hybridization_max_time(parms);
    alps::solvers::cthyb(parms, options.output_file);
  } catch (std::exception const& exc) {
    std::cerr << exc.what() << '\n';
    return -1;
  } catch (...) {
    std::cerr << "Fatal Error: Unknown Exception!\n";
    return -2;
  }
  return 0;
}
