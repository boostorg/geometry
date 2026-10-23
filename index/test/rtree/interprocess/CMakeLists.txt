# Boost.Geometry
# Copyright (c) 2026 Tinko Sebastian Bartels.
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

foreach(item IN ITEMS
    rtree_interprocess_linear
    rtree_interprocess_linear_dyn
    rtree_interprocess_quadratic
    rtree_interprocess_quadratic_dyn
    rtree_interprocess_rstar
    rtree_interprocess_rstar_dyn
  )
  boost_geometry_add_unit_test("index" ${item})
  target_link_libraries(${BOOST_GEOMETRY_UNIT_TEST_NAME} PRIVATE Boost::interprocess)
  # These tests all use the shared-memory object named "shmem".
  set_tests_properties(${BOOST_GEOMETRY_UNIT_TEST_NAME}
    PROPERTIES RESOURCE_LOCK boost_geometry_rtree_interprocess)
endforeach()
