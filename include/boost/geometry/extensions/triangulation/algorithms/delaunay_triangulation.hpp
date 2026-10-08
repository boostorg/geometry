// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_ALGORITHMS_DELAUNAY_TRIANGULATION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_ALGORITHMS_DELAUNAY_TRIANGULATION_HPP

#include <vector>
#include <utility>
#include <tuple>
#include <iterator>
#include <algorithm>

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/extensions/triangulation/geometries/triangulation.hpp>
#include <boost/geometry/extensions/triangulation/strategies/cartesian/in_circle_robust.hpp>
#include <boost/geometry/extensions/triangulation/strategies/delaunay_triangulation.hpp>
#include <boost/geometry/extensions/triangulation/strategies/cartesian/accelerated_shull.hpp>
#include <boost/geometry/strategies/cartesian/side_by_triangle.hpp>

namespace boost { namespace geometry
{

template
<
    typename PointContainer,
    typename Triangulation,
    typename SideStrategy = strategy::side::side_by_triangle<>,
    typename InCircleStrategy = strategy::in_circle::in_circle_robust<>,
    typename ConstructionStrategy =
        typename strategy::delaunay_triangulation::services::default_strategy
            <
                typename tag<typename PointContainer::value_type>::type,
                typename cs_tag<typename PointContainer::value_type>::type,
                dimension<typename PointContainer::value_type>::value
            >::type
>
inline void delaunay_triangulation(PointContainer const & in,
                                   Triangulation& out,
                                   bool legalize = true)
{
    ConstructionStrategy::template apply
        <
            PointContainer,
            Triangulation,
            SideStrategy,
            InCircleStrategy
        >(in, out, legalize);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_ALGORITHMS_DELAUNAY_TRIANGULATION_HPP
