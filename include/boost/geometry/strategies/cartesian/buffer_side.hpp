// Boost.Geometry (aka GGL, Generic Geometry Library)
// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_SIDE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_SIDE_HPP

#include <cstddef>
#include <iterator>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/algorithms/detail/buffer/buffered_piece_collection.hpp>
#include <boost/geometry/algorithms/detail/buffer/line_line_intersection.hpp>
#include <boost/geometry/algorithms/detail/buffer/parallel_continue.hpp>

#include <boost/geometry/algorithms/simplify.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{


struct buffer_side
{

    template
    <
        typename Point,
        typename OutputPointType,
        typename DistanceStrategy
    >
    static inline void apply(
                Point const& input_p1, Point const& input_p2,
                strategy::buffer::buffer_side_selector side,
                DistanceStrategy const& distance,
                // TODO: the output will be a range
                OutputPointType& side_p1, OutputPointType& side_p2)
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

        set<0>(side_p1, get<0>(input_p1) + px * d);
        set<1>(side_p1, get<1>(input_p1) + py * d);
        set<0>(side_p2, get<0>(input_p2) + px * d);
        set<1>(side_p2, get<1>(input_p2) + py * d);
    }
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_SIDE_HPP
