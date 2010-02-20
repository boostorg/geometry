// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/geometry/geometries/geometries.hpp>

#include <algorithms/test_perimeter.hpp>


template <typename P>
void test_all()
{
    // 3-4-5 triangle
    //test_geometry<std::pair<P, P> >("LINESTRING(0 0,3 4)", 5);

    test_geometry<boost::geometry::linear_ring<P> >(
            "POLYGON((0 0,0 1,1 1,1 0,0 0))", 4);
    test_geometry<boost::geometry::polygon<P> >(
            "POLYGON((0 0,0 1,1 0,0 0))", 1.0 + 1.0 + sqrt(2.0));
    test_geometry<boost::geometry::polygon<P> >(
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,2 1,2 2,1 2,1 1))", 20);
}

int test_main(int, char* [])
{
    //test_all<boost::geometry::point_xy<int> >();
    test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

#if defined(HAVE_CLN)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif
#if defined(HAVE_GMP)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::gmp_value_type> >();
#endif

    return 0;
}
