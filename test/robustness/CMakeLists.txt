# Boost.Geometry
# Copyright (c) 2024, Oracle and/or its affiliates.
# Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


function(boost_geometry_add_robustness_test item)
  set(robustness_test_name "boost_geometry_robustness_${item}")
  add_executable(${robustness_test_name} ${item}.cpp)

  # Add a dependency to Boost.Geometry
  target_link_libraries(${robustness_test_name}
    PRIVATE
      Boost::geometry)

  # Link to additional libraries required for testing.
  target_link_libraries(${robustness_test_name}
    PRIVATE
      Boost::crc
      Boost::program_options)

  # Include the main Geometry test folder and the current folder,
  # and the robustness test folder
  target_include_directories(${robustness_test_name}
    PRIVATE
      "${PROJECT_SOURCE_DIR}/test/robustness"
      "${PROJECT_SOURCE_DIR}/test"
      "${PROJECT_SOURCE_DIR}/index/test"
      .)
 
  # To compile with C++14
  target_compile_features(${robustness_test_name} PRIVATE cxx_std_14)
  
  # Add a dependency to the global robustness tests target
  add_dependencies(robustness_tests ${robustness_test_name})

endfunction()

if (NOT TARGET robustness_tests)
  add_custom_target(robustness_tests)
endif()

add_subdirectory(convex_hull)
add_subdirectory(overlay)
#add_subdirectory(within)
