/*****************************************************************************
*
* ALPS Project Applications
*
* Copyright (C) 2026 ALPS Collaboration
*
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
*
*****************************************************************************/

// Checks that maxent::params reads parameter files and command lines the way
// ALPSCore's alps::params does (see maxent_parameters.hpp).
#include "maxent_parameters.hpp"
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <cstdlib>

int main() {
  const char* tmpdir = std::getenv("TMPDIR");
  const std::string fname = std::string(tmpdir ? tmpdir : "/tmp") + "/maxent_parameters_test.param";
  const char* f = fname.c_str();
  {
    std::ofstream o(f);
    o << "# comment\nBETA=8  #inverse temperature\nNDAT = 40\nDATA=\"my file.dat\"\n; other\n"
         "KERNEL = fermionic ; trailing\nX_0=1.5\nEMPTY=\nSEED=7\nSPLIT=1\\\n2\n[help]\nmodels=true\n";
  }
  const char* argv[] = {"./maxent", f, "--NDAT=20", "--VERBOSE", "--ALPHA_MIN=0.5"};
  maxent::params p(5, argv);
  p.define<double>("BETA", "beta");
  p.define<int>("NDAT", "ndat");
  p.define<std::string>("DATA", "", "data");
  p.define<std::string>("KERNEL", "bosonic", "kernel");
  p.define<bool>("VERBOSE", false, "v");
  p.define<double>("ALPHA_MIN", 0.01, "a");
  p.define<double>("ALPHA_MAX", 20, "a");
  p.define<double>("OMEGA_MIN", "required, missing");
  p.define<std::string>("EMPTY", "x", "e");
  p.define("help.models", "models");
  p.define<unsigned int>("SEED", 0, "seed");
  p.define<int>("SPLIT", 0, "continuation");
  p.define<std::string>("BASENAME", "", "b");

  assert(static_cast<double>(p["BETA"]) == 8.0);
  int ndat = p["NDAT"]; assert(ndat == 20);                // command line wins
  std::string data = p["DATA"]; assert(data == "my file.dat");
  assert(p["KERNEL"] == "fermionic");
  assert(p["VERBOSE"] == true && p["VERBOSE"]);
  double amin = p["ALPHA_MIN"]; assert(amin == 0.5);
  double amax = p["ALPHA_MAX"]; assert(amax == 20.0);      // int default read as double
  assert(!p.exists("OMEGA_MIN") && p.defined("OMEGA_MIN"));
  double omega_min = p.exists("OMEGA_MIN") ? p["OMEGA_MIN"] : -10.0; assert(omega_min == -10.0);
  assert(p["EMPTY"] == "");
  assert(p["help.models"] == true);
  assert(p["SEED"].as<std::mt19937::result_type>() == 7u);
  assert(p["SPLIT"].as<int>() == 12);
  assert(p.defaulted("BASENAME") && !p.defaulted("BETA"));
  assert(p.supplied("X_0") && !p.defined("X_1"));
  p.define<double>("X_0", "");
  assert(static_cast<double>(p["X_0"]) == 1.5);
  assert(p.errors().size() == 1);                          // OMEGA_MIN
  p["BASENAME"] = std::string("run");
  p["DEFAULT_MODEL"] = "flat";
  assert(p["BASENAME"].as<std::string>() == "run");
  bool threw = false;
  try { int bad = p["BETA"]; (void)bad; } catch (const std::invalid_argument&) { threw = true; }
  assert(threw);                                            // no double -> int
  assert(maxent::remove_extensions("dir/in.param") == "dir/in");
  assert(maxent::remove_extensions("a.b.c") == "a");
  assert(p.origin_name() == f);
  const char* help[] = {"./maxent", "--help"};
  maxent::params h(2, help);
  h.description("Maxent test");
  h.define<int>("N_ALPHA", 60, "Number of alpha samples");
  assert(h.help_requested(std::cout));
  std::remove(f);
  std::printf("maxent_parameters_test: PASS\n");
}
