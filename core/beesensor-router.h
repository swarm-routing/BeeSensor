// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// BeeSensor's per-node state and decisions, v0 (docs/fidelity.md, built from
// public secondary sources; swarm-routing/.github#881). Ports in, decisions
// out: no simulator header (golden rule 11). The adapter hands every received
// BeeSensor packet to the matching on*() call and carries out what comes back.
//
// The agents, as the sheet's Mechanisms rows name them:
//   packer        onEvent(): an event finds a forager (a path) or waits;
//   forward scout broadcast from a source with no path, rebroadcast within HL
//                 hops, beyond with probability Pi; the first copy to reach a
//                 sink delivers its event;
//   backward scout the sink's answer, one per neighbour the scout arrived
//                 from (up to maxPaths), building node-disjoint paths back;
//   forager       a DataHeader on point-to-point forwarding entries;
//   swarm         the foragers that reached the sink, returned together after
//                 sinkSwarmWait; at the source they keep their path alive.
#ifndef BEESENSOR_ROUTER_H
#define BEESENSOR_ROUTER_H

#include "beesensor-config.h"
#include "beesensor-wire.h"
#include "swarm-observer.h"
#include "swarm-ports.h"
#include "swarm-route-decision.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace beesensor::core {

using swarm::core::RouteAction;
using swarm::core::Time;
using Decision = swarm::core::RouteDecision<Message>;

/// What onEvent() decided for one event at the source.
struct EventDecision {
    /// Unicast: send the event behind `message` (a DataHeader) to `nextHop`.
    /// Broadcast: send the event in the payload of `message` (a ForwardScout).
    /// Queue: hold the event; routeReady fires when a path exists.
    /// Drop: the event cache is full.
    Decision decision;
};

/// A path as the source keeps it (the routing table).
struct Path {
    NodeAddress sink = swarm::core::kInvalidAddress;
    std::uint32_t pathId = 0;
    NodeAddress nextHop = swarm::core::kInvalidAddress;
    std::uint8_t hops = 0;
    std::uint16_t energy = kEnergyUnset;
    std::uint32_t outstanding = 0;  ///< foragers sent, not yet back
    Time oldestOutstanding = 0;     ///< when the oldest of them left
    Time lastActive = 0;            ///< last forager sent or returned
};

/// The path reward (fidelity.md, Parameters: path reward): least residual
/// energy over hop count; an unset energy reads as 1.
double reward(const Path& p);

class Router {
public:
    /// Called with a control message the router sends on its own (a swarm
    /// leaving the sink).
    using ControlSink = std::function<void(const Decision&)>;
    /// Called at the source when a path appears while events wait.
    using RouteReady = std::function<void()>;

    Router(NodeAddress self, bool isSink, const Config& config, swarm::core::IClock& clock,
           swarm::core::IRng& rng, swarm::core::ITimerScheduler& timers,
           const swarm::core::IEnergy* energy = nullptr);
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    void setControlSink(ControlSink sink) { controlSink_ = std::move(sink); }
    void setRouteReady(RouteReady ready) { routeReady_ = std::move(ready); }
    /// An optional observer (not owned); nullptr removes it.
    void setObserver(swarm::core::IRouterObserver* observer) { observer_ = observer; }

    /// The packer: one event generated at this node.
    EventDecision onEvent();
    /// A forward scout from neighbour `from`. A sink returns Deliver (for the
    /// first copy of a generation) and/or Unicast backward scouts.
    std::vector<Decision> onForwardScout(const ForwardScout& scout, NodeAddress from);
    Decision onBackwardScout(const BackwardScout& scout, NodeAddress from);
    /// A forager from `from`; `header` is updated in place (hops, energy).
    Decision onData(DataHeader& header, NodeAddress from);
    Decision onSwarm(const Swarm& swarm, NodeAddress from);

    NodeAddress self() const { return self_; }
    bool isSink() const { return isSink_; }
    /// The source's live paths, after expiring the stale ones.
    const std::vector<Path>& paths();
    std::size_t forwardingEntries() const { return forwarding_.size(); }
    std::size_t scoutCacheEntries() const { return scoutCache_.size(); }
    std::size_t queuedEvents() const { return queuedEvents_; }
    bool searching() const { return searching_; }

private:
    struct ScoutEntry {
        NodeAddress prevHop = swarm::core::kInvalidAddress;
        Time at = 0;
        bool answered = false;  ///< a backward scout of this generation passed
    };
    struct SinkGeneration {
        std::vector<NodeAddress> neighbours;  ///< one backward scout each
    };
    struct Forwarding {
        NodeAddress nextHop = swarm::core::kInvalidAddress;  ///< towards the sink
        NodeAddress prevHop = swarm::core::kInvalidAddress;  ///< towards the source
        Time at = 0;
    };
    struct Waiting {
        NodeAddress source = swarm::core::kInvalidAddress;
        std::uint16_t foragers = 0;
        std::uint8_t hops = 0;
        std::uint16_t energy = kEnergyUnset;
    };
    using GenKey = std::pair<NodeAddress, std::uint32_t>;   // (source, scoutId)
    using PathKey = std::pair<NodeAddress, std::uint32_t>;  // (sink, pathId)

    Time now() const { return clock_.now(); }
    std::uint16_t ownEnergy() const;
    void expire();
    void expirePaths();
    void addPath(const Path& p);
    Path* choosePath();
    ForwardScout newScout();
    void rememberScout(const GenKey& key, NodeAddress from);
    void addForwarding(const PathKey& key, const Forwarding& f);
    void sendSwarm(PathKey key);
    void sent(Kind kind, bool broadcast);
    void received(Kind kind);

    NodeAddress self_;
    bool isSink_;
    Config config_;
    swarm::core::IClock& clock_;
    swarm::core::IRng& rng_;
    swarm::core::ITimerScheduler& timers_;
    const swarm::core::IEnergy* energy_;
    swarm::core::IRouterObserver* observer_ = nullptr;
    ControlSink controlSink_;
    RouteReady routeReady_;

    // source
    std::vector<Path> paths_;  ///< at most config_.maxPaths
    std::uint32_t nextScoutId_ = 1;
    bool searching_ = false;
    Time searchStarted_ = 0;
    std::size_t queuedEvents_ = 0;
    // every node
    std::map<GenKey, ScoutEntry> scoutCache_;
    std::deque<GenKey> scoutOrder_;
    std::map<PathKey, Forwarding> forwarding_;
    std::deque<PathKey> forwardingOrder_;
    // sink
    std::uint32_t nextPathId_ = 1;
    std::map<GenKey, SinkGeneration> sinkGenerations_;
    std::deque<GenKey> sinkGenerationOrder_;
    std::map<PathKey, Waiting> waiting_;
};

/// The observer's names for the kind bytes.
const char* kindName(std::uint8_t kind);

}  // namespace beesensor::core

#endif  // BEESENSOR_ROUTER_H
