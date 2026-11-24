// Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
// SPDX-License-Identifier: MIT OR GPL-2.0-only
//
// BeeSensor's core: the package's entry header. The skeleton only (#41): no
// protocol logic, which waits for the reviewed fidelity sheet
// (docs/fidelity.md; AGENTS.md golden rule 10, #40). Includes only the C++
// standard library and swarm-core's core/ (golden rule 11).
#ifndef BEESENSOR_CORE_H
#define BEESENSOR_CORE_H

#include <string_view>

namespace beesensor::core {

/// The beesensor-core package version this library was built as (.cz.toml's,
/// or BEESENSOR_CORE_VERSION), e.g. "0.1.0".
std::string_view version() noexcept;

}  // namespace beesensor::core

#endif  // BEESENSOR_CORE_H
