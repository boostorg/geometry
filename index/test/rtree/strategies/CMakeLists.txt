# Boost.Geometry
# Copyright (c) 2025 Adam Wulkiewicz, Lodz, Poland.
# Copyright (c) 2024, Oracle and/or its affiliates.
# Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

foreach(item IN ITEMS
    rtree_with_strategies_b_l
    rtree_with_strategies_b_q
    rtree_with_strategies_b_r
    rtree_with_strategies_p_l
    rtree_with_strategies_p_q
    rtree_with_strategies_p_r
    rtree_with_strategies_s_l
    rtree_with_strategies_s_q
    rtree_with_strategies_s_r
  )
  boost_geometry_add_unit_test("index" ${item})
endforeach()
