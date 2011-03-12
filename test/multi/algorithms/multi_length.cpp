// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/test_length.hpp>

#include <boost/geometry/multi/multi.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>



template <typename P>
void test_all()
{
    test_geometry<bg::model::multi_linestring<bg::model::linestring<P> > >
        ("MULTILINESTRING((0 0,3 4,4 3))", 5 + sqrt(2.0));
}

int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<double> >();

#ifdef HAVE_TTMATH
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
