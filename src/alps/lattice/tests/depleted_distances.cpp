/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
*
*****************************************************************************/

#include <gtest/gtest.h>
#include <alps/lattice.h>
#include <map>
#include <string>
#include <vector>

namespace {

alps::Parameters square_lattice(const std::string& name) {
  alps::Parameters parameters;
  parameters["LATTICE_LIBRARY"] = "lattices.xml";
  parameters["LATTICE"] = name;
  parameters["L"] = 4;
  parameters["DEPLETION"] = 0.3;
  parameters["DEPLETION_SEED"] = 423756;
  return parameters;
}

// Every pair of sites with the same distance index must be related by the
// same displacement (mod L on the periodic 4x4 lattice), and the
// multiplicity of a distance index must be the number of site pairs that
// have it.
void expect_consistent_distances(const alps::graph_helper<>& lattice) {
  const std::size_t n = lattice.num_sites();
  std::vector<unsigned int> counted(lattice.num_distances(), 0);
  std::map<std::size_t, std::pair<int, int> > displacement;
  for (std::size_t x = 0; x < n; ++x)
    for (std::size_t y = 0; y < n; ++y) {
      const std::size_t d = lattice.distance(x, y);
      ASSERT_LT(d, counted.size());
      ++counted[d];
      const auto& cx = lattice.coordinate(x);
      const auto& cy = lattice.coordinate(y);
      const std::pair<int, int> delta((int(cy[0] - cx[0]) + 4) % 4, (int(cy[1] - cx[1]) + 4) % 4);
      auto found = displacement.find(d);
      if (found == displacement.end())
        displacement[d] = delta;
      else
        EXPECT_EQ(found->second, delta)
            << "sites " << x << " (" << lattice.coordinate_string(x) << ") and " << y << " ("
            << lattice.coordinate_string(y) << ") share distance index " << d;
    }
  EXPECT_EQ(lattice.distance_multiplicities(), counted);
}

} // namespace

TEST(LatticeDistances, PeriodicSquareLattice) {
  alps::graph_helper<> lattice(square_lattice("square lattice"));
  ASSERT_EQ(lattice.num_sites(), 16u);
  ASSERT_EQ(lattice.num_distances(), 16u);
  expect_consistent_distances(lattice);
}

TEST(LatticeDistances, DepletedSquareLattice) {
  alps::graph_helper<> lattice(square_lattice("depleted square lattice"));
  // The seed removes sites; the test needs a lattice that lost some.
  ASSERT_LT(lattice.num_sites(), 16u);
  ASSERT_GT(lattice.num_sites(), 1u);
  expect_consistent_distances(lattice);
}
