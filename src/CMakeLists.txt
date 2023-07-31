#  Diagnostic Server library CMake File
#  Copyright (C) 2023  Rui Peng
# 
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.12)
project(diag-server-lib)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Build all the libraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Build the diag server application
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/appl)

#Export library
install(EXPORT DiagServerLibTargets
        FILE DiagServerLibTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DiagServerLib
        )

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/DiagServerLibConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DiagServerLib
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/DiagServerLibConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DiagServerLib
        )
