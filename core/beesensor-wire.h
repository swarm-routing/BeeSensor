// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// BeeSensor's control messages and data header, v0 (golden rule 3): POD
// structs, written and read little-endian through swarm-core's bounds-checked
// wire helpers, each starting with the version byte and the kind byte. The
// fields are the ones docs/fidelity.md's Mechanisms rows name; the layout is a
// repo choice (fidelity.md: "Control packets ... as a versioned wire header").
//
//   ForwardScout   ver kind | scoutId u32 | source i32 | hops u8 | energy u16                         13 bytes
//   BackwardScout  ver kind | scoutId u32 | source i32 | sink i32 | pathId u32 | hops u8 | energy u16  21 bytes
//   Swarm          ver kind | source i32 | sink i32 | pathId u32 | foragers u16 | hops u8 | energy u16  19 bytes
//   DataHeader     ver kind | source i32 | sink i32 | pathId u32 | hops u8 | energy u16                17 bytes
//
// `energy` is the least residual energy seen on the path, in units of 1/10000
// of a full battery (0..10000); kEnergyUnset (0xFFFF) is the "infinite" start
// value, kept while no node on the path has reported an energy.
#ifndef BEESENSOR_WIRE_H
#define BEESENSOR_WIRE_H

#include "swarm-types.h"

#include <cstdint>
#include <optional>
#include <span>
#include <variant>
#include <vector>

namespace beesensor::core {

using swarm::core::NodeAddress;

inline constexpr std::uint8_t kWireVersion = 1;
inline constexpr std::uint16_t kEnergyUnset = 0xFFFF;
inline constexpr std::uint16_t kEnergyFull = 10000;

enum class Kind : std::uint8_t {
    ForwardScout = 1,
    BackwardScout = 2,
    Swarm = 3,
    Data = 4,
};

struct ForwardScout {
    std::uint32_t scoutId = 0;
    NodeAddress source = swarm::core::kInvalidAddress;
    std::uint8_t hops = 0;
    std::uint16_t energy = kEnergyUnset;
    bool operator==(const ForwardScout&) const = default;
};

struct BackwardScout {
    std::uint32_t scoutId = 0;
    NodeAddress source = swarm::core::kInvalidAddress;
    NodeAddress sink = swarm::core::kInvalidAddress;
    std::uint32_t pathId = 0;
    std::uint8_t hops = 0;
    std::uint16_t energy = kEnergyUnset;
    bool operator==(const BackwardScout&) const = default;
};

struct Swarm {
    NodeAddress source = swarm::core::kInvalidAddress;
    NodeAddress sink = swarm::core::kInvalidAddress;
    std::uint32_t pathId = 0;
    std::uint16_t foragers = 0;
    std::uint8_t hops = 0;
    std::uint16_t energy = kEnergyUnset;
    bool operator==(const Swarm&) const = default;
};

/// The header a forager puts in front of the event it carries.
struct DataHeader {
    NodeAddress source = swarm::core::kInvalidAddress;
    NodeAddress sink = swarm::core::kInvalidAddress;
    std::uint32_t pathId = 0;
    std::uint8_t hops = 0;
    std::uint16_t energy = kEnergyUnset;
    bool operator==(const DataHeader&) const = default;
};

using Message = std::variant<ForwardScout, BackwardScout, Swarm, DataHeader>;

/// The encoded message, version byte first.
std::vector<std::uint8_t> encode(const Message& m);

/// The message in `bytes`, or nullopt: a wrong version, an unknown kind, a
/// short buffer or trailing bytes all fail closed.
std::optional<Message> decode(std::span<const std::uint8_t> bytes);

/// A residual-energy fraction (swarm-core IEnergy, 0..1, NaN when unknown) in
/// wire units: NaN gives kEnergyUnset, the rest is clamped to 0..kEnergyFull.
std::uint16_t energyToWire(double fraction);

/// The wire value as a fraction; kEnergyUnset reads as 1 (no node reported
/// less than a full battery).
double energyFromWire(std::uint16_t wire);

/// The smaller of two wire energies, kEnergyUnset being the largest.
std::uint16_t minEnergy(std::uint16_t a, std::uint16_t b);

}  // namespace beesensor::core

#endif  // BEESENSOR_WIRE_H
