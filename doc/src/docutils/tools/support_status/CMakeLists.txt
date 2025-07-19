# Boost.Geometry

# Copyright (c) 2025-2025 Barend Gehrels, Amsterdam, the Netherlands.

# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Official repository: https://github.com/boostorg/geometry
# Documentation: http://www.boost.org/libs/geometry

cmake_minimum_required(VERSION 3.8...3.20)
project(support_status LANGUAGES CXX)

add_executable(${PROJECT_NAME} support_status.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::geometry)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
