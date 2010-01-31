// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_STRATEGY_CENTROID_HPP
#define BOOST_GEOMETRY_MULTI_STRATEGY_CENTROID_HPP

#include <boost/geometry/strategies/cartesian/centroid_bashein_detmer.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


template <typename Point, typename Geometry>
struct strategy_centroid<cartesian_tag, multi_polygon_tag, 2, Point, Geometry>
{
    typedef strategy::centroid_::bashein_detmer
        <
            Point,
            typename point_type<Geometry>::type
        > type;
};

#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_STRATEGY_CENTROID_HPP
