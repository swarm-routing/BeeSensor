# Copyright (c) 2026 Daniel Joppi (https://github.com/swarm-routing)
# SPDX-License-Identifier: MIT OR GPL-2.0-only
#
# BeeSensor's simulator-free core on its own: a static library and its tests,
# with no ns-3 tree. It builds on swarm-core's dual-licensed primitives
# (swarm-routing/swarm-core core/). Today it is the skeleton only (#41): the
# package, the build and the tests, with no protocol logic, which waits for the
# reviewed fidelity sheet (AGENTS.md golden rule 10, #40).
#
#   cmake -S core -B build/core && cmake --build build/core && ctest --test-dir build/core
#
# swarm-core comes from its vcpkg port (swarm-routing/.github#489): with the
# vcpkg toolchain, vcpkg installs the version ../vcpkg.json pins from
# swarm-routing/vcpkg-registry at ../vcpkg-configuration.json's baseline (manifest
# mode), and find_package(swarm-core CONFIG REQUIRED) finds it:
#
#   cmake -S core -B build/core -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
#
# Any installed swarm-core package works too (CMAKE_PREFIX_PATH).
# SWARM_CORE_DIR: a local swarm-core core/ directory instead (development
# against an unreleased swarm-core).
# BEESENSOR_CORE_SANITIZE=ON adds ASan + UBSan, as the sanitized CI leg.
# BEESENSOR_CORE_TESTS builds the tests (ON at the top level).
# BEESENSOR_CORE_SWARM_CORE_FROM_PACKAGE=ON takes swarm-core from find_package
# even when SWARM_CORE_DIR is set; BEESENSOR_CORE_INSTALL=ON (which needs the
# package) adds the install rules and the CMake package beesensor-core:
# find_package(beesensor-core CONFIG REQUIRED) gives the imported target
# beesensor::core (swarm-routing/.github#436, the vcpkg port).
# BEESENSOR_CORE_VERSION is the package version (default: .cz.toml's, else 0.0.0).
cmake_minimum_required(VERSION 3.20)
# The manifest is at the repository root, not in core/: tell the vcpkg
# toolchain (read by project() below) where it is, unless the caller did.
if(NOT DEFINED VCPKG_MANIFEST_DIR AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../vcpkg.json"
   AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
endif()
# The package version: .cz.toml's (Commitizen bumps it at each release), or
# BEESENSOR_CORE_VERSION when core/ is built without it (a release tarball, a port).
set(BEESENSOR_CORE_VERSION "" CACHE STRING "beesensor-core's package version; empty: .cz.toml's")
if(BEESENSOR_CORE_VERSION)
  set(_beesensor_core_version "${BEESENSOR_CORE_VERSION}")
elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../.cz.toml")
  file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../.cz.toml" _beesensor_core_version REGEX "^version = " LIMIT_COUNT 1)
  # a release bump of .cz.toml re-runs the configure step (the package version)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../.cz.toml")
  string(REGEX REPLACE "^version = \"([0-9.]+)\".*" "\\1" _beesensor_core_version "${_beesensor_core_version}")
else()
  set(_beesensor_core_version 0.0.0)
endif()
project(beesensor_core VERSION ${_beesensor_core_version} LANGUAGES CXX)

# The C++ standard of the ns-3 release the org supports (C++23 from ns-3.46),
# the standard swarm-core's core/ targets too (swarm-routing/ns3-swarm-core#141).
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SWARM_CORE_DIR "" CACHE PATH "swarm-core's core/ directory; empty: find_package(swarm-core CONFIG), its vcpkg port")
option(BEESENSOR_CORE_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(_beesensor_core_top ON)
else()
  set(_beesensor_core_top OFF)
endif()
option(BEESENSOR_CORE_TESTS "Build the tests and register them with ctest" ${_beesensor_core_top})
option(BEESENSOR_CORE_SWARM_CORE_FROM_PACKAGE "Take swarm-core from find_package(swarm-core CONFIG) even when SWARM_CORE_DIR is set" OFF)
option(BEESENSOR_CORE_INSTALL "Add the install rules and the CMake package beesensor-core" OFF)
if(SWARM_CORE_DIR AND NOT BEESENSOR_CORE_SWARM_CORE_FROM_PACKAGE)
  set(_beesensor_core_swarm_core_from_dir ON)
else()
  set(_beesensor_core_swarm_core_from_dir OFF)
endif()
if(BEESENSOR_CORE_INSTALL AND _beesensor_core_swarm_core_from_dir)
  message(FATAL_ERROR "BEESENSOR_CORE_INSTALL needs swarm-core from its package, not SWARM_CORE_DIR: the package links an installed swarm-core")
endif()

# Sanitizer flags first: they are directory options, so swarm-core's sources,
# when SWARM_CORE_DIR adds them below, are instrumented too (the vcpkg port is
# a prebuilt library, linked as published).
if(BEESENSOR_CORE_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# swarm-core: its installed CMake package (the vcpkg port, or any install on
# CMAKE_PREFIX_PATH), which defines the imported target swarm::core; or, for
# development, a checkout's core/ as a subproject, which defines the same target.
if(_beesensor_core_swarm_core_from_dir)
  if(NOT EXISTS "${SWARM_CORE_DIR}/swarm-ports.h")
    message(FATAL_ERROR "SWARM_CORE_DIR=${SWARM_CORE_DIR} is not swarm-core's core/ directory")
  endif()
  add_subdirectory(${SWARM_CORE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/swarm-core)
else()
  find_package(swarm-core CONFIG REQUIRED)
endif()

# The strict warnings the org CI applies (sim-containers' swarm-quality.cmake),
# on BeeSensor's own sources only. A swarm-core deprecation stays a warning.
set(BEESENSOR_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast
                       -Wcast-align -Woverloaded-virtual -Wnull-dereference -Wformat=2 -Werror
                       -Wno-error=deprecated-declarations)

add_library(beesensor_core STATIC
  beesensor-core.cc
  beesensor-router.cc
  beesensor-wire.cc
)
add_library(beesensor::core ALIAS beesensor_core)
# The installed package's imported target is beesensor::core too (EXPORT_NAME
# and the export's NAMESPACE).
set_target_properties(beesensor_core PROPERTIES EXPORT_NAME core)
include(GNUInstallDirs)
target_include_directories(beesensor_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/beesensor-core>)
target_compile_definitions(beesensor_core PRIVATE BEESENSOR_CORE_VERSION_STRING="${PROJECT_VERSION}")
if(BEESENSOR_CORE_SANITIZE)
  # The objects are instrumented, so whatever links them needs the sanitizer
  # runtime: add_link_options above reaches this directory's targets only.
  target_link_options(beesensor_core INTERFACE -fsanitize=address,undefined)
endif()
target_link_libraries(beesensor_core PUBLIC swarm::core)
target_compile_options(beesensor_core PRIVATE ${BEESENSOR_WARNINGS})

if(BEESENSOR_CORE_TESTS)
  add_executable(beesensor_core_tests
    test/all-tests.cc
    test/run-core-tests.cc
    test/test-router.cc
    test/test-skeleton.cc
    test/test-wire.cc
  )
  target_link_libraries(beesensor_core_tests PRIVATE beesensor_core)
  target_compile_definitions(beesensor_core_tests PRIVATE BEESENSOR_CORE_VERSION_STRING="${PROJECT_VERSION}")
  target_compile_options(beesensor_core_tests PRIVATE ${BEESENSOR_WARNINGS})

  enable_testing()
  add_test(NAME beesensor-core COMMAND beesensor_core_tests)
endif()

if(BEESENSOR_CORE_INSTALL)
  include(CMakePackageConfigHelpers)
  set(_beesensor_core_cmake_dir ${CMAKE_INSTALL_LIBDIR}/cmake/beesensor-core)
  install(TARGETS beesensor_core EXPORT beesensor-coreTargets
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES
    beesensor-config.h
    beesensor-core.h
    beesensor-router.h
    beesensor-wire.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/beesensor-core)
  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/beesensor-core)
  install(EXPORT beesensor-coreTargets NAMESPACE beesensor:: DESTINATION ${_beesensor_core_cmake_dir})
  configure_package_config_file(beesensor-coreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/beesensor-coreConfig.cmake
    INSTALL_DESTINATION ${_beesensor_core_cmake_dir})
  # As swarm-core (its docs/api-policy.md): before 1.0 any minor may break,
  # so a 0.y release satisfies a request for 0.y only; from 1.0, its major.
  if(PROJECT_VERSION_MAJOR EQUAL 0)
    set(_beesensor_core_compat SameMinorVersion)
  else()
    set(_beesensor_core_compat SameMajorVersion)
  endif()
  write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/beesensor-coreConfigVersion.cmake
    COMPATIBILITY ${_beesensor_core_compat})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/beesensor-coreConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/beesensor-coreConfigVersion.cmake
          DESTINATION ${_beesensor_core_cmake_dir})
endif()
