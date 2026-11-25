// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The list every runner walks.
#include "beesensor-core-tests.h"

#include <array>

namespace beesensor::core::test {

namespace {

constexpr std::array<NamedTest, 25> kTests{{
    {"skeleton: version() is the package's", TestVersionIsThePackages},
    {"skeleton: swarm-core's energy port is reachable", TestSwarmCoreReachable},
    {"wire: roundtrip, sizes, little-endian", TestWireRoundtrip},
    {"wire: the decoder fails closed", TestWireFailsClosed},
    {"wire: seeded fuzz", TestWireFuzz},
    {"wire: energy units", TestEnergyUnits},
    {"packer: a forward scout, then the event cache, then drop", TestPackerLaunchesScoutThenQueues},
    {"forward scout: within HL always, beyond with Pi; duplicates dropped",
     TestForwardScoutRebroadcastRule},
    {"sink: delivers once, one backward scout per neighbour", TestSinkAnswersEachNeighbour},
    {"backward scout: node-disjoint and loop-free", TestBackwardScoutDisjointAndLoopFree},
    {"line: discovery, foragers, swarm", TestLineEndToEnd},
    {"packer: events waiting for discovery are released", TestEventsQueuedDuringDiscoveryReleased},
    {"maintenance: unreturned foragers past FORG remove the path", TestPathLostWithoutSwarm},
    {"maintenance: an idle path expires after the forager TTL", TestIdlePathExpires},
    {"diamond: two node-disjoint paths", TestDisjointPathsOnDiamond},
    {"choice: stochastic by reward", TestStochasticChoiceByReward},
    {"swarm: a path below pL is dropped when a better one exists", TestEnergyFloorDropsPath},
    {"bounds: the scout cache keeps its cap", TestTablesAreBounded},
    {"determinism: a fixed seed repeats", TestDeterminism},
    {"forwarding: expiry is not blocked by a refreshed entry",
     TestForwardingExpiryIgnoresRefreshOrder},
    {"forwarding: the cap holds after a loop flush", TestForwardingCapHoldsAfterLoopFlush},
    {"scouting: retried on a timer while events wait", TestScoutRetriedWhileEventsWait},
    {"scouting: no retry when no event waits", TestNoScoutRetryWithoutWaitingEvents},
    {"sink: pending swarm groups and their timers are bounded", TestSinkSwarmGroupsBounded},
    {"maintenance: FORG runs from the last return", TestForgMeasuredFromLastReturn},
}};

}  // namespace

std::span<const NamedTest> AllTests() {
    return kTests;
}

}  // namespace beesensor::core::test
