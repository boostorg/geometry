// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <iterator>
#include <string>

#include <algorithms/test_convex_hull.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>

#include <boost/geometry/multi/core/point_order.hpp>
#include <boost/geometry/multi/core/point_type.hpp>

#include <boost/geometry/multi/iterators/range_type.hpp>

#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/detail/for_each_range.hpp>

#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>

#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>




template <typename P>
void test_all()
{
    typedef bg::model::multi_point<P> mp;
    typedef bg::model::multi_linestring<bg::model::linestring<P> > ml;
    typedef bg::model::multi_polygon<bg::model::polygon<P> > mpoly;
    test_geometry<mp>("multipoint((1.1 1.1), (2.5 2.1), (3.1 3.1), (4.9 1.1), (3.1 1.9))", 5, 4, 3.8);
    test_geometry<ml>("multilinestring((2 4, 3 4, 3 5), (4 3,4 4,5 4))", 6, 5, 3.0);
    test_geometry<mpoly>("multipolygon(((1 4,1 6,2 5,3 5,4 6,4 4,1 4)), ((4 2,4 3,6 3,6 2,4 2)))", 12, 7, 14.0);
}


int test_main(int, char* [])
{
    //test_all<bg::model::d2::point_xy<int> >();
    //test_all<bg::model::d2::point_xy<float> >();
    test_all<bg::model::d2::point_xy<double> >();

#ifdef HAVE_TTMATH
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
