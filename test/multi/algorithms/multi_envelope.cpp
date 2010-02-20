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

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>




template <typename P>
void test_2d()
{
    /*test_envelope<boost::geometry::multi_point<P> >(
            "MULTIPOINT((1 1),(1 0),(1 2))", 1, 1, 0, 2);
    test_envelope<boost::geometry::multi_linestring<boost::geometry::linestring<P> > >(
            "MULTILINESTRING((0 0,1 1),(1 1,2 2),(2 2,3 3))", 0, 3, 0, 3);
*/
    test_envelope<boost::geometry::multi_polygon<boost::geometry::polygon<P> > >(
            "MULTIPOLYGON(((1 1,1 3,3 3,3 1,1 1)),((4 4,4 6,6 6,6 4,4 4)))", 1, 6, 1, 6);
}


template <typename P>
void test_3d()
{
    typedef boost::geometry::multi_point<P> mp;
}


int test_main( int , char* [] )
{
    test_2d<boost::tuple<float, float> >();
    test_2d<boost::geometry::point_xy<float> >();
    test_2d<boost::geometry::point_xy<double> >();

    test_3d<boost::tuple<float, float, float> >();
    test_3d<boost::geometry::point<double, 3, boost::geometry::cs::cartesian> >();

    return 0;
}
