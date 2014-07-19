// Boost.Geometry (aka GGL, Generic Geometry Library)
// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_SIDE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_SIDE_HPP

#include <cstddef>

#include <boost/assert.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/side.hpp>



namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{


struct buffer_side
{

    template
    <
        typename Point,
        typename OutputRange,
        typename DistanceStrategy
    >
    static inline void apply(
                Point const& input_p1, Point const& input_p2,
                strategy::buffer::buffer_side_selector side,
                DistanceStrategy const& distance,
                OutputRange& output_range)
    {
        typedef typename coordinate_type<Point>::type coordinate_type;
        // Generate a block along (left or right of) the segment

        // Simulate a vector d (dx,dy)
        coordinate_type dx = get<0>(input_p2) - get<0>(input_p1);
        coordinate_type dy = get<1>(input_p2) - get<1>(input_p1);

        // For normalization [0,1] (=dot product d.d, sqrt)
        // TODO promoted_type
        coordinate_type const length = geometry::math::sqrt(dx * dx + dy * dy);

        // Because coordinates are not equal, length should not be zero
        BOOST_ASSERT((! geometry::math::equals(length, 0)));

        // Generate the normalized perpendicular p, to the left (ccw)
        coordinate_type const px = -dy / length;
        coordinate_type const py = dx / length;

        coordinate_type const d = distance.apply(input_p1, input_p2, side);

        output_range.resize(2);

        set<0>(output_range.front(), get<0>(input_p1) + px * d);
        set<1>(output_range.front(), get<1>(input_p1) + py * d);
        set<0>(output_range.back(), get<0>(input_p2) + px * d);
        set<1>(output_range.back(), get<1>(input_p2) + py * d);
    }
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_SIDE_HPP
