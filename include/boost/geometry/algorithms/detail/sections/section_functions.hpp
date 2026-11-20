// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015-2024.
// Modifications copyright (c) 2015-2024, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_FUNCTIONS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_FUNCTIONS_HPP

#include <boost/geometry/strategies/detail/section.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace section
{

// Compatibility for callers that do not yet supply an umbrella strategy.
template
<
    std::size_t Dimension,
    typename Geometry,
    typename CastedCSTag = tag_cast_t<cs_tag_t<Geometry>, spherical_tag>
>
using preceding_check = strategies::detail::section_detail::preceding_check
    <Dimension, Geometry, CastedCSTag>;


template
<
    std::size_t Dimension,
    typename Point,
    typename Box
>
inline bool preceding(int dir,
                      Point const& point,
                      Box const& point_box,
                      Box const& other_box)
{
    return preceding_check<Dimension, Box>::apply(dir, point, point_box, other_box);
}

template
<
    std::size_t Dimension,
    typename Point,
    typename Box
>
inline bool exceeding(int dir,
                      Point const& point,
                      Box const& point_box,
                      Box const& other_box)
{
    return preceding<Dimension>(-dir, point, point_box, other_box);
}

template <std::size_t Dimension, typename Point, typename Box, typename Strategy>
inline bool preceding(int dir, Point const& point, Box const& point_box,
                      Box const& other_box, Strategy const& strategy)
{
    return strategies::detail::get_section(strategy).template preceding<Dimension>
        (dir, point, point_box, other_box);
}

template <std::size_t Dimension, typename Point, typename Box, typename Strategy>
inline bool exceeding(int dir, Point const& point, Box const& point_box,
                      Box const& other_box, Strategy const& strategy)
{
    return preceding<Dimension>(-dir, point, point_box, other_box, strategy);
}


}} // namespace detail::section
#endif


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_FUNCTIONS_HPP
