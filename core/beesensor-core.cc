// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
#include "beesensor-core.h"

namespace beesensor::core {

std::string_view version() noexcept {
    return BEESENSOR_CORE_VERSION_STRING;
}

}  // namespace beesensor::core
