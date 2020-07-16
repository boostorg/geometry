// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018-2020 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_GENERIC_SEGMENT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_GENERIC_SEGMENT_BOX_HPP

#include <boost/geometry/algorithms/detail/distance/segment_to_box.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/normalize.hpp>
#include <boost/geometry/strategies/spherical/disjoint_box_box.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>
#include <boost/geometry/strategies/cartesian/point_in_box.hpp> // spherical
#include <boost/geometry/strategies/spherical/ssf.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace distance
{

struct generic_segment_box
{
    template
    <
            typename LessEqual,
            typename ReturnType,
            typename SegmentPoint,
            typename BoxPoint,
            typename SegmentBoxStrategy,
            typename AzimuthStrategy,
            typename EnvelopeSegmentStrategy,
            typename NormalizePointStrategy,
            typename DisjointPointBoxStrategy,
            typename DisjointBoxBoxStrategy
    >
    static inline ReturnType segment_below_of_box(
            SegmentPoint const& p0,
            SegmentPoint const& p1,
            BoxPoint const& top_left,
            BoxPoint const& top_right,
            BoxPoint const& bottom_left,
            BoxPoint const& bottom_right,
            SegmentBoxStrategy const& sb_strategy,
            AzimuthStrategy const& az_strategy,
            EnvelopeSegmentStrategy const& es_strategy,
            NormalizePointStrategy const& np_strategy,
            DisjointPointBoxStrategy const& dpb_strategy,
            DisjointBoxBoxStrategy const& dbb_strategy)
    {
        ReturnType result;
        typename LessEqual::other less_equal;
        typedef geometry::model::segment<SegmentPoint> segment_type;
        // if cs_tag is spherical_tag check segment's cs_tag with spherical_equatorial_tag as default
        typedef typename boost::mpl::if_c
            <
                boost::is_same<typename SegmentBoxStrategy::cs_tag, spherical_tag>::value,
                typename boost::mpl::if_c
                    <
                        boost::is_same
                            <
                                typename geometry::cs_tag<segment_type>::type,
                                spherical_polar_tag
                            >::value,
                        spherical_polar_tag, spherical_equatorial_tag
                    >::type,
                typename SegmentBoxStrategy::cs_tag
            >::type cs_tag;
        typedef geometry::detail::disjoint::
                disjoint_segment_box_sphere_or_spheroid<cs_tag>
                disjoint_sb;
        typedef typename disjoint_sb::disjoint_info disjoint_info_type;

        segment_type seg(p0, p1);

        geometry::model::box<BoxPoint> input_box;
        geometry::set_from_radian<geometry::min_corner, 0>
                (input_box, geometry::get_as_radian<0>(bottom_left));
        geometry::set_from_radian<geometry::min_corner, 1>
                (input_box, geometry::get_as_radian<1>(bottom_left));
        geometry::set_from_radian<geometry::max_corner, 0>
                (input_box, geometry::get_as_radian<0>(top_right));
        geometry::set_from_radian<geometry::max_corner, 1>
                (input_box, geometry::get_as_radian<1>(top_right));

        SegmentPoint p_max;

        disjoint_info_type disjoint_result = disjoint_sb::
                apply(seg, input_box, p_max,
                      az_strategy, np_strategy, dpb_strategy, dbb_strategy);

        if (disjoint_result == disjoint_info_type::intersect) //intersect
        {
            strategy::distance::services::closest_points_seg_box<SegmentBoxStrategy>
                    ::apply(p0, p1,
                            top_left, top_right, bottom_left, bottom_right,
                            sb_strategy, result);
            return result;
        }
        // disjoint but vertex not computed
        if (disjoint_result == disjoint_info_type::disjoint_no_vertex)
        {
            typedef typename coordinate_type<SegmentPoint>::type CT;

            geometry::model::box<SegmentPoint> mbr;
            geometry::envelope(seg, mbr, es_strategy);

            CT lon1 = geometry::get_as_radian<0>(p0);
            CT lat1 = geometry::get_as_radian<1>(p0);
            CT lon2 = geometry::get_as_radian<0>(p1);
            CT lat2 = geometry::get_as_radian<1>(p1);

            if (lon1 > lon2)
            {
                std::swap(lon1, lon2);
                std::swap(lat1, lat2);
            }

            CT vertex_lat;
            CT lat_sum = lat1 + lat2;
            if (lat_sum > CT(0))
            {
                vertex_lat = geometry::get_as_radian<geometry::max_corner, 1>(mbr);
            } else {
                vertex_lat = geometry::get_as_radian<geometry::min_corner, 1>(mbr);
            }

            CT alp1;
            az_strategy.apply(lon1, lat1, lon2, lat2, alp1);
            CT vertex_lon = geometry::formula::vertex_longitude
                    <
                    CT,
                    cs_tag
                    >::apply(lon1, lat1, lon2, lat2,
                             vertex_lat, alp1, az_strategy);

            geometry::set_from_radian<0>(p_max, vertex_lon);
            geometry::set_from_radian<1>(p_max, vertex_lat);
        }
        //otherwise disjoint and vertex computed inside disjoint

        if (less_equal(geometry::get_as_radian<0>(bottom_left),
                       geometry::get_as_radian<0>(p_max)))
        {
            result = boost::numeric_cast<ReturnType>(typename
                        SegmentBoxStrategy::distance_ps_strategy::type()
                                            .apply(bottom_left, p0, p1));

            strategy::distance::services::swap_result_points
               <
                  SegmentBoxStrategy
               >::apply(result);
        }
        else
        {
            result = geometry::detail::distance::segment_to_box_2D
                <
                    ReturnType,
                    SegmentPoint,
                    BoxPoint,
                    SegmentBoxStrategy
                >::template call_above_of_box
                    <
                        typename LessEqual::other
                    >(p1, p0, p_max, bottom_right, sb_strategy);
        }
        return result;
    }

    template <typename SPoint, typename BPoint>
    static bool mirror(SPoint& p0,
                       SPoint& p1,
                       BPoint& bottom_left,
                       BPoint& bottom_right,
                       BPoint& top_left,
                       BPoint& top_right)
    {
        //if segment's vertex is the southest point then mirror geometries
        if (geometry::get<1>(p0) + geometry::get<1>(p1) < 0)
        {
            BPoint bl = bottom_left;
            BPoint br = bottom_right;
            geometry::set<1>(p0, geometry::get<1>(p0) * -1);
            geometry::set<1>(p1, geometry::get<1>(p1) * -1);
            geometry::set<1>(bottom_left, geometry::get<1>(top_left) * -1);
            geometry::set<1>(top_left, geometry::get<1>(bl) * -1);
            geometry::set<1>(bottom_right, geometry::get<1>(top_right) * -1);
            geometry::set<1>(top_right, geometry::get<1>(br) * -1);
            return true;
        }
        return false;
    }
};

}} // namespace strategy::distance

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_GENERIC_SEGMENT_BOX_HPP
