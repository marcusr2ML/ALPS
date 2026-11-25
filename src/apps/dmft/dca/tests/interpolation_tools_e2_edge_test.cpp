// Copyright (C) 2026 by the ALPS collaboration
// ALPS Project: https://alps.comp-phys.org/
// SPDX-License-Identifier: MIT

// Edge cases of what the CHANGELOG entry for #471 says the interpolation
// tools refuse (#491): a self-energy that is not on fermionic Matsubara
// frequencies, and FLAVORS other than 2 in either parameter file, for both
// sigma_interpolate_normal and sigma_interpolate_sc. The helpers follow
// interpolation_tools_test.cpp.

#include <complex>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

#include "gtest/gtest.h"

#include "alps/hdf5.hpp"
#include "alps/params.hpp"

#include "cluster_transform.h"

namespace {

namespace fs = std::filesystem;

const double beta = 5.;
const int n_old = 64;

void write_parameters(const fs::path& file, int n_matsubara, const std::string& extra = "") {
  std::ofstream f(file);
  f << extra;
  f << "BETA = " << beta << "\nMU = 0.3\nU = 2\nt = 1\nN = 100\nNMATSUBARA = " << n_matsubara << "\n"
    << "dca.LATTICE = Betts2D-4A\ndca.SITES = 4\ndca.L = 8\n"
    << "dca.LATTICE_LIBRARY = " << DCA_CLUSTER_LIBRARY << "\n"
    << "dca.G0_OMEGA_INPUT = G0.h5\ndca.G0_OMEGA_OUTPUT = G0.h5\n";
}

alps::params transformer_parameters() {
  alps::params p;
  p["dca.L"] = 8;
  p["dca.LATTICE"] = "Betts2D-4A";
  p["dca.SITES"] = 4;
  p["dca.LATTICE_LIBRARY"] = DCA_CLUSTER_LIBRARY;
  p["MU"] = 0.3;
  p["t"] = 1.;
  p["tprime"] = 0.;
  p["U"] = 2.;
  p["V"] = 0.;
  p["BETA"] = beta;
  p["FLAVORS"] = 2;
  p["NMATSUBARA"] = n_old;
  p["N"] = 100;
  p["dca.G0_OMEGA_INPUT"] = "/tmp/none";
  p["dca.G0_OMEGA_OUTPUT"] = "/tmp/none";
  p["dca.INITIAL_G0"] = "FREE";
  p["dca.AFM_SELFCONSISTENCY"] = false;
  p["dca.CO_SELFCONSISTENCY"] = false;
  p["dca.H_INIT"] = 0.;
  p["dca.H"] = 0.;
  p["dca.ETA1_INIT"] = 0.;
  p["dca.ETA2_INIT"] = 0.;
  p["dca.ETA1"] = 0.;
  p["dca.ETA2"] = 0.;
  return p;
}

std::complex<double> sigma_value(int w, int k, int a, int b) {
  const std::complex<double> iw(0., (2 * w + 1) * M_PI / beta);
  return (0.3 + 0.1 * k + 0.05 * a - 0.02 * b) + (1.5 + 0.1 * a * b) / iw;
}

void write_normal_sigma(alps::hdf5::archive& ar, alps::gf::statistics::statistics_type stat) {
  NormalStateClusterTransformer T(transformer_parameters());
  cluster_matsubara_kspace_gf sigma(alps::gf::omega_k_sigma_gf(
      alps::gf::matsubara_positive_mesh(beta, n_old, stat), T.momentum_mesh(), alps::gf::index_mesh(2)));
  for (alps::gf::matsubara_index w(0); w < n_old; ++w)
    for (alps::gf::momentum_index k(0); k < 4; ++k)
      for (alps::gf::index f(0); f < 2; ++f) sigma(w, k, f) = sigma_value(w(), k(), f(), 0);
  sigma.save(ar, "/Sigma");
}

void write_nambu_sigma(alps::hdf5::archive& ar, alps::gf::statistics::statistics_type stat) {
  NambuClusterTransformer T(transformer_parameters());
  cluster_nambu_matsubara_kspace_gf sigma(alps::gf::omega_k_sigma1_sigma2_gf(
      alps::gf::matsubara_positive_mesh(beta, n_old, stat), T.momentum_mesh(), alps::gf::index_mesh(2),
      alps::gf::index_mesh(2)));
  for (alps::gf::matsubara_index w(0); w < n_old; ++w)
    for (alps::gf::momentum_index k(0); k < 4; ++k)
      for (alps::gf::index a(0); a < 2; ++a)
        for (alps::gf::index b(0); b < 2; ++b) sigma(w, k, a, b) = sigma_value(w(), k(), a(), b());
  sigma.save(ar, "/Sigma");
}

using writer = std::function<void(alps::hdf5::archive&, alps::gf::statistics::statistics_type)>;

int run_tool(const std::string& tool, const fs::path& dir, const writer& write_sigma,
             alps::gf::statistics::statistics_type stat, const std::string& old_parameters = "",
             const std::string& new_parameters = "") {
  fs::remove_all(dir);
  fs::create_directories(dir);
  write_parameters(dir / "old.ini", n_old, old_parameters);
  write_parameters(dir / "new.ini", 32, new_parameters);
  {
    alps::hdf5::archive ar((dir / "sigma.h5").string(), "w");
    write_sigma(ar, stat);
  }
  const std::string command =
      "\"" + tool + "\" --pfile_old=old.ini --pfile_new=new.ini --sigma=sigma.h5 --g0=out.h5 > tool.log 2>&1";
  const fs::path cwd = fs::current_path();
  fs::current_path(dir);
  const int status = std::system(command.c_str());
  fs::current_path(cwd);
  return status;
}

const auto fermionic = alps::gf::statistics::FERMIONIC;
const auto bosonic = alps::gf::statistics::BOSONIC;

}  // namespace

