// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_UNION_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_UNION_HPP


#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/multi/core/closure.hpp>
#include <boost/geometry/multi/core/geometry_id.hpp>
#include <boost/geometry/multi/core/is_areal.hpp>
#include <boost/geometry/multi/core/point_order.hpp>
#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/within.hpp>
#include <boost/geometry/multi/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/multi/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/multi/algorithms/detail/overlay/copy_segments.hpp>
#include <boost/geometry/multi/algorithms/detail/overlay/copy_segment_point.hpp>
#include <boost/geometry/multi/algorithms/detail/overlay/select_rings.hpp>
#include <boost/geometry/multi/algorithms/detail/sections/range_by_section.hpp>
#include <boost/geometry/multi/algorithms/detail/sections/sectionalize.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// Currently empty, all done in overlay / assemble



} // namespace dispatch
#endif

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_UNION_HPP

