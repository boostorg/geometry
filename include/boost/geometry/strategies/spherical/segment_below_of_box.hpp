// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SEGMENT_BELOW_OF_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SEGMENT_BELOW_OF_BOX_HPP

#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/azimuth.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace segment_below_of_box
{

template <typename LessEqual, typename ReturnType, typename PPStrategy>
struct spherical_or_geographic
{
    template <typename SegmentPoint, typename BoxPoint, typename PSStrategy>
    static inline ReturnType apply(SegmentPoint const& p0,
                                   SegmentPoint const& p1,
                                   BoxPoint const&,
                                   BoxPoint const& top_right,
                                   BoxPoint const& bottom_left,
                                   BoxPoint const& bottom_right,
                                   PSStrategy const& ps_strategy)
    {
        ReturnType result;
        typename LessEqual::other less_equal;

        typedef geometry::model::segment<SegmentPoint> Segment;
        Segment seg(p0, p1);

        geometry::model::box<BoxPoint> input_box;
        geometry::set_from_radian<geometry::min_corner, 0>
                (input_box, geometry::get_as_radian<0>(bottom_left));
        geometry::set_from_radian<geometry::min_corner, 1>
                (input_box, geometry::get_as_radian<1>(bottom_left));
        geometry::set_from_radian<geometry::max_corner, 0>
                (input_box, geometry::get_as_radian<0>(top_right));
        geometry::set_from_radian<geometry::max_corner, 1>
                (input_box, geometry::get_as_radian<1>(top_right));

        typedef typename cs_tag<Segment>::type segment_cs_type;
        SegmentPoint p_max;

        typedef geometry::detail::disjoint::
           disjoint_segment_box_sphere_or_spheroid<segment_cs_type> disjoint_sb;
        typedef typename disjoint_sb::disjoint_info disjoint_info_type;

        disjoint_info_type disjoint_result = disjoint_sb::
                apply(seg, input_box, ps_strategy.get_azimuth_strategy(), p_max);

        if (disjoint_result == disjoint_info_type::intersect) //intersect
        {
            return 0;
        }
        // disjoint but vertex not computed
        if (disjoint_result == disjoint_info_type::disjoint_no_vertex)
        {
            typedef typename coordinate_type<SegmentPoint>::type CT;
            geometry::model::box<SegmentPoint> mbr;

            geometry::envelope(seg, mbr, ps_strategy.get_envelope_segment_strategy());

            CT lon1 = geometry::get_as_radian<0>(p0);
            CT lat1 = geometry::get_as_radian<1>(p0);
            CT lon2 = geometry::get_as_radian<0>(p1);
            CT lat2 = geometry::get_as_radian<1>(p1);

            CT vertex_lat;
            CT lat_sum = lat1 + lat2;
            if (lat_sum > CT(0))
            {
                vertex_lat = geometry::get_as_radian<geometry::max_corner, 1>(mbr);
            } else {
                vertex_lat = geometry::get_as_radian<geometry::min_corner, 1>(mbr);
            }

            CT alp1;
            ps_strategy.get_azimuth_strategy().apply(lon1, lat1, lon2, lat2, alp1);
            CT vertex_lon = geometry::formula::vertex_longitude<CT, segment_cs_type>
                            ::apply(lon1, lat1, lon2, lat2,
                                    vertex_lat, alp1,
                                    ps_strategy.get_azimuth_strategy());

            geometry::set_from_radian<0>(p_max, vertex_lon);
            geometry::set_from_radian<1>(p_max, vertex_lat);
        }
        //otherwise disjoint and vertex computed inside disjoint

        if (less_equal(geometry::get_as_radian<0>(bottom_left),
                       geometry::get_as_radian<0>(p_max)))
        {
            result = boost::numeric_cast<ReturnType>(ps_strategy.apply(bottom_left,
                                                                       p0, p1));
        }
        else
        {
            result = geometry::detail::distance::segment_to_box_2D
                    <
                        ReturnType,
                        SegmentPoint,
                        BoxPoint,
                        PPStrategy,
                        PSStrategy
                    >::template call_above_of_box
                    <
                        typename LessEqual::other
                    >(p1, p0, p_max, bottom_right, ps_strategy);
        }
        return result;
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename LessEqual, typename ReturnType, typename PPStrategy>
struct default_strategy<LessEqual, ReturnType, PPStrategy, spherical_equatorial_tag>
{
    typedef spherical_or_geographic<LessEqual, ReturnType, PPStrategy> type;
};

}
#endif


}} // namespace strategy::segment_below_of_box

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SEGMENT_BELOW_OF_BOX_HPP
