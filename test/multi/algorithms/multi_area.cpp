// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/test_area.hpp>

#include <boost/geometry/multi/multi.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>




template <typename P>
void test_all()
{
    typedef bg::model::multi_polygon<bg::model::polygon<P> > mp;
    test_geometry<mp>("MULTIPOLYGON(((0 0,0 7,4 2,2 0,0 0)))", 16.0);
}

int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
