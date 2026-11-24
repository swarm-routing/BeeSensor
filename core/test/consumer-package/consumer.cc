// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The scratch consumer of the installed beesensor-core package: the headers
// resolve from include/beesensor-core/ and include/swarm-core/, and
// beesensor::core links.
#include "beesensor-core.h"
#include "beesensor-router.h"
#include "beesensor-wire.h"

#include <cstdio>
#include <string>

int main() {
    const std::string v{beesensor::core::version()};
    const auto bytes = beesensor::core::encode(beesensor::core::ForwardScout{1, 2, 0, 0});
    if (!beesensor::core::decode(bytes)) return 1;
    std::printf("consumer: ok (beesensor-core %s)\n", v.c_str());
    return 0;
}
