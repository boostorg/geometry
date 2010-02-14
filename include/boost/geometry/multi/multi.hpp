// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Copyright Mateusz Loskot 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_HPP
#define BOOST_GEOMETRY_MULTI_HPP


#include <boost/geometry/multi/core/geometry_id.hpp>
#include <boost/geometry/multi/core/is_multi.hpp>
#include <boost/geometry/multi/core/interior_rings.hpp>
#include <boost/geometry/multi/core/point_type.hpp>
#include <boost/geometry/multi/core/ring_type.hpp>
#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/core/topological_dimension.hpp>


#include <boost/geometry/multi/algorithms/area.hpp>
#include <boost/geometry/multi/algorithms/centroid.hpp>
#include <boost/geometry/multi/algorithms/convex_hull.hpp>
#include <boost/geometry/multi/algorithms/correct.hpp>
#include <boost/geometry/multi/algorithms/dissolve.hpp>
#include <boost/geometry/multi/algorithms/distance.hpp>
#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/algorithms/equals.hpp>
#include <boost/geometry/multi/algorithms/for_each.hpp>
#include <boost/geometry/multi/algorithms/get_section.hpp>
#include <boost/geometry/multi/algorithms/intersection.hpp>
#include <boost/geometry/multi/algorithms/length.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/perimeter.hpp>
#include <boost/geometry/multi/algorithms/sectionalize.hpp>
#include <boost/geometry/multi/algorithms/simplify.hpp>
#include <boost/geometry/multi/algorithms/transform.hpp>
#include <boost/geometry/multi/algorithms/union.hpp>
#include <boost/geometry/multi/algorithms/unique.hpp>
#include <boost/geometry/multi/algorithms/within.hpp>

#include <boost/geometry/multi/algorithms/detail/modify_with_predicate.hpp>
#include <boost/geometry/multi/algorithms/detail/multi_sum.hpp>
#include <boost/geometry/multi/algorithms/overlay/copy_segments.hpp>
#include <boost/geometry/multi/algorithms/overlay/self_turn_points.hpp>
#include <boost/geometry/multi/algorithms/overlay/get_intersection_points.hpp>


#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/geometry/multi/geometries/concepts/check.hpp>
#include <boost/geometry/multi/geometries/concepts/multi_point_concept.hpp>
#include <boost/geometry/multi/geometries/concepts/multi_linestring_concept.hpp>
#include <boost/geometry/multi/geometries/concepts/multi_polygon_concept.hpp>

#include <boost/geometry/multi/iterators/range_type.hpp>

#include <boost/geometry/multi/strategies/centroid.hpp>
#include <boost/geometry/multi/strategies/cartesian/centroid_average.hpp>

#include <boost/geometry/multi/util/write_dsv.hpp>



#endif // BOOST_GEOMETRY_MULTI_HPP
