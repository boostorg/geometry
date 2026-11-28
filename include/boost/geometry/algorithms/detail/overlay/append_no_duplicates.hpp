// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2018-2020.
// Modifications copyright (c) 2018-2020 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPLICATES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPLICATES_HPP


#include <type_traits>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>

#include <boost/geometry/util/range.hpp>
#include <boost/geometry/strategies/detail/point_conversion.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template <typename Strategy, typename Range, typename Point>
inline bool try_append_no_duplicates(Strategy const& strategy, Range& range, Point const& point)
{
    auto const hook = [&](auto const& s) -> decltype(s.append_no_duplicates(range, point), void())
    { s.append_no_duplicates(range, point); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
    {
        hook(strategy);
        return true;
    }
    return false;
}

template <typename Range, typename Point>
inline void append_with_duplicates(Range& range, Point const& point)
{
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    std::cout << "  add: ("
        << geometry::get<0>(point) << ", " << geometry::get<1>(point) << ")"
        << std::endl;
#endif
    geometry::append(range, point);
}

template <typename Range, typename Point, typename Strategy>
inline void append_no_duplicates(Range& range, Point const& point,
                                 Strategy const& strategy)
{
    if (try_append_no_duplicates(strategy, range, point))
    {
        return;
    }

    if ( boost::empty(range)
      || ! geometry::detail::equals::equals_point_point(geometry::range::back(range),
                                                        point,
                                                        strategy) )
    {
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
        std::cout << "  add: ("
            << geometry::get<0>(point) << ", " << geometry::get<1>(point) << ")"
            << std::endl;
#endif
        typename boost::range_value<Range>::type converted;
        strategies::detail::convert_geometry(strategy, point, converted);
        traits::push_back<Range>::apply(range, std::move(converted));
    }
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPEND_NO_DUPLICATES_HPP
