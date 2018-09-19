// Boost.Geometry
// Unit Test

// Copyright (c) 2018, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html


#include <geometry_test_common.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/algorithms/line_interpolate_point.hpp>
#include <boost/geometry/algorithms/length.hpp>

#include <boost/geometry/iterators/segment_iterator.hpp>

#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/line_interpolate_point.hpp>
#include <boost/geometry/strategies/geographic/distance.hpp>
#include <boost/geometry/strategies/geographic/line_interpolate_point.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/strategies/spherical/line_interpolate_point.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>

template <typename P>
void check_points(P const& p0, P const& p1)
{
    double p00 = bg::get<0>(p0);
    double p10 = bg::get<0>(p1);

    BOOST_CHECK_CLOSE(p00, p10, 0.0000001);

    double p01 = bg::get<1>(p0);
    double p11 = bg::get<1>(p1);

    BOOST_CHECK_CLOSE(p01, p11, 0.0000001);
}


template <typename P, typename G>
inline void test(std::string const& wkt1,
                 double fraction,
                 std::string const& wkt2)
{
    G g;
    bg::read_wkt(wkt1, g);

    P o;
    bg::read_wkt(wkt2, o);

    P p;
    bg::line_interpolate_point(g, fraction, p);
    check_points(p, o);
}

template <typename P>
void test_all()
{
    typedef bg::model::segment<P> s_t;
    typedef bg::model::linestring<P> ls_t;

    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0,   "POINT(0 0)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.1, "POINT(0.4 0)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.2, "POINT(0.8 0)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.3, "POINT(1 0.2)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.4, "POINT(1 0.6)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.5, "POINT(1 1)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.6, "POINT(0.6 1)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.7, "POINT(0.2 1)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.8, "POINT(0 1.2)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 0.9, "POINT(0 1.6)");
    test<P,ls_t>("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 2)", 1,   "POINT(0 2)");
}

int test_main(int, char* [])
{
    test_all< bg::model::point<double, 2, bg::cs::cartesian> >();
    //test_all< bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > >();
    //test_all< bg::model::point<double, 2, bg::cs::geographic<bg::degree> > >();

    return 0;
}

