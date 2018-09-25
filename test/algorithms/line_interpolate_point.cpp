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

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>

template <typename P>
void check_points(P const& p0, P const& p1)
{
    double p00 = bg::get<0>(p0);
    double p10 = bg::get<0>(p1);

    BOOST_CHECK_CLOSE(p00, p10, 0.001);

    double p01 = bg::get<1>(p0);
    double p11 = bg::get<1>(p1);

    BOOST_CHECK_CLOSE(p01, p11, 0.001);
}


template <typename P, typename G, typename S>
inline void test(std::string const& wkt1,
                 double fraction,
                 std::string const& wkt2,
                 S str)
{
    G g;
    bg::read_wkt(wkt1, g);

    P o;
    bg::read_wkt(wkt2, o);

    P p;
    bg::line_interpolate_point(g, fraction, p);
    check_points(p, o);

    bg::line_interpolate_point(g, fraction, p, str);
    check_points(p, o);
}

template <typename P, typename G>
inline void test(std::string const& wkt1,
                 double fraction,
                 std::string const& wkt2)
{
    test<P,G>(wkt1, fraction, wkt2, bg::default_strategy());
}

void test_car()
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> P;
    typedef bg::model::segment<P> s_t;
    typedef bg::model::linestring<P> ls_t;

    std::string const s = "SEGMENT(1 1, 2 2)";
    std::string const l = "LINESTRING(1 1, 2 1, 2 2, 1 2, 1 3)";

    test<P,s_t>(s, 0,   "POINT(1 1)");
    test<P,s_t>(s, 0.5, "POINT(1.5 1.5)");
    test<P,s_t>(s, 1,   "POINT(2 2)");

    test<P,ls_t>(l, 0,   "POINT(1 1)");
    test<P,ls_t>(l, 0.1, "POINT(1.4 1)");
    test<P,ls_t>(l, 0.2, "POINT(1.8 1)");
    test<P,ls_t>(l, 0.3, "POINT(2 1.2)");
    test<P,ls_t>(l, 0.4, "POINT(2 1.6)");
    test<P,ls_t>(l, 0.5, "POINT(2 2)");
    test<P,ls_t>(l, 0.6, "POINT(1.6 2)");
    test<P,ls_t>(l, 0.7, "POINT(1.2 2)");
    test<P,ls_t>(l, 0.8, "POINT(1 2.2)");
    test<P,ls_t>(l, 0.9, "POINT(1 2.6)");
    test<P,ls_t>(l, 1,   "POINT(1 3)");
}

void test_sph()
{
    typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > P;
    typedef bg::model::segment<P> s_t;
    typedef bg::model::linestring<P> ls_t;

    std::string const s = "SEGMENT(1 1, 2 2)";
    std::string const l = "LINESTRING(1 1, 2 1, 2 2, 1 2, 1 3)";

    test<P,s_t>(s, 0,   "POINT(1 1)");
    test<P,s_t>(s, 0.5, "POINT(1.4998857365615981 1.5000570914791198)");
    test<P,s_t>(s, 1,   "POINT(2 2)");

    test<P,ls_t>(l, 0,   "POINT(1 1)");
    test<P,ls_t>(l, 0.1, "POINT(1.39998476912905323 1.0000365473536286)");
    test<P,ls_t>(l, 0.2, "POINT(1.79996953825810646 1.0000243679448551)");
    test<P,ls_t>(l, 0.3, "POINT(2 1.1999238595669637)");
    test<P,ls_t>(l, 0.4, "POINT(2 1.5998477098527744)");
    test<P,ls_t>(l, 0.5, "POINT(2 1.9997715601385837)");
    test<P,ls_t>(l, 0.6, "POINT(1.6000609543036084 2.0000730473928678)");
    test<P,ls_t>(l, 0.7, "POINT(1.1998933176222553 2.0000486811516014)");
    test<P,ls_t>(l, 0.8, "POINT(1 2.2001522994279883)");
    test<P,ls_t>(l, 0.9, "POINT(1 2.6000761497139444)");
    test<P,ls_t>(l, 1,   "POINT(1 3)");
}

template <typename S>
void test_geo(S str)
{
    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > P;
    typedef bg::model::segment<P> s_t;
    typedef bg::model::linestring<P> ls_t;

    std::string const s = "SEGMENT(1 1, 2 2)";
    std::string const l = "LINESTRING(1 1, 2 1, 2 2, 1 2, 1 3)";

    test<P,s_t>(s, 0,   "POINT(1 1)", str);
    test<P,s_t>(s, 0.5, "POINT(1.4998780900539985 1.5000558288006378)", str);
    test<P,s_t>(s, 1,   "POINT(2 2)", str);

    test<P,ls_t>(l, 0,   "POINT(1 1)", str);
    test<P,ls_t>(l, 0.1, "POINT(1.3986445638301882 1.0000367522730751)", str);
    test<P,ls_t>(l, 0.2, "POINT(1.79728912766037641 1.0000247772611039)", str);
    test<P,ls_t>(l, 0.3, "POINT(2 1.1972285554368427)", str);
    test<P,ls_t>(l, 0.4, "POINT(2 1.598498298996567)", str);
    test<P,ls_t>(l, 0.5, "POINT(2 1.9997664696834965)", str);
    test<P,ls_t>(l, 0.6, "POINT(1.6013936980010324 2.0000734568388099)", str);
    test<P,ls_t>(l, 0.7, "POINT(1.2025664628960846 2.0000494983098767)", str);
    test<P,ls_t>(l, 0.8, "POINT(1 2.1974612279909937)", str);
    test<P,ls_t>(l, 0.9, "POINT(1 2.5987263175375022)", str);
    test<P,ls_t>(l, 1,   "POINT(1 3)", str);
}

int test_main(int, char* [])
{
    test_car();
    test_sph();
    test_geo(bg::strategy::segment_interpolate_point::geographic<bg::strategy::andoyer>());
    test_geo(bg::strategy::segment_interpolate_point::geographic<bg::strategy::thomas>());
    test_geo(bg::strategy::segment_interpolate_point::geographic<bg::strategy::vincenty>());
    return 0;
}

