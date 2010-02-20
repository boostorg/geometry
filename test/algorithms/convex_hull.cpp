// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <string>

#include <algorithms/test_convex_hull.hpp>

#include <boost/geometry/geometries/geometries.hpp>



template <typename P>
void test_all(bool do_rectangular = true)
{
    // from sample linestring

    test_geometry<boost::geometry::linestring<P> >(
        "linestring(1.1 1.1, 2.5 2.1, 3.1 3.1, 4.9 1.1, 3.1 1.9)", 5, 4, 3.8);

    if (do_rectangular)
    {
        // rectangular, with concavity
        test_geometry<boost::geometry::polygon<P> >(
            "polygon((1 1, 1 4, 3 4, 3 3, 4 3, 4 4, 5 4, 5 1, 1 1))",
                    9, 5, 12.0);
    }

    // from sample polygon, with concavity
    test_geometry<boost::geometry::polygon<P> >(
        "polygon((2.0 1.3, 2.4 1.7, 2.8 1.8, 3.4 1.2, 3.7 1.6,3.4 2.0, 4.1 3.0"
        ", 5.3 2.6, 5.4 1.2, 4.9 0.8, 2.9 0.7,2.0 1.3))",
                12, 8, 5.245);
}

int test_main(int, char* [])
{
    //test_all<boost::geometry::point_xy<int> >();
    //test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

#if defined(HAVE_CLN)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif
#if defined(HAVE_GMP)
    // GMP compiles, but gives 'errors' in results (6 points in hull instead of 5)
    // This is because of a vertex on the line, which is OK for the convex hull,
    // but not OK for the strictly convex hull.
    // Actually this is unexpected from a high-precision arithmetic library, the
    // problem is the comparison (something is non-zero which should be calculated
    // zero, in the direction).

    test_all<boost::geometry::point_xy<boost::numeric_adaptor::gmp_value_type> >(false);
#endif

    return 0;
}
