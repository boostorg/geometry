# Originally generated by `boostdep --cmake geometry`
# Adapted manually
# Copyright 2020, 2021 Peter Dimov
# Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.8...3.20)

project(boost_geometry VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

add_library(boost_geometry INTERFACE)
add_library(Boost::geometry ALIAS boost_geometry)

target_include_directories(boost_geometry INTERFACE include)
target_compile_features(boost_geometry INTERFACE cxx_std_14)

target_link_libraries(boost_geometry
  INTERFACE
    Boost::algorithm
    Boost::any
    Boost::array
    Boost::assert
    Boost::concept_check
    Boost::config
    Boost::core
    Boost::crc
    Boost::function_types
    Boost::graph
    Boost::iterator
    Boost::lexical_cast
    Boost::math
    Boost::move
    Boost::mpl
    Boost::multiprecision
    Boost::numeric_conversion
    Boost::program_options
    Boost::qvm
    Boost::range
    Boost::rational
    Boost::static_assert
    Boost::throw_exception
    Boost::tokenizer
    Boost::tuple
    Boost::type_traits
    Boost::utility
    Boost::variant
)

# Required for Boost.Geometry Index
target_link_libraries(boost_geometry
  INTERFACE
    Boost::container
    Boost::serialization
)

# Optional requirements (for example, for adaptations)
if(BOOST_GEOMETRY_BUILD_OPTIONAL)
  target_link_libraries(boost_geometry
    INTERFACE
      Boost::fusion
      Boost::integer
      Boost::polygon
      Boost::variant2
  )

  # Mentioned in SRS Shared_grids_boost (which is optional)
  target_link_libraries(boost_geometry
    INTERFACE
      Boost::thread
  )

  # Requirements for extensions
  target_link_libraries(boost_geometry
    INTERFACE
      Boost::endian
      Boost::predef
      )

endif()

if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")

  if (${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    # Project is root. Find Boost source.
    set(BOOST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE STRING "Boost source dir to use when running CMake from this directory")
    if (NOT IS_ABSOLUTE ${BOOST_SRC_DIR})
      set(BOOST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${BOOST_SRC_DIR}")
    endif()

    set(BOOST_SRC_DIR_IS_VALID ON)
    foreach (PATH "CMakeLists.txt" "Jamroot" "boost-build.jam" "libs")
      if (NOT EXISTS "${BOOST_SRC_DIR}/${PATH}")
        message(STATUS "${BOOST_SRC_DIR}/${PATH} does not exist. Fallback to find_package.")
        set(BOOST_SRC_DIR_IS_VALID OFF)
        break()
      endif()
    endforeach()

    # Create Boost targets from source dir or boost package.
    # These are the direct dependencies currently used in unit tests.
    set(BOOST_INCLUDE_LIBRARIES
          config
          algorithm
          any
          crc
          graph
          lexical_cast
          math
          multiprecision
          program_options
          qvm
          rational
          serialization
          tokenizer
          variant
          test)

    if (BOOST_SRC_DIR_IS_VALID)
      set(BOOST_EXCLUDE_LIBRARIES ${PROJECT_NAME})
      add_subdirectory(${BOOST_SRC_DIR} deps_/boost EXCLUDE_FROM_ALL)
    else()
      find_package(Boost 1.81.0 REQUIRED)
      foreach (BOOST_INCLUDE_LIBRARY ${BOOST_INCLUDE_LIBRARIES})
        add_library(Boost::${BOOST_INCLUDE_LIBRARY} ALIAS Boost::headers)
      endforeach ()
    endif()

  endif()

  enable_testing()
  add_subdirectory(test EXCLUDE_FROM_ALL)
  add_subdirectory(index/test EXCLUDE_FROM_ALL)

endif()

if(BUILD_DOCUMENTATION)
  add_subdirectory(doc/src/examples)
  add_subdirectory(doc/src/docutils/tools/support_status)
endif()

