// Boost.Geometry

// Copyright (c) 2016-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_DISTANCE_CROSS_TRACK_BOX_BOX_HPP

namespace boost { namespace geometry {

namespace strategy { namespace distance

{

template <typename ReturnType>
class cross_track_box_box_generic
{
public :

    template
    <
            typename Box1,
            typename Box2,
            typename Strategy
    >
    ReturnType static inline apply (Box1 const& box1,
                                    Box2 const& box2,
                                    Strategy ps_strategy)
    {

        // this method assumes that the coordinates of the point and
        // the box are normalized

        typedef typename point_type<Box1>::type box_point_type1;
        typedef typename point_type<Box2>::type box_point_type2;

        box_point_type1 bottom_left1, bottom_right1, top_left1, top_right1;
        geometry::detail::assign_box_corners(box1,
                                             bottom_left1, bottom_right1,
                                             top_left1, top_right1);

        box_point_type2 bottom_left2, bottom_right2, top_left2, top_right2;
        geometry::detail::assign_box_corners(box2,
                                             bottom_left2, bottom_right2,
                                             top_left2, top_right2);

        ReturnType lon_min1 = geometry::get_as_radian<0>(bottom_left1);
        ReturnType const lat_min1 = geometry::get_as_radian<1>(bottom_left1);
        ReturnType lon_max1 = geometry::get_as_radian<0>(top_right1);
        ReturnType const lat_max1 = geometry::get_as_radian<1>(top_right1);

        ReturnType lon_min2 = geometry::get_as_radian<0>(bottom_left2);
        ReturnType const lat_min2 = geometry::get_as_radian<1>(bottom_left2);
        ReturnType lon_max2 = geometry::get_as_radian<0>(top_right2);
        ReturnType const lat_max2 = geometry::get_as_radian<1>(top_right2);

        ReturnType const two_pi = math::two_pi<ReturnType>();

        // Test which sides of the boxes are closer and if boxes cross
        // antimeridian
        bool right_wrap;

        if (lon_min2 > 0 && lon_max2 < 0) // box2 crosses antimeridian
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(box2 crosses antimeridian)";
#endif
            right_wrap = lon_min2 - lon_max1 < lon_min1 - lon_max2;
            lon_max2 += two_pi;
            if (lon_min1 > 0 && lon_max1 < 0) // both boxes crosses antimeridian
            {
                lon_max1 += two_pi;
            }
        }
        else if (lon_min1 > 0 && lon_max1 < 0) // only box1 crosses antimeridian
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(box1 crosses antimeridian)";
#endif
            return apply(box2, box1, ps_strategy);
        }
        else
        {
            right_wrap = lon_max1 <= lon_min2
                         ? lon_min2 - lon_max1 < two_pi - (lon_max2 - lon_min1)
                         : lon_min1 - lon_max2 > two_pi - (lon_max1 - lon_min2);

        }

        // Check1: if box2 crosses the band defined by the
        // minimum and maximum longitude of box1; if yes, determine
        // if the box2 is above, below or intersects/is inside box1 and compute
        // the distance (easy in this case)

        bool lon_min12 = lon_min1 <= lon_min2;
        bool right = lon_max1 <= lon_min2;
        bool left = lon_min1 >= lon_max2;
        bool lon_max12 = lon_max1 <= lon_max2;

        if ((lon_min12 && !right)
                || (!left && !lon_max12)
                || (!lon_min12 && lon_max12))
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(up-down)\n";
#endif
            if (lat_min1 > lat_max2)
            {
                return geometry::strategy::distance::services::result_from_distance
                    <
                        Strategy, box_point_type1, box_point_type2
                    >::apply(ps_strategy, ps_strategy.get_distance_strategy()
                               .meridian(lat_min1, lat_max2));
            }
            else if (lat_max1 < lat_min2)
            {
                return geometry::strategy::distance::services::result_from_distance
                    <
                        Strategy, box_point_type1, box_point_type2
                    >::apply(ps_strategy, ps_strategy.get_distance_strategy().
                             meridian(lat_min2, lat_max1));
            }
            else
            {
                //BOOST_GEOMETRY_ASSERT(plat >= lat_min && plat <= lat_max);
                return ReturnType(0);
            }
        }

        // Check2: if box2 is right/left of box1
        // the max lat of box2 should be less than the max lat of box1
        bool bottom_max;

        ReturnType top_common = std::min(lat_max1, lat_max2);
        ReturnType bottom_common = std::max(lat_min1, lat_min2);

        // true if the closest points are on northern hemisphere
        bool north_shortest = std::abs(top_common) > std::abs(bottom_common)
                || lat_max1 <= lat_min2
                || lat_min1 >= lat_max2;

        if (north_shortest)
        {
            bottom_max = lat_max1 >= lat_max2;
        }
        else
        {
            bottom_max = lat_min1 <= lat_min2;
        }

#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
        std::cout << "(diagonal)";
#endif
        if (bottom_max && !right_wrap)
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(bottom left)";
#endif
            if (north_shortest)
            {
                return ps_strategy.apply(top_right2, top_left1, bottom_left1);
            }
            return ps_strategy.apply(bottom_right2, top_left1, bottom_left1);
        }
        if (bottom_max && right_wrap)
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(bottom right)";
#endif
            if (north_shortest)
            {
                return ps_strategy.apply(top_left2, top_right1, bottom_right1);
            }
            return ps_strategy.apply(bottom_left2, top_right1, bottom_right1);
        }
        if (!bottom_max && !right_wrap)
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(top left)";
#endif
            if (north_shortest)
            {
                return ps_strategy.apply(top_left1, top_right2, bottom_right2);
            }
            return ps_strategy.apply(bottom_left1, top_right2, bottom_right2);
        }
        if (!bottom_max && right_wrap)
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK_BOX_BOX
            std::cout << "(top right)";
#endif
            if (north_shortest)
            {
                return ps_strategy.apply(top_right1, top_left2, bottom_left2);
            }
            return ps_strategy.apply(bottom_right1, top_left2, bottom_left2);
        }
        return ReturnType(0);
    }
};

}}}} // namespace boost::geometry::strategy::distance

#endif // BOOST_GEOMETRY_STRATEGIES_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
