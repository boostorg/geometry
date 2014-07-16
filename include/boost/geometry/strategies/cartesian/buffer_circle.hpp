// Boost.Geometry (aka GGL, Generic Geometry Library)
// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_CIRCLE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_CIRCLE_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/buffer.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

// Strategy to create a buffer around a point using a specified amount of points
class buffer_circle
{
public :
    explicit buffer_circle(std::size_t count)
        : m_count(count)
    {}

    template
    <
        typename Point,
        typename OutputRange,
        typename DistanceStrategy
    >
    inline void apply(Point const& point,
                DistanceStrategy const& distance_strategy,
                OutputRange& output_range) const
    {
        typedef typename boost::range_value<OutputRange>::type output_point_type;

        typedef typename geometry::select_most_precise
            <
                typename geometry::select_most_precise
                    <
                        typename geometry::coordinate_type<Point>::type,
                        typename geometry::coordinate_type<output_point_type>::type
                    >::type,
                double
            >::type promoted_type;

        promoted_type const buffer_distance = distance_strategy.apply(point, point,
                        strategy::buffer::buffer_side_left);

        promoted_type const two = 2.0;
        promoted_type const two_pi = two * geometry::math::pi<promoted_type>();

        promoted_type const diff = two_pi / promoted_type(m_count);
        promoted_type a = 0;

        for (std::size_t i = 0; i < m_count; i++, a -= diff)
        {
            output_point_type p;
            set<0>(p, get<0>(point) + buffer_distance * cos(a));
            set<1>(p, get<1>(point) + buffer_distance * sin(a));
            output_range.push_back(p);
        }

        // Close it:
        output_range.push_back(output_range.front());
    }

private :
    std::size_t m_count;
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_CIRCLE_HPP
