// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace disjoint
{

struct spherical_segment_box_with_info
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
        cartesian_segment_box_with_info::apply(segment, box_point1,
                                               box_point2, strategy, result);
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
                      Strategy const&,
                      T& result) const
    {
        // treat the other (parallel) edges of box differently
        // since they are not (geodesic) segments

        typedef typename geometry::point_type<Box>::type box_point;
        box_point top_left, top_right, bottom_left, bottom_right;
        geometry::detail::assign_box_corners(box,
                                             bottom_left, bottom_right,
                                             top_left, top_right);

        typedef typename geometry::point_type<Segment>::type SegmentPoint;
        SegmentPoint p[2];
        geometry::detail::assign_point_from_index<0>(segment, p[0]);
        geometry::detail::assign_point_from_index<1>(segment, p[1]);

        typedef typename coordinate_type<Segment>::type CT;

        CT lon1 = get_as_radian<0>(p[0]);
        CT const lat1 = get_as_radian<1>(p[0]);
        CT lon2 = get_as_radian<0>(p[1]);
        CT const lat2 = get_as_radian<1>(p[1]);

        CT lat3 = get_as_radian<1>(bottom_left);

        CT lon3_1;
        CT lon3_2;

        if (! formula::crossing_parallel(lon1, lat1,
                                         lon2, lat2,
                                         lat3, lon3_1, lon3_2))
        {
            lat3 = get_as_radian<1>(top_left);
            if (! formula::crossing_parallel(lon1, lat1,
                                             lon2, lat2,
                                             lat3, lon3_1, lon3_2))
            {
                return;
            }
        }

        SegmentPoint p_result;
        set_from_radian<1>(p_result, lat3);
        CT lon_left = get_as_radian<0>(bottom_left);
        CT lon_right = get_as_radian<0>(bottom_right);

#ifdef BOOST_GEOMETRY_DEBUG_SPHERICAL_SEGMENT_BOX
        std::cout << lon_left * math::r2d<CT>() << " , "
                  << lon_right * math::r2d<CT>() << " , "
                  << lon3_1 * math::r2d<CT>() << " , "
                  << lon3_2 * math::r2d<CT>() << std::endl;
#endif
        math::normalize_longitude<radian>(lon1);
        math::normalize_longitude<radian>(lon2);
        math::normalize_longitude<radian>(lon_left);
        math::normalize_longitude<radian>(lon_right);
        math::normalize_longitude<radian>(lon3_1);
        math::normalize_longitude<radian>(lon3_2);
#ifdef BOOST_GEOMETRY_DEBUG_SPHERICAL_SEGMENT_BOX
        std::cout << lon_left * math::r2d<CT>() << " , "
                  << lon_right * math::r2d<CT>() << " , "
                  << lon3_1 * math::r2d<CT>() << " , "
                  << lon3_2 * math::r2d<CT>() << std::endl;
#endif
        if (lon2 < lon1)
        {
            swap(lon1, lon2);
        }
        if (( lon_left < lon3_1 && lon3_1 < lon_right ) &&
                ( lon1 < lon3_1 && lon3_1 < lon2 ))
        {
            set_from_radian<0>(p_result, lon3_1);
            result.count = 1;
            result.intersections[0] = p_result;
            return;
        }
        if (( lon_left < lon3_2 && lon3_2 < lon_right ) &&
                ( lon1 < lon3_2 && lon3_2 < lon2 ))
        {
            set_from_radian<0>(p_result, lon3_2);
            result.count = 1;
            result.intersections[0] = p_result;
        }

    }
};


}} // namespace strategy::disjoint


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP
