// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
#include "beesensor-wire.h"

#include "swarm-wire.h"

#include <algorithm>
#include <cmath>
#include <type_traits>

namespace beesensor::core {

namespace {

using swarm::core::WireReader;
using swarm::core::WireWriter;

struct Writer {
    WireWriter& w;
    void operator()(const ForwardScout& m) const {
        w.u8(static_cast<std::uint8_t>(Kind::ForwardScout));
        w.u32(m.scoutId);
        w.i32(m.source);
        w.u8(m.hops);
        w.u16(m.energy);
    }
    void operator()(const BackwardScout& m) const {
        w.u8(static_cast<std::uint8_t>(Kind::BackwardScout));
        w.u32(m.scoutId);
        w.i32(m.source);
        w.i32(m.sink);
        w.u32(m.pathId);
        w.u8(m.hops);
        w.u16(m.energy);
    }
    void operator()(const Swarm& m) const {
        w.u8(static_cast<std::uint8_t>(Kind::Swarm));
        w.i32(m.source);
        w.i32(m.sink);
        w.u32(m.pathId);
        w.u16(m.foragers);
        w.u8(m.hops);
        w.u16(m.energy);
    }
    void operator()(const DataHeader& m) const {
        w.u8(static_cast<std::uint8_t>(Kind::Data));
        w.i32(m.source);
        w.i32(m.sink);
        w.u32(m.pathId);
        w.u8(m.hops);
        w.u16(m.energy);
    }
};

}  // namespace

std::vector<std::uint8_t> encode(const Message& m) {
    std::vector<std::uint8_t> out;
    WireWriter w(out);
    w.version(kWireVersion);
    std::visit(Writer{w}, m);
    return out;
}

std::optional<Message> decode(std::span<const std::uint8_t> bytes) {
    WireReader r(bytes);
    if (!r.expectVersion(kWireVersion)) return std::nullopt;
    const std::uint8_t kind = r.u8();
    if (!r.ok()) return std::nullopt;
    Message m;
    switch (static_cast<Kind>(kind)) {
    case Kind::ForwardScout: {
        ForwardScout s;
        s.scoutId = r.u32();
        s.source = r.i32();
        s.hops = r.u8();
        s.energy = r.u16();
        m = s;
        break;
    }
    case Kind::BackwardScout: {
        BackwardScout s;
        s.scoutId = r.u32();
        s.source = r.i32();
        s.sink = r.i32();
        s.pathId = r.u32();
        s.hops = r.u8();
        s.energy = r.u16();
        m = s;
        break;
    }
    case Kind::Swarm: {
        Swarm s;
        s.source = r.i32();
        s.sink = r.i32();
        s.pathId = r.u32();
        s.foragers = r.u16();
        s.hops = r.u8();
        s.energy = r.u16();
        m = s;
        break;
    }
    case Kind::Data: {
        DataHeader h;
        h.source = r.i32();
        h.sink = r.i32();
        h.pathId = r.u32();
        h.hops = r.u8();
        h.energy = r.u16();
        m = h;
        break;
    }
    default:
        return std::nullopt;
    }
    if (!r.atEnd()) return std::nullopt;
    return m;
}

std::uint16_t energyToWire(double fraction) {
    if (std::isnan(fraction)) return kEnergyUnset;
    const double clamped = std::clamp(fraction, 0.0, 1.0);
    return static_cast<std::uint16_t>(std::lround(clamped * kEnergyFull));
}

double energyFromWire(std::uint16_t wire) {
    if (wire == kEnergyUnset) return 1.0;
    return static_cast<double>(std::min(wire, kEnergyFull)) / kEnergyFull;
}

std::uint16_t minEnergy(std::uint16_t a, std::uint16_t b) {
    return std::min(a, b);  // kEnergyUnset (0xFFFF) is the largest value
}

}  // namespace beesensor::core
