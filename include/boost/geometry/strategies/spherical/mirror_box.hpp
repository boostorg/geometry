// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_MIRROR_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_MIRROR_BOX_HPP

namespace boost { namespace geometry
{


namespace strategy { namespace mirror_box
{

struct spherical_or_geographic
{
    template <typename SPoint, typename BPoint>
    static void apply(SPoint& p0,
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
        }
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<spherical_equatorial_tag>
{
    typedef spherical_or_geographic type;
};

}
#endif

}} // namespace strategy::mirror_box

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_MIRROR_BOX_HPP
