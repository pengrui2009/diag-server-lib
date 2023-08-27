#  Diagnostic Server library CMake File
#  Copyright (C) 2023  Rui Peng
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.12)

project(diag-server-example-1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find needed packages
find_package(Threads REQUIRED)

if (${ENABLE_WIN_SYSTEM} MATCHES "ON")
        include_directories(${BOOST_PATH}/include)
        link_directories(${BOOST_PATH}/lib)
else()
        find_package(Boost 1.78.0)
endif()

#find main sources
file(GLOB MAIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME}
        ${MAIN_SRCS}
        )

# diag server lib headers
set(diag_server_header_path ${CMAKE_CURRENT_SOURCE_DIR}/../../diag-server-lib/appl)

# Src headers
set(src_header_path src)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${src_header_path}
        ${diag_server_header_path}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/uds-transport-layer-api/
        )

if (${ENABLE_WIN_SYSTEM} MATCHES "ON")
        # link all dynamic libraries
        target_link_libraries(${PROJECT_NAME} PRIVATE
                diag-server
                Threads::Threads
                libboost_system-mgw7-mt-s-x64-1_78.a
                )
else()
        #link target directory
        target_link_libraries(${PROJECT_NAME} PRIVATE
                diag-server
                Threads::Threads
                ${Boost_LIBRARIES}
                )
endif()