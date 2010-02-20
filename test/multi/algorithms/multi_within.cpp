// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/multi/algorithms/within.hpp>

#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <algorithms/test_within.hpp>


template <typename P>
void test_all()
{
    typedef boost::geometry::multi_polygon<boost::geometry::polygon<P> > mp;

    // trivial cases
    test_geometry<P, mp>("POINT(1 1)", "MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0)))", true);
    test_geometry<P, mp>("POINT(3 3)", "MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0)))", false);

    // test if it is in one of them
    test_geometry<P, mp>("POINT(4 4)", "MULTIPOLYGON("
        "((0 0,0 2,2 2,2 0,0 0))"
        "((3 3,3 6,6 6,6 3,3 3))"
        ")",
        true);
}

int test_main( int , char* [] )
{
    //test_all<boost::geometry::point_xy<int> >();
    test_all<boost::geometry::point_xy<double> >();

#if defined(HAVE_CLN)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif
#if defined(HAVE_GMP)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::gmp_value_type> >();
#endif

    return 0;
}
