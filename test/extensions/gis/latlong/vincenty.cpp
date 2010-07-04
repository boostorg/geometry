// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/concept_check.hpp>

#include <boost/geometry/extensions/gis/geographic/strategies/vincenty.hpp>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <test_common/test_point.hpp>

#ifdef HAVE_TTMATH
#  include <boost/geometry/extensions/contrib/ttmath_stub.hpp>
#endif



template <typename P1, typename P2>
void test_vincenty(double lon1, double lat1, double lon2, double lat2, double expected_km)
{
    typedef boost::geometry::strategy::distance::vincenty<P1, P2> vincenty_type;

    BOOST_CONCEPT_ASSERT( (boost::geometry::concept::PointDistanceStrategy<vincenty_type>) );

    vincenty_type vincenty;
    typedef typename vincenty_type::return_type return_type;


    P1 p1, p2;

    boost::geometry::assign(p1, lon1, lat1);
    boost::geometry::assign(p2, lon2, lat2);

    BOOST_CHECK_CLOSE(vincenty.apply(p1, p2), return_type(1000.0) * return_type(expected_km), 0.001);
}

template <typename P1, typename P2>
void test_all()
{
    test_vincenty<P1, P2>(0, 90, 1, 80, 1116.828862); // polar
    test_vincenty<P1, P2>(4, 52, 4, 52, 0.0); // no point difference
    test_vincenty<P1, P2>(4, 52, 3, 40, 1336.039890); // normal case
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
    test_all<point<int, 2, geographic<degree> > >();
    //test_all<point<float, 2, geographic<degree> > >();
    test_all<point<double, 2, geographic<degree> > >();

#if defined(HAVE_TTMATH)
    test_all<point<ttmath::Big<1,4>, 2, geographic<degree> > >();
    test_all<point<ttmath_big, 2, geographic<degree> > >();
#endif


    return 0;
}
