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

function(boost_geometry_add_unit_test prefix item)
  set(unit_test_name "boost_geometry_${prefix}_${item}")
  add_executable(${unit_test_name} ${item}.cpp)

  target_link_libraries(${unit_test_name} PRIVATE Boost::geometry)

  target_link_libraries(${unit_test_name} 
    PRIVATE
      Boost::algorithm
      Boost::any
      Boost::array
      Boost::assert
      Boost::concept_check
      Boost::config
      Boost::core
      Boost::function_types
      Boost::iterator
      Boost::lexical_cast
      Boost::math
      Boost::move
      Boost::mpl
      Boost::multiprecision
      Boost::numeric_conversion
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

  target_include_directories(${unit_test_name} PRIVATE "${PROJECT_SOURCE_DIR}/test" .)

  # Add the Boost.Test framework
  target_include_directories(${unit_test_name} PRIVATE "${BOOST_SRC_DIR}/libs/test/include" .)

  # To compile with C++14
  target_compile_features(${unit_test_name} PRIVATE cxx_std_14)

  # To be able to run ctest
  add_test(NAME ${unit_test_name} COMMAND ${unit_test_name})

  # Add a dependency to the global tests target
  add_dependencies(tests ${unit_test_name})

  # Inform the caller about the test name. It can then set defines, if necessary.
  set(BOOST_GEOMETRY_UNIT_TEST_NAME ${unit_test_name} PARENT_SCOPE)
endfunction()

if (NOT TARGET tests)
  add_custom_target(tests)
endif()

add_subdirectory(algorithms)
