// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// BeeSensor's core test functions. Each records its failures in the Checker;
// the runners (run-core-tests.cc, and the ns-3 suite once ns3/ exists) report
// them.
#ifndef BEESENSOR_CORE_TESTS_H
#define BEESENSOR_CORE_TESTS_H

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace beesensor::core::test {

/// Collects a test's failures; a test passes when it records none.
class Checker {
public:
    void expect(bool ok, std::string what) {
        if (!ok) failures_.push_back(std::move(what));
    }
    const std::vector<std::string>& failures() const { return failures_; }

private:
    std::vector<std::string> failures_;
};

void TestVersionIsThePackages(Checker& c);
void TestSwarmCoreReachable(Checker& c);

struct NamedTest {
    const char* name;
    void (*run)(Checker&);
};

/// Every core test, in a fixed order.
std::span<const NamedTest> AllTests();

}  // namespace beesensor::core::test

#endif  // BEESENSOR_CORE_TESTS_H
