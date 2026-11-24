// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The skeleton's tests (#41): the library links, reports the package version
// it was built as, and reaches swarm-core's ports. No protocol behaviour yet.
#include "beesensor-core-tests.h"
#include "beesensor-core.h"
#include "swarm-ports.h"

#include <array>
#include <cmath>
#include <limits>
#include <string_view>

namespace beesensor::core::test {

void TestVersionIsThePackages(Checker& c) {
    c.expect(version() == std::string_view{BEESENSOR_CORE_VERSION_STRING},
             "version() is the package version the build was configured with");
    c.expect(!version().empty(), "version() is not empty");
}

namespace {

// swarm-core's energy port, as an adapter with no energy model provides it.
class NoEnergyModel final : public swarm::core::IEnergy {
public:
    double residualEnergyFraction() const override {
        return std::numeric_limits<double>::quiet_NaN();
    }
};

}  // namespace

void TestSwarmCoreReachable(Checker& c) {
    const NoEnergyModel energy;
    const swarm::core::IEnergy& port = energy;
    c.expect(std::isnan(port.residualEnergyFraction()),
             "swarm-core's IEnergy port is reachable from core/");
}

namespace {

constexpr std::array<NamedTest, 2> kTests{{
    {"skeleton: version() is the package's", TestVersionIsThePackages},
    {"skeleton: swarm-core's energy port is reachable", TestSwarmCoreReachable},
}};

}  // namespace

std::span<const NamedTest> AllTests() {
    return kTests;
}

}  // namespace beesensor::core::test
