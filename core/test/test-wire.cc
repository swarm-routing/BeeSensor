// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The wire format (golden rule 3): roundtrip, the decoder failing closed, a
// seeded fuzz, and the energy units.
#include "beesensor-core-tests.h"
#include "beesensor-test-support.h"
#include "beesensor-wire.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace beesensor::core::test {

namespace {

std::vector<Message> samples() {
    return {
        ForwardScout{7, 3, 2, 4321},           ForwardScout{0xFFFFFFFFu, -5, 255, kEnergyUnset},
        BackwardScout{9, 1, 4, 77, 5, 1200},   Swarm{1, 4, 77, 12, 5, 900},
        DataHeader{1, 4, 77, 3, kEnergyUnset},
    };
}

}  // namespace

void TestWireRoundtrip(Checker& c) {
    const std::vector<std::size_t> sizes{13, 13, 21, 19, 17};
    const auto msgs = samples();
    for (std::size_t i = 0; i < msgs.size(); ++i) {
        const auto bytes = encode(msgs[i]);
        c.expect(bytes.size() == sizes[i],
                 "message " + std::to_string(i) + " has its documented size");
        c.expect(!bytes.empty() && bytes[0] == kWireVersion, "the version byte leads");
        const auto back = decode(bytes);
        c.expect(back.has_value() && *back == msgs[i],
                 "message " + std::to_string(i) + " roundtrips");
    }
    // little-endian: the scout ID's low byte follows version and kind
    const auto fs = encode(ForwardScout{0x04030201u, 0, 0, 0});
    c.expect(fs[2] == 0x01 && fs[5] == 0x04, "fields are little-endian");
}

void TestWireFailsClosed(Checker& c) {
    auto bytes = encode(BackwardScout{9, 1, 4, 77, 5, 1200});
    auto wrongVersion = bytes;
    wrongVersion[0] = kWireVersion + 1;
    c.expect(!decode(wrongVersion), "a wrong version byte is refused");
    auto unknownKind = bytes;
    unknownKind[1] = 99;
    c.expect(!decode(unknownKind), "an unknown kind is refused");
    for (std::size_t n = 0; n < bytes.size(); ++n) {
        std::vector<std::uint8_t> shorter(bytes.begin(), bytes.begin() + static_cast<long>(n));
        c.expect(!decode(shorter), "a buffer cut to " + std::to_string(n) + " bytes is refused");
    }
    auto longer = bytes;
    longer.push_back(0);
    c.expect(!decode(longer), "trailing bytes are refused");
}

void TestWireFuzz(Checker& c) {
    FakeRng rng(20261124);
    auto u32 = [&] { return static_cast<std::uint32_t>(rng.uniform() * 4294967295.0); };
    auto u16 = [&] { return static_cast<std::uint16_t>(rng.uniformInt(65536)); };
    auto u8 = [&] { return static_cast<std::uint8_t>(rng.uniformInt(256)); };
    int roundtrips = 0;
    for (int i = 0; i < 5000; ++i) {
        // a random valid message roundtrips
        Message m;
        switch (rng.uniformInt(4)) {
        case 0:
            m = ForwardScout{u32(), static_cast<NodeAddress>(u32()), u8(), u16()};
            break;
        case 1:
            m = BackwardScout{u32(),
                              static_cast<NodeAddress>(u32()),
                              static_cast<NodeAddress>(u32()),
                              u32(),
                              u8(),
                              u16()};
            break;
        case 2:
            m = Swarm{static_cast<NodeAddress>(u32()),
                      static_cast<NodeAddress>(u32()),
                      u32(),
                      u16(),
                      u8(),
                      u16()};
            break;
        default:
            m = DataHeader{static_cast<NodeAddress>(u32()), static_cast<NodeAddress>(u32()), u32(),
                           u8(), u16()};
        }
        auto bytes = encode(m);
        const auto back = decode(bytes);
        if (back && *back == m) ++roundtrips;
        // a mutated or random buffer never crashes, and what decodes re-encodes as it was
        bytes[static_cast<std::size_t>(rng.uniformInt(static_cast<int>(bytes.size())))] = u8();
        if (rng.uniformInt(3) == 0) bytes.resize(static_cast<std::size_t>(rng.uniformInt(24)));
        if (auto d = decode(bytes)) {
            c.expect(encode(*d) == bytes, "a decoded mutated buffer re-encodes byte for byte");
        }
    }
    c.expect(roundtrips == 5000, "every random valid message roundtrips");
}

void TestEnergyUnits(Checker& c) {
    c.expect(energyToWire(std::numeric_limits<double>::quiet_NaN()) == kEnergyUnset,
             "an unknown energy is the unset value");
    c.expect(energyToWire(1.0) == kEnergyFull && energyToWire(0.0) == 0, "0 and 1 map to the ends");
    c.expect(energyToWire(2.0) == kEnergyFull && energyToWire(-1.0) == 0,
             "out of range is clamped");
    c.expect(energyToWire(0.25) == 2500, "a quarter is 2500");
    c.expect(std::abs(energyFromWire(2500) - 0.25) < 1e-12, "and back");
    c.expect(energyFromWire(kEnergyUnset) == 1.0, "unset reads as a full battery");
    c.expect(minEnergy(kEnergyUnset, 300) == 300 && minEnergy(300, 200) == 200,
             "the minimum treats unset as the largest");
}

}  // namespace beesensor::core::test
