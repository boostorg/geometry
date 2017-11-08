// Boost.Geometry

// Copyright (c) 2014-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_DISTANCE_CROSS_TRACK_POINT_BOX_HPP

#include <boost/geometry/formulas/result_direct.hpp>

namespace boost { namespace geometry {

namespace strategy { namespace distance

{

template <typename ReturnType>
class cross_track_point_box_generic
{
public :

    template
    <
            typename Point,
            typename Box,
            typename Strategy
    >
    ReturnType static inline apply (Point const& point,
                                    Box const& box,
                                    Strategy ps_strategy)
    {
        // this method assumes that the coordinates of the point and
        // the box are normalized

        typedef typename point_type<Box>::type box_point_type;

        // TODO: This strategy as well as other cross-track strategies
        // and therefore e.g. spherical within(Point, Box) may not work
        // properly for a Box degenerated to a Segment or Point

        box_point_type bottom_left, bottom_right, top_left, top_right;
        geometry::detail::assign_box_corners(box,
                                             bottom_left, bottom_right,
                                             top_left, top_right);

        ReturnType const plon = geometry::get_as_radian<0>(point);
        ReturnType const plat = geometry::get_as_radian<1>(point);

        ReturnType const lon_min = geometry::get_as_radian<0>(bottom_left);
        ReturnType const lat_min = geometry::get_as_radian<1>(bottom_left);
        ReturnType const lon_max = geometry::get_as_radian<0>(top_right);
        ReturnType const lat_max = geometry::get_as_radian<1>(top_right);

        ReturnType const pi = math::pi<ReturnType>();
        ReturnType const two_pi = math::two_pi<ReturnType>();

        typedef typename point_type<Box>::type box_point_type;

        // First check if the point is within the band defined by the
        // minimum and maximum longitude of the box; if yes, determine
        // if the point is above, below or inside the box and compute
        // the distance (easy in this case)
        //
        // Notice that the point may not be inside the longitude range
        // of the box, but the shifted point may be inside the
        // longitude range of the box; in this case the point is still
        // considered as inside the longitude range band of the box
        if ((plon >= lon_min && plon <= lon_max) || plon + two_pi <= lon_max)
        {
            if (plat > lat_max)
            {
                //std::cout << "plat=" << plat << std::endl;
                //std::cout << "lat_max=" << lat_max << std::endl;
                //std::cout << "radius=" << ps_strategy.get_distance_strategy().radius() << std::endl;
                //std::cout << "r1=" << ps_strategy.get_distance_strategy().radius() * (plat - lat_max) << std::endl;
                //std::cout << "r2=" << ps_strategy.get_distance_strategy().meridian(plat, lat_max) << std::endl;
                return geometry::strategy::distance::services::result_from_distance
                        <
                        Strategy, Point, box_point_type
                        //>::apply(ps_strategy, ps_strategy.get_distance_strategy().radius() * (plat - lat_max));
                        >::apply(ps_strategy, ps_strategy.get_distance_strategy().meridian(plat, lat_max));
                //get_strategy(ps_strategy) --> haversine
                //haversine special case for vertical distance
            }
            else if (plat < lat_min)
            {
                return geometry::strategy::distance::services::result_from_distance
                        <
                        Strategy, Point, box_point_type
                        //>::apply(ps_strategy, ps_strategy.radius() * (lat_min - plat));
                        >::apply(ps_strategy, ps_strategy.get_distance_strategy().meridian(lat_min, plat));
            }
            else
            {
                BOOST_GEOMETRY_ASSERT(plat >= lat_min && plat <= lat_max);
                return ReturnType(0);
            }
        }

        // Otherwise determine which among the two medirian segments of the
        // box the point is closest to, and compute the distance of
        // the point to this closest segment

        // Below lon_midway is the longitude of the meridian that:
        // (1) is midway between the meridians of the left and right
        //     meridians of the box, and
        // (2) does not intersect the box
        ReturnType const two = 2.0;
        bool use_left_segment;
        if (lon_max > pi)
        {
            // the box crosses the antimeridian

            // midway longitude = lon_min - (lon_min + (lon_max - 2 * pi)) / 2;
            ReturnType const lon_midway = (lon_min - lon_max) / two + pi;
            BOOST_GEOMETRY_ASSERT(lon_midway >= -pi && lon_midway <= pi);

            use_left_segment = plon > lon_midway;
        }
        else
        {
            // the box does not cross the antimeridian

            ReturnType const lon_sum = lon_min + lon_max;
            if (math::equals(lon_sum, ReturnType(0)))
            {
                // special case: the box is symmetric with respect to
                // the prime meridian; the midway meridian is the antimeridian

                use_left_segment = plon < lon_min;
            }
            else
            {
                // midway long. = lon_min - (2 * pi - (lon_max - lon_min)) / 2;
                ReturnType lon_midway = (lon_min + lon_max) / two - pi;

                // normalize the midway longitude
                if (lon_midway > pi)
                {
                    lon_midway -= two_pi;
                }
                else if (lon_midway < -pi)
                {
                    lon_midway += two_pi;
                }
                BOOST_GEOMETRY_ASSERT(lon_midway >= -pi && lon_midway <= pi);

                // if lon_sum is positive the midway meridian is left
                // of the box, or right of the box otherwise
                use_left_segment = lon_sum > 0
                        ? (plon < lon_min && plon >= lon_midway)
                        : (plon <= lon_max || plon > lon_midway);
            }
        }

        return use_left_segment
                ? ps_strategy.apply(point, bottom_left, top_left)
                : ps_strategy.apply(point, bottom_right, top_right);
    }
};

}}}} // namespace boost::geometry::strategy::distance

#endif // BOOST_GEOMETRY_STRATEGIES_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
