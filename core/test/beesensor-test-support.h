// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// Test doubles for swarm-core's ports, and a tiny synchronous network that
// carries BeeSensor's decisions between routers (no simulator).
#ifndef BEESENSOR_TEST_SUPPORT_H
#define BEESENSOR_TEST_SUPPORT_H

#include "beesensor-router.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace beesensor::core::test {

class FakeClock : public swarm::core::IClock {
public:
    Time now() const override { return t; }
    Time t = 0;
};

/// A seeded linear congruential generator, or a fixed value when `fixed` is set.
class FakeRng : public swarm::core::IRng {
public:
    explicit FakeRng(std::uint64_t seed = 1) : state_(seed) {}
    double uniform() override {
        if (fixed >= 0) return fixed;
        state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<double>(state_ >> 11) / static_cast<double>(1ULL << 53);
    }
    int uniformInt(int n) override { return n <= 0 ? 0 : static_cast<int>(uniform() * n) % n; }
    double fixed = -1;

private:
    std::uint64_t state_;
};

/// Timers run when the test advances the clock past their deadline.
class FakeTimers : public swarm::core::ITimerScheduler {
public:
    explicit FakeTimers(FakeClock& clock) : clock_(clock) {}
    void schedule(Time delay, std::function<void()> callback) override {
        pending_.emplace(clock_.t + delay, Entry{swarm::core::kNoTimer, std::move(callback)});
    }
    swarm::core::TimerHandle scheduleCancellable(Time delay,
                                                 std::function<void()> callback) override {
        const swarm::core::TimerHandle h = ++lastHandle_;
        pending_.emplace(clock_.t + delay, Entry{h, std::move(callback)});
        return h;
    }
    void cancel(swarm::core::TimerHandle timer) override {
        if (timer == swarm::core::kNoTimer) return;
        std::erase_if(pending_, [&](const auto& kv) { return kv.second.handle == timer; });
    }
    /// Timers scheduled and not yet run or cancelled.
    std::size_t pending() const { return pending_.size(); }
    /// Runs every timer due by `until`, in deadline order, moving the clock.
    void advance(Time until) {
        while (!pending_.empty() && pending_.begin()->first <= until) {
            auto node = pending_.extract(pending_.begin());
            clock_.t = node.key();
            node.mapped().callback();
        }
        clock_.t = until;
    }

private:
    struct Entry {
        swarm::core::TimerHandle handle = swarm::core::kNoTimer;
        std::function<void()> callback;
    };
    FakeClock& clock_;
    std::multimap<Time, Entry> pending_;
    swarm::core::TimerHandle lastHandle_ = swarm::core::kNoTimer;
};

class FakeEnergy : public swarm::core::IEnergy {
public:
    double residualEnergyFraction() const override { return level; }
    double level = std::numeric_limits<double>::quiet_NaN();
};

/// Routers on a fixed undirected topology. Messages go through a FIFO queue
/// and are delivered by run(), so the order is deterministic.
class Net {
public:
    Net(int nodes, const std::vector<std::pair<int, int>>& links, const std::vector<int>& sinks,
        const Config& config = {}, std::uint64_t seed = 1)
        : timers(clock), adjacency_(static_cast<std::size_t>(nodes)) {
        for (auto [a, b] : links) {
            adjacency_[static_cast<std::size_t>(a)].push_back(b);
            adjacency_[static_cast<std::size_t>(b)].push_back(a);
        }
        for (int i = 0; i < nodes; ++i) {
            rngs.push_back(std::make_unique<FakeRng>(seed + static_cast<std::uint64_t>(i)));
            energies.push_back(std::make_unique<FakeEnergy>());
            const bool sink = std::find(sinks.begin(), sinks.end(), i) != sinks.end();
            routers.push_back(std::make_unique<Router>(i, sink, config, clock, *rngs.back(), timers,
                                                       energies.back().get()));
            routers.back()->setControlSink([this, i](const Decision& d) { carry(i, d); });
            routers.back()->setRouteReady([this, i] { ++routeReady[i]; });
        }
    }

    /// One event at `node`: what the packer decided.
    Decision event(int node) {
        Decision d = routers[static_cast<std::size_t>(node)]->onEvent().decision;
        carry(node, d);
        return d;
    }

    /// Delivers queued messages until none is left (or `limit` deliveries).
    void run(int limit = 100000) {
        while (!queue_.empty() && limit-- > 0) {
            auto [to, from, m] = queue_.front();
            queue_.pop_front();
            Router& r = *routers[static_cast<std::size_t>(to)];
            if (auto* fs = std::get_if<ForwardScout>(&m)) {
                for (const Decision& d : r.onForwardScout(*fs, from)) {
                    if (d.action == RouteAction::Deliver) ++delivered[to];
                    carry(to, d);
                }
            } else if (auto* bs = std::get_if<BackwardScout>(&m)) {
                carry(to, r.onBackwardScout(*bs, from));
            } else if (auto* sw = std::get_if<Swarm>(&m)) {
                carry(to, r.onSwarm(*sw, from));
            } else if (auto* h = std::get_if<DataHeader>(&m)) {
                Decision d = r.onData(*h, from);
                if (d.action == RouteAction::Deliver) ++delivered[to];
                carry(to, d);
            }
        }
    }

    /// Loses every message in flight (a lossy link).
    void dropQueued() { queue_.clear(); }

    /// Advances the clock, firing timers, and runs the network.
    void advance(Time until) {
        timers.advance(until);
        run();
    }

    Router& at(int node) { return *routers[static_cast<std::size_t>(node)]; }

    FakeClock clock;
    FakeTimers timers;
    std::vector<std::unique_ptr<FakeRng>> rngs;
    std::vector<std::unique_ptr<FakeEnergy>> energies;
    std::vector<std::unique_ptr<Router>> routers;
    std::map<int, int> delivered;   ///< events (scout payloads and foragers) per sink
    std::map<int, int> routeReady;  ///< routeReady callbacks per source
    std::map<Kind, int> transmissions;

private:
    void carry(int from, const Decision& d) {
        if (!d.message) return;
        // every message goes through the wire, as an adapter's would
        auto decoded = decode(encode(*d.message));
        if (!decoded) return;
        const Kind kind = std::visit(
            [](const auto& m) {
                using T = std::decay_t<decltype(m)>;
                if constexpr (std::is_same_v<T, ForwardScout>)
                    return Kind::ForwardScout;
                else if constexpr (std::is_same_v<T, BackwardScout>)
                    return Kind::BackwardScout;
                else if constexpr (std::is_same_v<T, Swarm>)
                    return Kind::Swarm;
                else
                    return Kind::Data;
            },
            *decoded);
        if (d.action == RouteAction::Broadcast) {
            ++transmissions[kind];
            for (int n : adjacency_[static_cast<std::size_t>(from)])
                queue_.emplace_back(n, from, *decoded);
        } else if (d.action == RouteAction::Unicast) {
            ++transmissions[kind];
            queue_.emplace_back(d.nextHop, from, *decoded);
        }
    }

    std::vector<std::vector<int>> adjacency_;
    std::deque<std::tuple<int, int, Message>> queue_;
};

}  // namespace beesensor::core::test

#endif  // BEESENSOR_TEST_SUPPORT_H
