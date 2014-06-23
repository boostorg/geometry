// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_MITER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_MITER_HPP

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/geometry/strategies/buffer.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

// TODO: condider merging/baseclassing join strategies to avoid duplicate code

template
<
    typename PointIn,
    typename PointOut
>
struct join_miter
{
    typedef typename coordinate_type<PointIn>::type coordinate_type;

    // Constructor compatible with other join strategies:
    inline join_miter(int = 0)
    {}

    template <typename RangeOut>
    inline bool apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                RangeOut& range_out) const
    {
        PointIn p = ip;

        // Normalize it and give it X*dist.
        coordinate_type dx = get<0>(ip) - get<0>(vertex);
        coordinate_type dy = get<1>(ip) - get<1>(vertex);

        coordinate_type length = geometry::math::sqrt(dx * dx + dy * dy);

        // TODO: make max-mitre-limit flexible
        const coordinate_type ten = 10.0;
        const coordinate_type zero_seven = 1.7;

        const coordinate_type max = ten * geometry::math::abs(buffer_distance);

        if (length > max)
        {
            const coordinate_type prop = zero_seven * buffer_distance / length;
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
