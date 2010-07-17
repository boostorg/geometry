// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_within.hpp>


#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>


template <typename P>
void test_all()
{
    // trivial case
    test_ring<P>("POINT(1 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", true, false);

    // on border/corner
    test_ring<P>("POINT(0 0)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", false, true);
    test_ring<P>("POINT(0 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", false, true);

    // aligned to segment/vertex
    test_ring<P>("POINT(1 1)", "POLYGON((0 0,0 3,3 3,3 1,2 1,2 0,0 0))", true, false);
    test_ring<P>("POINT(1 1)", "POLYGON((0 0,0 3,4 3,3 1,2 2,2 0,0 0))", true, false);

    // same polygon, but point on border
    test_ring<P>("POINT(3 3)", "POLYGON((0 0,0 3,3 3,3 1,2 1,2 0,0 0))", false, true);
    test_ring<P>("POINT(3 3)", "POLYGON((0 0,0 3,4 3,3 1,2 2,2 0,0 0))", false, true);

    // holes
    test_geometry<P, boost::geometry::polygon<P> >("POINT(2 2)",
        "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,3 1,3 3,1 3,1 1))", false);


    test_geometry<P, boost::geometry::box<P> >("POINT(1 1)", "BOX(0 0,2 2)", true);

    // Mixed point types
    test_geometry
        <
            boost::geometry::point_xy<float>,
            boost::geometry::polygon<P>
        >("POINT(1 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", true);


    // Real-life problem (solved now), point is in the middle, 409623 is also a coordinate
    // on the border, it was wrong first.
    test_ring<P>("POINT(146383 409623)",
        "POLYGON((146351 410597,146521 410659,147906 410363,148088 410420"
        ",148175 410296,148281 409750,148215 409623,148154 409666,148154 409666"
        ",148130 409625,148035 409626,148035 409626,148008 409544,147963 409510"
        ",147993 409457,147961 409352,147261 408687,147008 408586,145714 408840"
        ",145001 409033,144486 409066,144616 409308,145023 410286,145254 410488"
        ",145618 410612,145618 410612,146015 410565,146190 410545,146351 410597))",
        true, false);
}




int test_main( int , char* [] )
{
    test_all<boost::geometry::point_xy<int> >();
    test_all<boost::geometry::point_xy<double> >();

#if defined(HAVE_CLN)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif
#if defined(HAVE_GMP)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::gmp_value_type> >();
#endif

    return 0;
}
