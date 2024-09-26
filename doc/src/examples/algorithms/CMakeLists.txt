# Boost.Geometry
# Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.
# Use, modification and distribution is subject to the Boost Software License,
# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

foreach(item IN ITEMS
    append
    area
    area_with_strategy
    assign
    assign_2d_point
    assign_3d_point
    assign_inverse
    assign_points
    buffer_with_strategies
    centroid
    clear
    closest_points
    closest_points_strategy
    comparable_distance
    convert
    convex_hull
    correct
    densify
    densify_strategy
    discrete_frechet_distance
    discrete_frechet_distance_strategy
    discrete_hausdorff_distance
    discrete_hausdorff_distance_strategy
    difference
    distance
    envelope
    equals
    expand
    for_each_point
    for_each_point_const
    for_each_segment_const
    intersection_ls_ls_point
    intersection_segment
    intersects_linestring
    is_simple
    is_valid
    is_valid_failure
    is_valid_message
    length
    length_with_strategy
    line_interpolate
    line_interpolate_strategy
    make_2d_point
    make_3d_point
    make_inverse
    num_geometries
    num_interior_rings
    num_points
    num_segments
    return_envelope
    relate
    relation
    reverse
    simplify
    sym_difference
    transform
    transform_with_strategy
    union
    unique
    within
  )

  boost_geometry_add_example("algorithms" ${item})

endforeach()

