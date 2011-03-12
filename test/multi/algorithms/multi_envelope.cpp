// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/test_envelope.hpp>


#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/domains/gis/io/wkt/read_wkt_multi.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>




template <typename P>
void test_2d()
{
    /*test_envelope<bg::model::multi_point<P> >(
            "MULTIPOINT((1 1),(1 0),(1 2))", 1, 1, 0, 2);
    test_envelope<bg::model::multi_linestring<bg::model::linestring<P> > >(
            "MULTILINESTRING((0 0,1 1),(1 1,2 2),(2 2,3 3))", 0, 3, 0, 3);
*/
    test_envelope<bg::model::multi_polygon<bg::model::polygon<P> > >(
            "MULTIPOLYGON(((1 1,1 3,3 3,3 1,1 1)),((4 4,4 6,6 6,6 4,4 4)))", 1, 6, 1, 6);
}


template <typename P>
void test_3d()
{
    typedef bg::model::multi_point<P> mp;
}


int test_main( int , char* [] )
{
    test_2d<boost::tuple<float, float> >();
    test_2d<bg::model::d2::point_xy<float> >();
    test_2d<bg::model::d2::point_xy<double> >();

    test_3d<boost::tuple<float, float, float> >();
    test_3d<bg::model::point<double, 3, bg::cs::cartesian> >();

#ifdef HAVE_TTMATH
    test_2d<bg::model::d2::point_xy<ttmath_big> >();
    test_2d<bg::model::point<ttmath_big, 3, bg::cs::cartesian> >();
#endif

    return 0;
}
