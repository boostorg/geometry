// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_BOX_HPP

#include <boost/geometry/core/access.hpp>

namespace boost { namespace geometry { namespace strategy { namespace buffer
{

namespace cartesian_buffer_detail
{

// Comparison with the unexpanded box between two represented points.
// Return -1 outside, 0 on the boundary, +1 strictly inside.
struct cartesian_buffer_box
{
    template <typename T, typename U>
    static int locate_axis(T const& p, U const& a, U const& b)
    {
        if (p == a || p == b)
        {
            return 0;
        }
        return (a < p && p < b) || (b < p && p < a) ? 1 : -1;
    }

    template <typename Point, typename Corner>
    static int locate(Point const& p, Corner const& a, Corner const& b)
    {
        int const x = locate_axis(get<0>(p), get<0>(a), get<0>(b));
        int const y = locate_axis(get<1>(p), get<1>(a), get<1>(b));
        return x < y ? x : y;
    }

    template <typename Iterator>
    static bool is_rectangle(Iterator first, Iterator last)
    {
        if (last - first != 5)
        {
            return false;
        }
        auto const& a = first[0];
        auto const& b = first[1];
        auto const& c = first[2];
        auto const& d = first[3];
        auto const& e = first[4];
        return get<0>(a) == get<0>(e) && get<1>(a) == get<1>(e)
            && get<0>(a) != get<0>(c) && get<1>(a) != get<1>(c)
            && ((get<0>(b) == get<0>(a) && get<1>(b) == get<1>(c)
              && get<0>(d) == get<0>(c) && get<1>(d) == get<1>(a))
             || (get<0>(d) == get<0>(a) && get<1>(d) == get<1>(c)
              && get<0>(b) == get<0>(c) && get<1>(b) == get<1>(a)));
    }
};

} // namespace cartesian_buffer_detail

}}}} // namespace boost::geometry::strategy::buffer

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_BOX_HPP
