// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/gis/geographic/strategies/andoyer.hpp>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <test_common/test_point.hpp>


template <typename P1, typename P2>
void test_andoyer(double lon1, double lat1, double lon2, double lat2, double expected_km)
{
    boost::geometry::strategy::distance::andoyer<P1, P2> andoyer;

    P1 p1, p2;

    boost::geometry::assign(p1, lon1, lat1);
    boost::geometry::assign(p2, lon2, lat2);

    BOOST_CHECK_CLOSE(double(andoyer.apply(p1, p2)), 1000.0 * expected_km, 0.001);
}

template <typename P1, typename P2>
void test_all()
{
    test_andoyer<P1, P2>(0, 90, 1, 80, 1116.814237); // polar
    test_andoyer<P1, P2>(4, 52, 4, 52, 0.0); // no point difference
    test_andoyer<P1, P2>(4, 52, 3, 40, 1336.039890); // normal case
}

template <typename P>
void test_all()
{
    test_all<P, P>();
}

int test_main(int, char* [])
{
    using namespace boost::geometry;
    using namespace boost::geometry::cs;

    //test_all<float[2]>();
    //test_all<double[2]>();
    test_all<point<float, 2, geographic<degree> > >();
    test_all<point<double, 2, geographic<degree> > >();

    return 0;
}
