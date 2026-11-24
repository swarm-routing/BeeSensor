// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
#include "beesensor-router.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>

namespace beesensor::core {

namespace {

constexpr std::array<swarm::core::KindName, 4> kKindNames{{
    {static_cast<std::uint8_t>(Kind::ForwardScout), "FSCOUT"},
    {static_cast<std::uint8_t>(Kind::BackwardScout), "BSCOUT"},
    {static_cast<std::uint8_t>(Kind::Swarm), "SWARM"},
    {static_cast<std::uint8_t>(Kind::Data), "FORAGER"},
}};

std::uint8_t incremented(std::uint8_t hops) {
    return hops == 0xFF ? hops : static_cast<std::uint8_t>(hops + 1);
}

}  // namespace

const char* kindName(std::uint8_t kind) {
    return swarm::core::findKindName(kKindNames, kind);
}

double reward(const Path& p) {
    const double hops = std::max<double>(1.0, p.hops);
    return energyFromWire(p.energy) / hops;
}

Router::Router(NodeAddress self, bool isSink, const Config& config, swarm::core::IClock& clock,
               swarm::core::IRng& rng, swarm::core::ITimerScheduler& timers,
               const swarm::core::IEnergy* energy)
    : self_(self), isSink_(isSink), config_(config), clock_(clock), rng_(rng), timers_(timers),
      energy_(energy) {
    config_.maxPaths = std::max<std::size_t>(1, config_.maxPaths);
    config_.scoutCacheSize = std::max<std::size_t>(1, config_.scoutCacheSize);
    config_.forwardingTableSize = std::max<std::size_t>(1, config_.forwardingTableSize);
}

std::uint16_t Router::ownEnergy() const {
    return energy_ ? energyToWire(energy_->residualEnergyFraction()) : kEnergyUnset;
}

void Router::sent(Kind kind, bool broadcast) {
    if (observer_) observer_->onControlSent(static_cast<std::uint8_t>(kind), broadcast, 0);
}

void Router::received(Kind kind) {
    if (observer_) observer_->onControlReceived(static_cast<std::uint8_t>(kind), 0);
}

// ---- expiry (fidelity.md: path maintenance; forager TTL; entry lifetime) ----

void Router::expirePaths() {
    const Time t = now();
    std::erase_if(paths_, [&](const Path& p) {
        const bool lost = p.outstanding > 0 && t - p.oldestOutstanding > config_.foragerReturnWait;
        const bool idle = t - p.lastActive > config_.foragerTtl;
        if ((lost || idle) && observer_) observer_->onRouteChanged(p.sink, p.nextHop, false);
        return lost || idle;
    });
}

void Router::expire() {
    expirePaths();
    const Time t = now();
    while (!forwardingOrder_.empty()) {
        auto it = forwarding_.find(forwardingOrder_.front());
        if (it != forwarding_.end() && t - it->second.at <= config_.forwardingLifetime) break;
        if (it != forwarding_.end()) forwarding_.erase(it);
        forwardingOrder_.pop_front();
    }
    while (!scoutOrder_.empty()) {
        auto it = scoutCache_.find(scoutOrder_.front());
        if (it != scoutCache_.end() && t - it->second.at <= config_.forwardingLifetime) break;
        if (it != scoutCache_.end()) scoutCache_.erase(it);
        scoutOrder_.pop_front();
    }
}

const std::vector<Path>& Router::paths() {
    expirePaths();
    return paths_;
}

// ---- the packer and the forward scout's launch ----

ForwardScout Router::newScout() {
    ForwardScout s;
    s.scoutId = nextScoutId_++;
    s.source = self_;
    s.hops = 0;
    s.energy = kEnergyUnset;  // fidelity.md: initial minimum-energy field (infinite)
    rememberScout({self_, s.scoutId}, self_);
    searching_ = true;
    searchStarted_ = now();
    sent(Kind::ForwardScout, true);
    return s;
}

Path* Router::choosePath() {
    if (paths_.empty()) return nullptr;
    double total = 0;
    for (const Path& p : paths_)
        total += reward(p);
    if (total <= 0) return &paths_.front();
    // fidelity.md, Parameters: path choice probability p_k = r_k / sum r
    double pick = rng_.uniform() * total;
    for (Path& p : paths_) {
        pick -= reward(p);
        if (pick < 0) return &p;
    }
    return &paths_.back();
}

EventDecision Router::onEvent() {
    expire();
    if (Path* p = choosePath()) {
        DataHeader h;
        h.source = self_;
        h.sink = p->sink;
        h.pathId = p->pathId;
        h.hops = 0;
        h.energy = kEnergyUnset;
        if (p->outstanding == 0) p->oldestOutstanding = now();
        ++p->outstanding;
        p->lastActive = now();
        return {Decision::unicast(p->nextHop, h)};
    }
    if (!searching_ || now() - searchStarted_ > config_.scoutTimeout) {
        // the event rides in the forward scout's payload ([ZSAC13] p. 3)
        return {Decision::broadcast(newScout())};
    }
    if (queuedEvents_ >= config_.eventCacheSize) return {Decision::drop()};
    ++queuedEvents_;
    return {Decision{RouteAction::Queue, swarm::core::kInvalidAddress, std::nullopt}};
}

// ---- scouting ----

void Router::rememberScout(const GenKey& key, NodeAddress from) {
    if (scoutCache_.size() >= config_.scoutCacheSize && !scoutOrder_.empty()) {
        scoutCache_.erase(scoutOrder_.front());
        scoutOrder_.pop_front();
    }
    scoutCache_[key] = ScoutEntry{from, now(), false};
    scoutOrder_.push_back(key);
}

void Router::addForwarding(const PathKey& key, const Forwarding& f) {
    if (forwarding_.size() >= config_.forwardingTableSize && !forwardingOrder_.empty()) {
        forwarding_.erase(forwardingOrder_.front());
        forwardingOrder_.pop_front();
    }
    forwarding_[key] = f;
    forwardingOrder_.push_back(key);
}

std::vector<Decision> Router::onForwardScout(const ForwardScout& scout, NodeAddress from) {
    received(Kind::ForwardScout);
    expire();
    std::vector<Decision> out;
    const GenKey gen{scout.source, scout.scoutId};
    if (scout.source == self_) return out;  // our own scout, echoed back

    ForwardScout s = scout;
    s.hops = incremented(s.hops);

    if (isSink_) {
        auto [it, fresh] = sinkGenerations_.try_emplace(gen);
        if (fresh) {
            sinkGenerationOrder_.push_back(gen);
            if (sinkGenerationOrder_.size() > config_.scoutCacheSize) {
                sinkGenerations_.erase(sinkGenerationOrder_.front());
                sinkGenerationOrder_.pop_front();
            }
            out.push_back(Decision::deliver());  // the event in the scout's payload
        }
        auto& seen = it->second.neighbours;
        if (seen.size() >= config_.maxPaths ||
            std::find(seen.begin(), seen.end(), from) != seen.end()) {
            return out;
        }
        seen.push_back(from);
        BackwardScout b;
        b.scoutId = s.scoutId;
        b.source = s.source;
        b.sink = self_;
        b.pathId = nextPathId_++;
        b.hops = s.hops;
        b.energy = kEnergyUnset;  // reset to infinite at the sink
        addForwarding({self_, b.pathId}, Forwarding{self_, from, now()});
        out.push_back(Decision::unicast(from, b));
        sent(Kind::BackwardScout, false);
        return out;
    }

    auto cached = scoutCache_.find(gen);
    if (cached != scoutCache_.end()) {
        out.push_back(Decision::drop());  // a duplicate: rebroadcast once only
        return out;
    }
    rememberScout(gen, from);
    s.energy = minEnergy(s.energy, ownEnergy());
    const bool rebroadcast =
        s.hops <= config_.unconditionalHops || rng_.uniform() < config_.rebroadcastProbability;
    if (rebroadcast) {
        out.push_back(Decision::broadcast(s));
        sent(Kind::ForwardScout, true);
    } else {
        out.push_back(Decision::drop());
    }
    return out;
}

void Router::addPath(const Path& p) {
    for (const Path& q : paths_) {
        if (q.sink == p.sink && q.pathId == p.pathId) return;
    }
    if (paths_.size() >= config_.maxPaths) {
        auto worst =
            std::min_element(paths_.begin(), paths_.end(),
                             [](const Path& a, const Path& b) { return reward(a) < reward(b); });
        if (reward(*worst) >= reward(p)) return;
        if (observer_) observer_->onRouteChanged(worst->sink, worst->nextHop, false);
        paths_.erase(worst);
    }
    paths_.push_back(p);
    if (observer_) observer_->onRouteChanged(p.sink, p.nextHop, true);
}

Decision Router::onBackwardScout(const BackwardScout& scout, NodeAddress from) {
    received(Kind::BackwardScout);
    expire();
    const GenKey gen{scout.source, scout.scoutId};
    const PathKey key{scout.sink, scout.pathId};

    if (scout.source == self_) {
        Path p;
        p.sink = scout.sink;
        p.pathId = scout.pathId;
        p.nextHop = from;
        p.hops = scout.hops;
        p.energy = scout.energy;
        p.lastActive = now();
        addPath(p);
        searching_ = false;
        if (queuedEvents_ > 0) {
            queuedEvents_ = 0;
            if (routeReady_) routeReady_();
        }
        return Decision::none();
    }

    if (forwarding_.contains(key)) {
        // a second visit: a loop; drop it and flush the entry ([IJARCCE13] p. 2963)
        forwarding_.erase(key);
        return Decision::drop();
    }
    auto cached = scoutCache_.find(gen);
    if (cached == scoutCache_.end() || cached->second.answered) {
        // no forward scout passed here, or this node already serves another
        // path of the generation: paths stay node-disjoint
        return Decision::drop();
    }
    cached->second.answered = true;
    addForwarding(key, Forwarding{from, cached->second.prevHop, now()});
    BackwardScout b = scout;
    b.energy = minEnergy(b.energy, ownEnergy());
    sent(Kind::BackwardScout, false);
    return Decision::unicast(cached->second.prevHop, b);
}

// ---- foraging and swarming ----

Decision Router::onData(DataHeader& header, NodeAddress /*from*/) {
    expire();
    auto it = forwarding_.find({header.sink, header.pathId});
    if (it == forwarding_.end()) {
        if (observer_) observer_->onDataDecision(header.sink, RouteAction::Drop);
        return Decision::drop();
    }
    it->second.at = now();
    header.hops = incremented(header.hops);
    if (it->second.nextHop == self_) {
        // the sink: the forager waits to return in a swarm
        const PathKey key{header.sink, header.pathId};
        auto [w, fresh] = waiting_.try_emplace(key);
        w->second.source = header.source;
        if (w->second.foragers < 0xFFFF) ++w->second.foragers;
        w->second.hops = std::max(w->second.hops, header.hops);
        w->second.energy = minEnergy(w->second.energy, header.energy);
        if (fresh) timers_.schedule(config_.sinkSwarmWait, [this, key] { sendSwarm(key); });
        if (observer_) observer_->onDataDecision(header.sink, RouteAction::Deliver);
        return Decision::deliver();
    }
    header.energy = minEnergy(header.energy, ownEnergy());
    if (observer_) observer_->onDataDecision(header.sink, RouteAction::Unicast);
    return Decision::unicast(it->second.nextHop, header);
}

void Router::sendSwarm(PathKey key) {
    auto w = waiting_.find(key);
    if (w == waiting_.end()) return;
    Swarm s;
    s.source = w->second.source;
    s.sink = key.first;
    s.pathId = key.second;
    s.foragers = w->second.foragers;
    s.hops = w->second.hops;
    s.energy = w->second.energy;
    waiting_.erase(w);
    expire();
    auto f = forwarding_.find(key);
    if (f == forwarding_.end()) return;  // the path expired meanwhile
    f->second.at = now();
    sent(Kind::Swarm, false);
    if (controlSink_) controlSink_(Decision::unicast(f->second.prevHop, s));
}

Decision Router::onSwarm(const Swarm& swarm, NodeAddress /*from*/) {
    received(Kind::Swarm);
    expire();
    if (swarm.source == self_) {
        auto p = std::find_if(paths_.begin(), paths_.end(), [&](const Path& q) {
            return q.sink == swarm.sink && q.pathId == swarm.pathId;
        });
        if (p == paths_.end()) return Decision::none();
        p->outstanding -= std::min<std::uint32_t>(p->outstanding, swarm.foragers);
        p->oldestOutstanding = now();
        p->lastActive = now();
        p->hops = swarm.hops;
        p->energy = swarm.energy;
        // fidelity.md, Parameters: pL; dropped only when a better path exists
        if (energyFromWire(p->energy) < config_.energyFloor) {
            const double r = reward(*p);
            const bool better = std::any_of(paths_.begin(), paths_.end(),
                                            [&](const Path& q) { return reward(q) > r; });
            if (better) {
                if (observer_) observer_->onRouteChanged(p->sink, p->nextHop, false);
                paths_.erase(p);
            }
        }
        return Decision::none();
    }
    auto it = forwarding_.find({swarm.sink, swarm.pathId});
    if (it == forwarding_.end()) return Decision::drop();
    it->second.at = now();
    sent(Kind::Swarm, false);
    return Decision::unicast(it->second.prevHop, swarm);
}

}  // namespace beesensor::core
