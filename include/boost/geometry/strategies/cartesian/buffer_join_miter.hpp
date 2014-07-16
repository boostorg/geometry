// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_MITER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_MITER_HPP

#include <boost/assert.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/geometry/strategies/buffer.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

// TODO: condider merging/baseclassing join strategies to avoid duplicate code

struct join_miter
{

    // Constructor compatible with other join strategies:
    inline join_miter(int = 0)
    {}

    template <typename Point, typename DistanceType, typename RangeOut>
    inline bool apply(Point const& ip, Point const& vertex,
                Point const& perp1, Point const& perp2,
                DistanceType const& buffer_distance,
                RangeOut& range_out) const
    {
        geometry::equal_to<Point> equals;
        if (equals(ip, vertex))
        {
            return false;
        }

        typedef typename coordinate_type<Point>::type coordinate_type;
        typedef typename geometry::select_most_precise
        <
            coordinate_type,
            double
        >::type promoted_type;

        Point p = ip;

        // Normalize it and give it X*dist.
        coordinate_type const dx = get<0>(ip) - get<0>(vertex);
        coordinate_type const dy = get<1>(ip) - get<1>(vertex);

        promoted_type const length = geometry::math::sqrt(dx * dx + dy * dy);

        BOOST_ASSERT(length != 0.0); // because ip/vertex are not equal

        // TODO: make max-mitre-limit flexible
        promoted_type const ten = 10.0;
        promoted_type const zero_seven = 0.7;

        promoted_type const max = ten * geometry::math::abs(buffer_distance);

        if (length > max)
        {
            promoted_type const prop = zero_seven * buffer_distance / length;
            set<0>(p, get<0>(vertex) + dx * prop);
            set<1>(p, get<1>(vertex) + dy * prop);
        }

        range_out.push_back(perp1);
        range_out.push_back(p);
        range_out.push_back(perp2);
        return true;
    }
};



}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_MITER_HPP
