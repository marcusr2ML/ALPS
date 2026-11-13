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
#include <alps/parser/xslt_path.h>
#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

namespace {

struct ShippedLibrary : alps::LatticeLibrary {
  explicit ShippedLibrary(std::istream& in) : alps::LatticeLibrary(in) {}
  const alps::LatticeMap& lattices() const { return lattices_; }
};

const ShippedLibrary& shipped_library() {
  static std::ifstream in(alps::search_xml_library_path("lattices.xml"));
  static const ShippedLibrary library(in);
  return library;
}

using Vectors = std::vector<std::vector<double> >;

template <class It>
Vectors evaluate(std::pair<It, It> range) {
  Vectors result;
  for (It v = range.first; v != range.second; ++v) {
    std::vector<double> x;
    for (const auto& c : *v)
      x.push_back(alps::evaluate<double>(c));
    result.push_back(x);
  }
  return result;
}

// Primitive vectors a_i and reciprocal vectors b_j must satisfy
// a_i . b_j = 2 pi delta_ij.
void expect_dual(const std::string& name, const alps::Parameters& parameters) {
  SCOPED_TRACE(name);
  auto it = shipped_library().lattices().find(name);
  ASSERT_NE(it, shipped_library().lattices().end());
  alps::LatticeDescriptor lattice(it->second);
  lattice.set_parameters(parameters);
  const Vectors a = evaluate(lattice.basis_vectors());
  const Vectors b = evaluate(lattice.reciprocal_basis_vectors());
  const std::size_t d = lattice.dimension();
  ASSERT_EQ(a.size(), d);
  ASSERT_EQ(b.size(), d);
  const double two_pi = 2. * boost::math::constants::pi<double>();
  for (std::size_t i = 0; i < d; ++i) {
    ASSERT_EQ(a[i].size(), d);
    ASSERT_EQ(b[i].size(), d);
  }
  for (std::size_t i = 0; i < d; ++i)
    for (std::size_t j = 0; j < d; ++j) {
      double dot = 0.;
      for (std::size_t k = 0; k < d; ++k)
        dot += a[i][k] * b[j][k];
      const double expected = i == j ? two_pi : 0.;
      EXPECT_NEAR(dot, expected, 1e-12 * two_pi)
          << "a_" << i + 1 << " . b_" << j + 1 << " = " << dot << ", expected " << expected
          << " (abs error " << std::abs(dot - expected) << ")";
    }
}

} // namespace

TEST(LatticeReciprocalBasis, EveryShippedLatticeAtDefaultParameters) {
  for (const auto& entry : shipped_library().lattices())
    if (entry.second.dimension() > 0)
      expect_dual(entry.first, alps::Parameters());
}

TEST(LatticeReciprocalBasis, CenteredRectangularLattice) {
  alps::Parameters parameters;
  parameters["a"] = 1.;
  parameters["b"] = 2.;
  expect_dual("centered rectangular lattice", parameters);
}

TEST(LatticeReciprocalBasis, ObliqueLattice) {
  alps::Parameters parameters;
  parameters["a"] = 1.;
  parameters["b"] = 1.5;
  parameters["phi"] = "Pi/3";
  expect_dual("oblique lattice", parameters);
}

// With the default phi = Pi/2 the oblique lattice is the rectangular lattice.
TEST(LatticeReciprocalBasis, ObliqueLatticeDefaultIsRectangular) {
  auto it = shipped_library().lattices().find("oblique lattice");
  ASSERT_NE(it, shipped_library().lattices().end());
  alps::LatticeDescriptor lattice(it->second);
  alps::Parameters parameters;
  parameters["b"] = 2.;
  lattice.set_parameters(parameters);
  const Vectors a = evaluate(lattice.basis_vectors());
  ASSERT_EQ(a.size(), 2u);
  EXPECT_NEAR(a[0][0], 1., 1e-15);
  EXPECT_NEAR(a[0][1], 0., 1e-15);
  EXPECT_NEAR(a[1][0], 0., 1e-15);
  EXPECT_NEAR(a[1][1], 2., 1e-15);
}
