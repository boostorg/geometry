// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINE_LINE_INTERSECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINE_LINE_INTERSECTION_HPP


#include <boost/geometry/util/math.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


// TODO: once change this to proper strategy
// It is different from current segment intersection because these are not segments but lines
// If we have the Line concept, we can create a strategy
struct line_line_intersection
{
    template <typename A, typename B, typename C, typename D>
    static inline A det(A const& a, B const& b, C const& c, D const& d)
    {
        return a * d - b * c;
    }

    template <typename Point>
    static inline bool apply(Point const& pi, Point const& pj,
        Point const& qi, Point const& qj, Point& ip)
    {
        // See http://mathworld.wolfram.com/Line-LineIntersection.html
        typedef typename coordinate_type<Point>::type coordinate_type;

        coordinate_type denominator = det(get<0>(pi) - get<0>(pj), get<1>(pi) - get<1>(pj), get<0>(qi) - get<0>(qj), get<1>(qi) - get<1>(qj));

        // TODO: code below will be removed - curve type is now checked before

        // The limit is arbitrary. If it is small, the IP will be far far away.
        // For round joins, it will not be used at all.
        // For miter joints, there is a miter limit
        // If segments are parallel we must be distinguish two cases
        coordinate_type const limit = 1.0e-9;
        if (geometry::math::abs(denominator) < limit)
        {
            denominator = limit;
        }
        // END TODO

        coordinate_type d1 = det(get<0>(pi), get<1>(pi), get<0>(pj), get<1>(pj));
        coordinate_type d2 = det(get<0>(qi), get<1>(qi), get<0>(qj), get<1>(qj));

        set<0>(ip, det(d1, get<0>(pi) - get<0>(pj), d2, get<0>(qi) - get<0>(qj)) / denominator);
        set<1>(ip, det(d1, get<1>(pi) - get<1>(pj), d2, get<1>(qi) - get<1>(qj)) / denominator);

        return true;
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINE_LINE_INTERSECTION_HPP
