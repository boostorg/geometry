// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_PT_PT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_PT_PT_HPP


#include <boost/geometry/strategies/closest_points/services.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

class dummy_pt_pt {};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct tag<dummy_pt_pt>
{
    typedef strategy_tag_distance_point_point type;
};


template <typename Point1, typename Point2>
struct default_strategy
    <
        point_tag, point_tag, Point1, Point2, cartesian_tag, cartesian_tag
    >
{
    typedef dummy_pt_pt type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::closest_points


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_PT_PT_HPP
