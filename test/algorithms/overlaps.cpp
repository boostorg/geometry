// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_overlaps.hpp>


#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>


template <typename P>
void test_2d()
{
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1, 3 3)", "BOX(0 0,2 2)", true);

    // touch -> false
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1, 3 3)", "BOX(3 3,5 5)", false);

    // disjoint -> false
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1, 3 3)", "BOX(4 4,6 6)", false);

    // within -> false
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1, 5 5)", "BOX(2 2,3 3)", false);

    // within+touch -> false
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1, 5 5)", "BOX(2 2,5 5)", false);
}

template <typename P>
void test_3d()
{
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1 1, 3 3 3)", "BOX(0 0 0,2 2 2)", true);
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1 1, 3 3 3)", "BOX(3 3 3,5 5 5)", false);
    test_geometry<boost::geometry::box<P>, boost::geometry::box<P> >("BOX(1 1 1, 3 3 3)", "BOX(4 4 4,6 6 6)", false);
}



int test_main( int , char* [] )
{
    test_2d<boost::geometry::point_xy<int> >();
    test_2d<boost::geometry::point_xy<double> >();

#if defined(HAVE_CLN)
    test_2d<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif
#if defined(HAVE_GMP)
    test_2d<boost::geometry::point_xy<boost::numeric_adaptor::gmp_value_type> >();
#endif

   //test_3d<boost::geometry::point<double, 3, boost::geometry::cs::cartesian> >();

    return 0;
}
