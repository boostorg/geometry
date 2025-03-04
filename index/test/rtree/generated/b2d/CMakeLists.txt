# Boost.Geometry
# Copyright (c) 2025 Adam Wulkiewicz, Lodz, Poland.
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

foreach(item IN ITEMS
    rtree_lin_add_b2d
    rtree_lin_mod_b2d
    rtree_lin_que_b2d
    rtree_qua_add_b2d
    rtree_qua_mod_b2d
    rtree_qua_que_b2d
    rtree_rst_add_b2d
    rtree_rst_mod_b2d
    rtree_rst_que_b2d
  )
  boost_geometry_add_unit_test("index" ${item})
endforeach()
