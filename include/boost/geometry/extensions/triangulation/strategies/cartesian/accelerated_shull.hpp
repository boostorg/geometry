// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_ACCELERATED_SHULL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_ACCELERATED_SHULL_HPP

#include <boost/geometry/extensions/triangulation/strategies/delaunay_triangulation.hpp>
#include <boost/geometry/extensions/triangulation/strategies/cartesian/detail/accelerated_shull.hpp>
#include <boost/geometry/extensions/triangulation/strategies/delaunay_triangulation.hpp>
#include <boost/geometry/extensions/triangulation/strategies/cartesian/in_circle_robust.hpp>
#include <boost/geometry/strategies/cartesian/side_by_triangle.hpp>

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

namespace boost { namespace geometry { namespace strategy
{ 

namespace delaunay_triangulation
{

struct accelerated_shull
{
    template
    <
        typename PointContainer,
        typename Triangulation,
        typename SideStrategy = strategy::side::side_by_triangle<>,
        typename InCircleStrategy = strategy::in_circle::in_circle_robust<>
    >
    static inline void apply(PointContainer const & in,
                             Triangulation& out,
                             bool legalize = true)
    {
        detail::accelerated_shull::apply<
            PointContainer,
            Triangulation,
            SideStrategy,
            InCircleStrategy>(in, out, legalize);
    }
};

namespace services
{
template <>
struct default_strategy<point_tag, cartesian_tag, 2>
{
    typedef accelerated_shull type;
};
}

}}}} // namespace boost::geometry::strategy::delaunay:triangulation

#endif // BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_ACCELERATED_SHULL_HPP
