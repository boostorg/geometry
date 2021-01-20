// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_as_subrange.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/intersection.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/policies/relate/intersection_points.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace disjoint
{

struct cartesian_segment_box_with_info
{
    template
    <
            typename Segment,
            typename BoxPoint,
            typename Strategy,
            typename T
    >
    inline void static apply(Segment const& segment,
                             BoxPoint const& box_point1,
                             BoxPoint const& box_point2,
                             Strategy const& strategy,
                             T& result)
    {
        typedef typename geometry::select_most_precise
        <
            typename coordinate_type<Segment>::type,
            typename coordinate_type<BoxPoint>::type
        >::type most_precise_type;

        typedef typename boost::mpl::if_c
        <
            boost::is_same
            <
                most_precise_type,
                typename coordinate_type<Segment>::type
            >::value,
            typename point_type<Segment>::type,
            BoxPoint
        >::type point_type;

        typedef segment_intersection_points<point_type> intersection_return_type;
        typedef policies::relate::segments_intersection_points
        <
            intersection_return_type
        > intersection_policy;

        typedef geometry::model::segment<BoxPoint> BoxSegment;
        BoxSegment box_segment = BoxSegment(box_point1, box_point2);

        geometry::detail::segment_as_subrange<Segment> sub_range1(segment);
        geometry::detail::segment_as_subrange<BoxSegment> sub_range2(box_segment);
        result = strategy.apply(sub_range1, sub_range2, intersection_policy());
    }

    template
    <
            typename Segment,
            typename Box,
            typename Strategy,
            typename T
    >
    inline void apply(Segment const& segment,
                      Box const& box,
                      Strategy const& strategy,
                      T& result) const
    {
        typedef typename geometry::point_type<Box>::type box_point;
        box_point top_left, top_right, bottom_left, bottom_right;
        geometry::detail::assign_box_corners(box,
                                             bottom_left, bottom_right,
                                             top_left, top_right);

        apply(segment, bottom_left, bottom_right, strategy, result);
        if (result.count == 0)
        {
            apply(segment, top_left, top_right, strategy, result);
        }

    }
};


}} // namespace strategy::disjoint


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP
