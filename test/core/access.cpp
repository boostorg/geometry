// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>


#include <boost/geometry/core/access.hpp>


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/algorithms/make.hpp>

#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/box.hpp>



template <typename G>
void test_get_set()
{
    typedef typename boost::geometry::coordinate_type<G>::type coordinate_type;

    G g;
    boost::geometry::set<0>(g, coordinate_type(1));
    boost::geometry::set<1>(g, coordinate_type(2));

    coordinate_type x = boost::geometry::get<0>(g);
    coordinate_type y = boost::geometry::get<1>(g);

    BOOST_CHECK_CLOSE(double(x), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(double(y), 2.0, 0.0001);
}

template <typename G>
void test_indexed_get_set(G& g)
{
    boost::geometry::set<0, 0>(g, 1);
    boost::geometry::set<0, 1>(g, 2);
    boost::geometry::set<1, 0>(g, 3);
    boost::geometry::set<1, 1>(g, 4);

    typedef typename boost::geometry::coordinate_type<G>::type coordinate_type;
    coordinate_type x1 = boost::geometry::get<0, 0>(g);
    coordinate_type y1 = boost::geometry::get<0, 1>(g);
    coordinate_type x2 = boost::geometry::get<1, 0>(g);
    coordinate_type y2 = boost::geometry::get<1, 1>(g);

    BOOST_CHECK_CLOSE(double(x1), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(double(y1), 2.0, 0.0001);
    BOOST_CHECK_CLOSE(double(x2), 3.0, 0.0001);
    BOOST_CHECK_CLOSE(double(y2), 4.0, 0.0001);
}

template <typename G, typename T>
void test_indexed_get(G const& g, T a, T b, T c, T d)
{
    T x1 = boost::geometry::get<0, 0>(g);
    T y1 = boost::geometry::get<0, 1>(g);
    T x2 = boost::geometry::get<1, 0>(g);
    T y2 = boost::geometry::get<1, 1>(g);

    BOOST_CHECK_CLOSE(double(x1), double(a), 0.0001);
    BOOST_CHECK_CLOSE(double(y1), double(b), 0.0001);
    BOOST_CHECK_CLOSE(double(x2), double(c), 0.0001);
    BOOST_CHECK_CLOSE(double(y2), double(d), 0.0001);
}

template <typename P>
void test_all()
{
    typedef typename boost::geometry::coordinate_type<P>::type coordinate_type;

    // POINT, setting coordinate
    test_get_set<P>();


    // BOX, setting left/right/top/bottom
    boost::geometry::box<P> b;
    test_indexed_get_set(b);

    // SEGMENT (in GGL not having default constructor; however that is not a requirement)
    P p1 = boost::geometry::make_zero<P>();
    P p2 = boost::geometry::make_zero<P>();
    boost::geometry::segment<P> s(p1, p2);
    test_indexed_get_set(s);

    // CONST SEGMENT
    boost::geometry::set<0>(p1, 1); // we don't use assign because dim in {2,3}
    boost::geometry::set<1>(p1, 2);
    boost::geometry::set<0>(p2, 3);
    boost::geometry::set<1>(p2, 4);
    boost::geometry::segment<const P> cs(p1, p2);
    test_indexed_get(cs,
        coordinate_type(1), coordinate_type(2),
        coordinate_type(3), coordinate_type(4));
}


int test_main(int, char* [])
{
    test_get_set<int[2]>();
    test_get_set<float[2]>();
    test_get_set<double[2]>();
    test_get_set<double[3]>();

    test_get_set<boost::tuple<double, double> >();

    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
