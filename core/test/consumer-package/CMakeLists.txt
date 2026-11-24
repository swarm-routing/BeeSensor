# Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
# SPDX-License-Identifier: MIT OR GPL-2.0-only
#
# A scratch consumer of the installed package beesensor-core (core.yml's
# package check): find_package finds it and its swarm-core dependency, and
# beesensor::core links.
cmake_minimum_required(VERSION 3.20)
project(beesensor_core_package_consumer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(beesensor-core CONFIG REQUIRED)
message(STATUS "beesensor-core ${beesensor-core_VERSION} from ${beesensor-core_DIR}")
add_executable(consumer consumer.cc)
target_link_libraries(consumer PRIVATE beesensor::core)
