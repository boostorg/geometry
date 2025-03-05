# Boost.Geometry
# Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

add_subdirectory(union)
add_subdirectory(intersection)
add_subdirectory(difference)
add_subdirectory(sym_difference)

foreach(item IN ITEMS
    set_ops_areal_areal
        )
  boost_geometry_add_unit_test("algorithms" ${item})
endforeach()