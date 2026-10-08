#pragma once
#include "problem.hpp"
#include "solver.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

// Wrapper: adapts the plain routine in jacobi_ed.cpp to the harness. Only this
// file knows about Solver and MatrixProblem.

// Defined in jacobi_ed.cpp.
std::vector<double> jacobi_eigenvalues(std::vector<double> a, int n);

namespace onboard {

class Algorithm final : public Solver {
public:
    explicit Algorithm(const MatrixProblem& p) : p_(p) {}

    std::optional<Estimate> groundStateEnergy() const override {
        std::vector<double> levels =
            jacobi_eigenvalues(p_.singleParticleMatrix(), p_.numSites());
        std::sort(levels.begin(), levels.end());

        double e = 0.0;
        for (int k = 0; k < p_.numParticles(); ++k) e += levels[k];
        if (!std::isfinite(e)) return std::nullopt;
        return Estimate{e};
    }

    const char* name() const override { return "JacobiED"; }

private:
    const MatrixProblem& p_;
};

// Factory the harness calls.
inline std::unique_ptr<const Solver> makeAlgorithm(const MatrixProblem& p) {
    return std::make_unique<Algorithm>(p);
}

} // namespace onboard
