// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_MIRROR_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_MIRROR_BOX_HPP

namespace boost { namespace geometry
{


namespace strategy { namespace mirror_box
{

struct geographic
{
    template <typename SPoint, typename BPoint>
    static void apply(SPoint& p0,
                      SPoint& p1,
                      BPoint& bottom_left,
                      BPoint& bottom_right,
                      BPoint& top_left,
                      BPoint& top_right)
    {
        return spherical_or_geographic::apply(p0, p1,
                                              bottom_left,
                                              bottom_right,
                                              top_left,
                                              top_right);
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<geographic_tag>
{
    typedef geographic type;
};

}
#endif

}} // namespace strategy::mirror_box

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_MIRROR_BOX_HPP
