// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// BeeSensor's protocol constants, v0. Each default cites its row of
// docs/fidelity.md ("Parameters"); every one is a repo choice, since the open
// sources name these quantities but give no values (the papers are closed and
// unread: the sheet's v0, swarm-routing/.github#881). Times are in seconds.
#ifndef BEESENSOR_CONFIG_H
#define BEESENSOR_CONFIG_H

#include <cstddef>
#include <cstdint>

namespace beesensor::core {

struct Config {
    /// HL: a node at most this many hops from the source rebroadcasts a
    /// forward scout unconditionally (fidelity.md, Parameters: HL).
    std::uint8_t unconditionalHops = 2;
    /// Pi: rebroadcast probability beyond HL (fidelity.md, Parameters: Pi).
    double rebroadcastProbability = 0.5;
    /// pL: a path whose least residual energy falls below this is dropped when
    /// a better path exists (fidelity.md, Parameters: pL).
    double energyFloor = 0.1;
    /// FORG: how long sent foragers may stay unreturned before their path is
    /// removed (fidelity.md, Parameters: FORG).
    double foragerReturnWait = 1.0;
    /// How long foragers wait at the sink before travelling back as one swarm
    /// (fidelity.md, Parameters: forager wait at the sink).
    double sinkSwarmWait = 0.5;
    /// Forager TTL: a path with no forager sent or returned for this long
    /// expires (fidelity.md, Parameters: forager TTL).
    double foragerTtl = 10.0;
    /// Forwarding-entry lifetime at the sink and intermediate nodes, refreshed
    /// by every forager and swarm (fidelity.md, Parameters: forwarding-entry
    /// lifetime).
    double forwardingLifetime = 30.0;
    /// A new forward scout may replace an unanswered one after this long
    /// (fidelity.md, Parameters: scout timeout).
    double scoutTimeout = 1.0;
    /// Events held at the source while a route is searched for (fidelity.md,
    /// Parameters: event cache size); golden rule 7.
    std::size_t eventCacheSize = 64;
    /// Node-disjoint paths kept per sink at the source; also the most paths a
    /// sink opens per scout generation (fidelity.md, Parameters: paths kept).
    std::size_t maxPaths = 4;
    /// Scout-cache and forwarding-table capacities, oldest evicted first
    /// (fidelity.md, Parameters: table caps); golden rule 7.
    std::size_t scoutCacheSize = 256;
    std::size_t forwardingTableSize = 256;
};

}  // namespace beesensor::core

#endif  // BEESENSOR_CONFIG_H