// The same input with fermionic frequencies is accepted; only the statistics differ.
TEST(InterpolationToolsE2Edge, NormalStateRefusesBosonicSigma) {
  ASSERT_EQ(run_tool(DCA_INTERPOLATE_NORMAL, fs::absolute("e2_normal_fermionic"), write_normal_sigma, fermionic), 0);
  EXPECT_NE(run_tool(DCA_INTERPOLATE_NORMAL, fs::absolute("e2_normal_bosonic"), write_normal_sigma, bosonic), 0);
}

TEST(InterpolationToolsE2Edge, NambuRefusesBosonicSigma) {
  ASSERT_EQ(run_tool(DCA_INTERPOLATE_SC, fs::absolute("e2_nambu_fermionic"), write_nambu_sigma, fermionic), 0);
  EXPECT_NE(run_tool(DCA_INTERPOLATE_SC, fs::absolute("e2_nambu_bosonic"), write_nambu_sigma, bosonic), 0);
}

// The FLAVORS check covers the old parameter file as well as the new one.
TEST(InterpolationToolsE2Edge, NormalStateRefusesOtherFlavorsInOldFile) {
  EXPECT_NE(run_tool(DCA_INTERPOLATE_NORMAL, fs::absolute("e2_normal_flavors_old"), write_normal_sigma, fermionic,
                     "FLAVORS = 1\n"),
            0);
}

TEST(InterpolationToolsE2Edge, NambuRefusesOtherFlavors) {
  EXPECT_NE(run_tool(DCA_INTERPOLATE_SC, fs::absolute("e2_nambu_flavors_new"), write_nambu_sigma, fermionic, "",
                     "FLAVORS = 4\n"),
            0);
  EXPECT_NE(run_tool(DCA_INTERPOLATE_SC, fs::absolute("e2_nambu_flavors_old"), write_nambu_sigma, fermionic,
                     "FLAVORS = 4\n"),
            0);
}
