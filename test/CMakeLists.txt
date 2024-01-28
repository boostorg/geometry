# Boost.Geometry
# Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# This takes care of many (but not all) floating point differences on arm64/clang14 on Mac.
# It also lets running the tests much faster.
if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-contract=fast")
endif()  

set(BOOST_GEOMETRY_TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Set the BOOST_ROOT variable (if not avaiable) 3 folders higher, above libs/geometry/test
set(BOOST_GEOMETRY_BOOST_ROOT "")
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../.." BOOST_GEOMETRY_BOOST_ROOT)
get_filename_component(BOOST_GEOMETRY_BOOST_ROOT "${BOOST_GEOMETRY_BOOST_ROOT}" REALPATH)

function(boost_geometry_add_unit_test prefix item)
  set(unit_test_name "${prefix}_${item}")
  add_executable(${unit_test_name} ${item}.cpp)
  target_include_directories(${unit_test_name} PRIVATE ${BOOST_GEOMETRY_BOOST_ROOT} ${BOOST_GEOMETRY_TEST_ROOT} .)
  target_compile_features(${unit_test_name} PRIVATE cxx_std_14)
  add_test(NAME ${unit_test_name} COMMAND ${unit_test_name})
  # Inform the caller about the test name. It can then set defines, if necessary.
  set(BOOST_GEOMETRY_UNIT_TEST_NAME ${unit_test_name} PARENT_SCOPE)
endfunction()

add_subdirectory(algorithms)

#add_subdirectory(geometries/custom_non_copiable)
# add_subdirectory(strategies)

#add_subdirectory(io/wkt)

