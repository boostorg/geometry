// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_length.hpp>


#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/std_pair_as_segment.hpp>




template <typename P>
void test_all()
{
    // 3-4-5 triangle
    test_geometry<std::pair<P, P> >("LINESTRING(0 0,3 4)", 5);

    // 3-4-5 plus 1-1
    test_geometry<boost::geometry::linestring<P> >("LINESTRING(0 0,3 4,4 3)", 5 + sqrt(2.0));

    // Geometries with length zero
    test_geometry<P>("POINT(0 0)", 0);
    test_geometry<boost::geometry::polygon<P> >("POLYGON((0 0,0 1,1 1,1 0,0 0))", 0);
}

int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<int> >();
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
