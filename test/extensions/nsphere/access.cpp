// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>


#include <boost/geometry/extensions/nsphere/core/access.hpp>
#include <boost/geometry/extensions/nsphere/geometries/nsphere.hpp>



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


template <typename P>
void test_all()
{
    typedef typename boost::geometry::coordinate_type<P>::type coordinate_type;

    // N-SPHERE, setting sphere center
    test_get_set<boost::geometry::nsphere<P, coordinate_type> >();

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
