// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// The scratch consumer of the installed beesensor-core package: the headers
// resolve from include/beesensor-core/ and include/swarm-core/, and
// beesensor::core links.
#include "beesensor-core.h"
#include "swarm-ports.h"

#include <cstdio>
#include <string>

int main() {
    const std::string v{beesensor::core::version()};
    std::printf("consumer: ok (beesensor-core %s)\n", v.c_str());
    return 0;
}
