// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The router's rules, each against its docs/fidelity.md row, and end to end on
// small topologies.
#include "beesensor-core-tests.h"
#include "beesensor-router.h"
#include "beesensor-test-support.h"

#include <string>
#include <variant>
#include <vector>

namespace beesensor::core::test {

namespace {

// 0 - 1 - 2 - 3 - 4, node 4 the sink
const std::vector<std::pair<int, int>> kLine{{0, 1}, {1, 2}, {2, 3}, {3, 4}};

struct Solo {
    FakeClock clock;
    FakeRng rng;
    FakeTimers timers{clock};
    FakeEnergy energy;
    Router router;
    explicit Solo(bool sink = false, Config config = {})
        : router(10, sink, config, clock, rng, timers, &energy) {}
};

}  // namespace

void TestPackerLaunchesScoutThenQueues(Checker& c) {
    Config config;
    config.eventCacheSize = 2;
    Solo s(false, config);
    auto first = s.router.onEvent().decision;
    c.expect(first.action == RouteAction::Broadcast &&
                 std::holds_alternative<ForwardScout>(*first.message),
             "no path: the event leaves in a broadcast forward scout");
    const auto& scout = std::get<ForwardScout>(*first.message);
    c.expect(scout.source == 10 && scout.hops == 0 && scout.energy == kEnergyUnset,
             "the scout starts with hops 0 and the infinite energy value");
    c.expect(s.router.onEvent().decision.action == RouteAction::Queue, "the next event waits");
    c.expect(s.router.onEvent().decision.action == RouteAction::Queue, "and the next");
    c.expect(s.router.onEvent().decision.action == RouteAction::Drop,
             "beyond the event cache the event is dropped (golden rule 7)");
    s.clock.t = config.scoutTimeout + 0.01;
    c.expect(s.router.onEvent().decision.action == RouteAction::Broadcast,
             "after the scout timeout a new forward scout leaves");
}

void TestForwardScoutRebroadcastRule(Checker& c) {
    Solo s;
    s.energy.level = 0.4;
    s.rng.fixed = 0.9;  // above Pi: only the HL rule can rebroadcast
    auto near = s.router.onForwardScout(ForwardScout{1, 0, 1, kEnergyUnset}, 0);
    c.expect(near.size() == 1 && near[0].action == RouteAction::Broadcast,
             "within HL hops the scout is rebroadcast unconditionally");
    const auto& fwd = std::get<ForwardScout>(*near[0].message);
    c.expect(fwd.hops == 2, "the hop count is incremented");
    c.expect(fwd.energy == 4000, "the energy field takes the node's lower level");

    auto dup = s.router.onForwardScout(ForwardScout{1, 0, 1, kEnergyUnset}, 3);
    c.expect(dup.size() == 1 && dup[0].action == RouteAction::Drop, "a duplicate is dropped");

    auto far = s.router.onForwardScout(ForwardScout{2, 0, 5, kEnergyUnset}, 0);
    c.expect(far.size() == 1 && far[0].action == RouteAction::Drop,
             "beyond HL a draw above Pi suppresses the rebroadcast");
    s.rng.fixed = 0.1;
    auto far2 = s.router.onForwardScout(ForwardScout{3, 0, 5, kEnergyUnset}, 0);
    c.expect(far2.size() == 1 && far2[0].action == RouteAction::Broadcast,
             "beyond HL a draw below Pi rebroadcasts");
}

void TestSinkAnswersEachNeighbour(Checker& c) {
    Config config;
    config.maxPaths = 2;
    Solo s(true, config);
    auto a = s.router.onForwardScout(ForwardScout{1, 0, 2, 3000}, 5);
    c.expect(a.size() == 2 && a[0].action == RouteAction::Deliver,
             "the first copy delivers the event in its payload");
    c.expect(a[1].action == RouteAction::Unicast && a[1].nextHop == 5,
             "and a backward scout goes back to the neighbour it came from");
    const auto& b = std::get<BackwardScout>(*a[1].message);
    c.expect(b.sink == 10 && b.source == 0 && b.hops == 3 && b.energy == kEnergyUnset,
             "the backward scout names the sink, the path length, and resets the energy");
    auto again = s.router.onForwardScout(ForwardScout{1, 0, 2, 3000}, 5);
    c.expect(again.empty(), "a second copy from the same neighbour adds nothing");
    auto other = s.router.onForwardScout(ForwardScout{1, 0, 3, 3000}, 6);
    c.expect(other.size() == 1 && other[0].action == RouteAction::Unicast && other[0].nextHop == 6,
             "a copy from another neighbour opens another path, without a second delivery");
    c.expect(std::get<BackwardScout>(*other[0].message).pathId != b.pathId, "with its own path ID");
    auto third = s.router.onForwardScout(ForwardScout{1, 0, 3, 3000}, 7);
    c.expect(third.empty(), "at most maxPaths paths per generation");
}

void TestBackwardScoutDisjointAndLoopFree(Checker& c) {
    Solo s;
    s.router.onForwardScout(ForwardScout{1, 0, 0, kEnergyUnset}, 0);  // from the source, node 0
    auto first = s.router.onBackwardScout(BackwardScout{1, 0, 9, 11, 3, kEnergyUnset}, 8);
    c.expect(first.action == RouteAction::Unicast && first.nextHop == 0,
             "the backward scout goes to the previous hop the forward scout came from");
    c.expect(s.router.forwardingEntries() == 1, "and leaves a forwarding entry");
    auto second = s.router.onBackwardScout(BackwardScout{1, 0, 9, 12, 3, kEnergyUnset}, 7);
    c.expect(second.action == RouteAction::Drop,
             "a second path of the same generation is refused: paths are node-disjoint");
    auto loop = s.router.onBackwardScout(BackwardScout{1, 0, 9, 11, 3, kEnergyUnset}, 8);
    c.expect(loop.action == RouteAction::Drop && s.router.forwardingEntries() == 0,
             "a backward scout visiting twice is dropped and its entry flushed");
    auto stray = s.router.onBackwardScout(BackwardScout{99, 0, 9, 13, 3, kEnergyUnset}, 8);
    c.expect(stray.action == RouteAction::Drop, "no forward scout passed: dropped");
}

void TestLineEndToEnd(Checker& c) {
    Net net(5, kLine, {4});
    auto d = net.event(0);
    c.expect(d.action == RouteAction::Broadcast, "the source scouts");
    net.run();
    c.expect(net.delivered[4] == 1, "the scout's event reaches the sink");
    const auto& paths = net.at(0).paths();
    c.expect(paths.size() == 1, "the backward scout builds one path on a line");
    if (paths.empty()) return;
    c.expect(paths[0].nextHop == 1 && paths[0].hops == 4 && paths[0].sink == 4,
             "via neighbour 1, four hops, to sink 4");
    c.expect(!net.at(0).searching(), "the search is over");

    for (int i = 0; i < 3; ++i)
        c.expect(net.event(0).action == RouteAction::Unicast, "foragers go");
    net.run();
    c.expect(net.delivered[4] == 4, "three foragers delivered");
    c.expect(net.at(0).paths()[0].outstanding == 3, "three foragers out");
    net.advance(0.6);  // past sinkSwarmWait
    c.expect(net.transmissions[Kind::Swarm] == 4, "one swarm, four hops back");
    c.expect(net.at(0).paths().size() == 1 && net.at(0).paths()[0].outstanding == 0,
             "the swarm returns all three foragers and the path lives");
}

void TestEventsQueuedDuringDiscoveryReleased(Checker& c) {
    Net net(5, kLine, {4});
    net.event(0);
    c.expect(net.event(0).action == RouteAction::Queue, "a second event waits for the path");
    net.run();
    c.expect(net.routeReady[0] == 1, "the path's arrival releases the waiting events");
    c.expect(net.at(0).queuedEvents() == 0, "and empties the cache");
}

void TestPathLostWithoutSwarm(Checker& c) {
    Net net(5, kLine, {4});
    net.event(0);
    net.run();
    net.event(0);
    // the forager is lost: the queue is not run, so nothing returns
    net.timers.advance(1.2);  // past FORG (1 s)
    c.expect(net.at(0).paths().empty(), "unreturned foragers past FORG remove the path");
}

void TestIdlePathExpires(Checker& c) {
    Net net(5, kLine, {4});
    net.event(0);
    net.run();
    c.expect(net.at(0).paths().size() == 1, "a path");
    net.advance(10.5);
    c.expect(net.at(0).paths().empty(), "no forager for the forager TTL: the path expires");
}

void TestDisjointPathsOnDiamond(Checker& c) {
    // 0 - {1, 2} - 3, sink 3: two node-disjoint paths
    Net net(4, {{0, 1}, {0, 2}, {1, 3}, {2, 3}}, {3});
    net.event(0);
    net.run();
    const auto& paths = net.at(0).paths();
    c.expect(paths.size() == 2, "two disjoint paths on a diamond");
    if (paths.size() == 2) {
        c.expect(paths[0].nextHop != paths[1].nextHop, "through different neighbours");
    }
}

void TestStochasticChoiceByReward(Checker& c) {
    Net net(4, {{0, 1}, {0, 2}, {1, 3}, {2, 3}}, {3});
    net.energies[1]->level = 0.9;
    net.energies[2]->level = 0.3;
    net.event(0);
    net.run();
    auto& paths = net.at(0).paths();
    c.expect(paths.size() == 2, "two paths");
    if (paths.size() != 2) return;
    const Path& rich = paths[0].nextHop == 1 ? paths[0] : paths[1];
    const Path& poor = paths[0].nextHop == 1 ? paths[1] : paths[0];
    c.expect(rich.energy == 9000 && poor.energy == 3000, "each path carries its least energy");
    c.expect(reward(rich) > reward(poor), "the richer path has the higher reward");
    int viaRich = 0;
    for (int i = 0; i < 400; ++i) {
        auto d = net.event(0);
        if (d.nextHop == 1) ++viaRich;
        net.run();
        net.advance(net.clock.t + 0.6);
    }
    // p(rich) = 0.9 / (0.9 + 0.3) = 0.75
    c.expect(viaRich > 260 && viaRich < 340,
             "events split by reward (" + std::to_string(viaRich) + " of 400 on the richer path)");
}

void TestEnergyFloorDropsPath(Checker& c) {
    Net net(4, {{0, 1}, {0, 2}, {1, 3}, {2, 3}}, {3});
    net.energies[1]->level = 0.9;
    net.energies[2]->level = 0.5;
    net.event(0);
    net.run();
    c.expect(net.at(0).paths().size() == 2, "two paths");
    net.energies[2]->level = 0.05;  // below pL = 0.1
    for (int i = 0; i < 20; ++i)
        net.event(0);
    net.run();
    net.advance(0.6);
    const auto& paths = net.at(0).paths();
    c.expect(paths.size() == 1 && paths[0].nextHop == 1,
             "a path below pL is dropped when a better one exists");
}

void TestTablesAreBounded(Checker& c) {
    Config config;
    config.scoutCacheSize = 4;
    Solo s(false, config);
    for (std::uint32_t i = 0; i < 20; ++i)
        s.router.onForwardScout(ForwardScout{i, 0, 0, 0}, 0);
    c.expect(s.router.scoutCacheEntries() <= 4, "the scout cache keeps its cap (golden rule 7)");
}

void TestDeterminism(Checker& c) {
    auto runOnce = [] {
        // a 3 x 3 grid, sink in the corner, sources elsewhere
        std::vector<std::pair<int, int>> grid;
        for (int r = 0; r < 3; ++r) {
            for (int col = 0; col < 3; ++col) {
                const int n = r * 3 + col;
                if (col < 2) grid.emplace_back(n, n + 1);
                if (r < 2) grid.emplace_back(n, n + 3);
            }
        }
        Net net(9, grid, {8}, {}, 42);
        for (int i = 0; i < 9; ++i)
            net.energies[static_cast<std::size_t>(i)]->level = 1.0 - 0.05 * i;
        for (int t = 0; t < 50; ++t) {
            net.event(t % 4);
            net.run();
            net.advance(net.clock.t + 0.3);
        }
        std::vector<int> out{net.delivered[8]};
        for (auto [k, v] : net.transmissions)
            out.push_back(v);
        return out;
    };
    const auto a = runOnce();
    const auto b = runOnce();
    c.expect(a == b, "a fixed seed gives the same run (golden rule 5)");
    c.expect(!a.empty() && a[0] >= 45, "and the grid delivers nearly every event");
}

namespace {

// Gives `s` (a relay, node 10) the forwarding entry of path `pathId` towards
// sink 9, from a generation `gen` that source 0 scouted.
void relayPath(Solo& s, std::uint32_t gen, std::uint32_t pathId) {
    s.router.onForwardScout(ForwardScout{gen, 0, 0, kEnergyUnset}, 0);
    s.router.onBackwardScout(BackwardScout{gen, 0, 9, pathId, 3, kEnergyUnset}, 8);
}

}  // namespace

void TestForwardingExpiryIgnoresRefreshOrder(Checker& c) {
    Solo s;
    relayPath(s, 1, 11);  // A at t = 0
    s.clock.t = 1;
    relayPath(s, 2, 12);  // B at t = 1
    s.clock.t = 2;
    DataHeader a{0, 9, 11, 1, kEnergyUnset};
    s.router.onData(a, 0);  // A refreshed at t = 2
    s.clock.t = 31.5;       // B (t = 1) is past the 30 s lifetime, A (t = 2) is not
    DataHeader b{0, 9, 12, 1, kEnergyUnset};
    c.expect(s.router.onData(b, 0).action == RouteAction::Drop,
             "an expired entry behind a refreshed one is expired too");
    DataHeader a2{0, 9, 11, 1, kEnergyUnset};
    c.expect(s.router.onData(a2, 0).action == RouteAction::Unicast, "the refreshed entry lives");
}

void TestForwardingCapHoldsAfterLoopFlush(Checker& c) {
    Config config;
    config.forwardingTableSize = 2;
    Solo s(false, config);
    relayPath(s, 1, 11);                                                       // A
    relayPath(s, 2, 12);                                                       // B
    s.router.onBackwardScout(BackwardScout{2, 0, 9, 12, 3, kEnergyUnset}, 8);  // B flushed (loop)
    relayPath(s, 3, 13);                                                       // C
    relayPath(s, 4, 14);                                                       // D evicts A
    relayPath(s, 5, 15);  // E must evict a live entry, not B's stale key
    c.expect(s.router.forwardingEntries() <= 2,
             "the forwarding table keeps its cap after a loop flush (" +
                 std::to_string(s.router.forwardingEntries()) + " entries)");
}

void TestScoutRetriedWhileEventsWait(Checker& c) {
    Solo s;
    std::vector<Decision> sent;
    s.router.setControlSink([&](const Decision& d) { sent.push_back(d); });
    const auto first = s.router.onEvent().decision;
    c.expect(first.action == RouteAction::Broadcast, "the first event scouts");
    c.expect(s.router.onEvent().decision.action == RouteAction::Queue, "the second waits");
    s.timers.advance(Config{}.scoutTimeout + 0.01);
    c.expect(sent.size() == 1 && sent[0].action == RouteAction::Broadcast &&
                 std::holds_alternative<ForwardScout>(*sent[0].message),
             "with events waiting and no path, a new forward scout leaves after the timeout");
    if (!sent.empty() && sent[0].message) {
        c.expect(std::get<ForwardScout>(*sent[0].message).scoutId !=
                     std::get<ForwardScout>(*first.message).scoutId,
                 "as a new generation");
    }
    c.expect(s.router.queuedEvents() == 1, "the retry adds no event to the cache");
}

void TestNoScoutRetryWithoutWaitingEvents(Checker& c) {
    Solo s;
    std::vector<Decision> sent;
    s.router.setControlSink([&](const Decision& d) { sent.push_back(d); });
    s.router.onEvent();  // the scout carries the only event; nothing waits
    s.timers.advance(5.0);
    c.expect(sent.empty(), "no event waits: no retry");
}

void TestSinkSwarmGroupsBounded(Checker& c) {
    Config config;
    config.forwardingTableSize = 2;
    Solo s(true, config);
    for (std::uint32_t g = 1; g <= 20; ++g) {
        auto out = s.router.onForwardScout(ForwardScout{g, 0, 2, kEnergyUnset}, 5);
        if (out.size() < 2 || !out[1].message) continue;
        const auto& b = std::get<BackwardScout>(*out[1].message);
        DataHeader h{0, b.sink, b.pathId, 2, kEnergyUnset};
        s.router.onData(h, 5);
    }
    c.expect(s.router.waitingGroups() <= 2,
             "pending swarm groups follow the forwarding table's cap (" +
                 std::to_string(s.router.waitingGroups()) + ")");
    c.expect(s.timers.pending() <= 2, "and their swarm timers are cancelled with them (" +
                                          std::to_string(s.timers.pending()) + " pending)");
}

void TestForgMeasuredFromLastReturn(Checker& c) {
    Net net(5, kLine, {4});
    net.event(0);
    net.run();
    net.event(0);  // forager 1: lost (the queue is cleared, never delivered)
    net.dropQueued();
    net.event(0);  // forager 2: arrives
    net.run();
    net.advance(0.6);  // forager 2 returns in a swarm; forager 1 stays out
    net.advance(1.3);  // past FORG from forager 1's send, within FORG of the return
    c.expect(net.at(0).paths().size() == 1,
             "a path that keeps returning foragers survives one lost forager");
    net.advance(1.7);  // past FORG from the last return, forager 1 still out
    c.expect(net.at(0).paths().empty(),
             "with no return for FORG while foragers are out, the path is removed");
}

}  // namespace beesensor::core::test
