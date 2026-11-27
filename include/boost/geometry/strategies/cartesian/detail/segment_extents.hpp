// Boost.Geometry

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_SEGMENT_EXTENTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_SEGMENT_EXTENTS_HPP

#include <algorithm>
#include <type_traits>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

template <typename P, typename Q>
using native_double_pair = std::integral_constant<bool,
    std::is_same<coordinate_type_t<P>, double>::value
    && std::is_same<coordinate_type_t<Q>, double>::value>;

template <int D, typename P, typename Q>
int segment_extent_overlap(P const& a, P const& b, Q const& c, Q const& d)
{
    auto const first = std::minmax({geometry::get<D>(a), geometry::get<D>(b)});
    auto const second = std::minmax({geometry::get<D>(c), geometry::get<D>(d)});
    auto const lo = (std::max)(first.first, second.first);
    auto const hi = (std::min)(first.second, second.second);
    return hi < lo ? -1 : hi > lo ? 1 : 0;
}

template <typename P, typename Q>
bool segment_y_disjoint(P const& a, P const& b, Q const& c, Q const& d)
{
    return segment_extent_overlap<1>(a, b, c, d) < 0;
}

}}}}
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_SEGMENT_EXTENTS_HPP
