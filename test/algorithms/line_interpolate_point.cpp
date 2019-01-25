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

template <typename P, typename Tag = typename bg::tag<P>::type>
struct check_points: bg::not_implemented<Tag>
{};

template <typename P>
struct check_points<P, bg::point_tag>
{
    static void apply(P const& p0, P const& p1)
    {
        double p00 = bg::get<0>(p0);
        double p10 = bg::get<0>(p1);

        BOOST_CHECK_CLOSE(p00, p10, 0.001);

        double p01 = bg::get<1>(p0);
        double p11 = bg::get<1>(p1);

        BOOST_CHECK_CLOSE(p01, p11, 0.001);
    }
};

template <typename P>
struct check_points<P, bg::multi_point_tag>
{
    template <typename Range>
    static void apply(Range const& r0, Range const& r1)
    {

        typedef typename boost::range_iterator<Range const>::type iterator_t;
        typedef typename boost::range_value<Range const>::type point_t;

        BOOST_CHECK_EQUAL(boost::size(r0), boost::size(r1));

        for (iterator_t it0 = boost::begin(r0), it1 = boost::begin(r1);
             it0 < boost::end(r0); it0++, it1++)
        {
            check_points<point_t>::apply(*it0, *it1);
        }

    }
};

template <typename G, typename P, typename S>
inline void test(std::string const& wkt1,
                 double fraction,
                 std::string const& wkt2,
                 S str)
{
    G g;
    bg::read_wkt(wkt1, g);

    P o;
    bg::read_wkt(wkt2, o);

    P p1;
    bg::line_interpolate_point(g, fraction * bg::length(g), p1, str);
    check_points<P>::apply(p1, o);

}

template <typename G, typename P>
inline void test(std::string const& wkt1,
                 double fraction,
                 std::string const& wkt2)
{
    G g;
    bg::read_wkt(wkt1, g);

    P o;
    bg::read_wkt(wkt2, o);

    P p1;
    bg::line_interpolate_point(g, fraction * bg::length(g), p1);
    check_points<P>::apply(p1, o);
}

void test_car()
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> P;
    typedef bg::model::multi_point<P> MP;
    typedef bg::model::segment<P> S;
    typedef bg::model::linestring<P> LS;

    std::string const s = "SEGMENT(1 1, 2 2)";
    std::string const l = "LINESTRING(1 1, 2 1, 2 2, 1 2, 1 3)";

    test<S,P>(s, 0,   "POINT(1 1)");
    test<S,P>(s, 0.5, "POINT(1.5 1.5)");
    test<S,P>(s, 1,   "POINT(2 2)");

    test<LS,P>(l, 0,   "POINT(1 1)");
    test<LS,P>(l, 0.1, "POINT(1.4 1)");
    test<LS,P>(l, 0.2, "POINT(1.8 1)");
    test<LS,P>(l, 0.3, "POINT(2 1.2)");
    test<LS,P>(l, 0.4, "POINT(2 1.6)");
    test<LS,P>(l, 0.5, "POINT(2 2)");
    test<LS,P>(l, 0.6, "POINT(1.6 2)");
    test<LS,P>(l, 0.7, "POINT(1.2 2)");
    test<LS,P>(l, 0.8, "POINT(1 2.2)");
    test<LS,P>(l, 0.9, "POINT(1 2.6)");
    test<LS,P>(l, 1,   "POINT(1 3)");

    test<LS,MP>(l, 0, "MULTIPOINT((1 1))");
    test<LS,MP>(l, 0.1, "MULTIPOINT((1.4 1)(1.8 1)(2 1.2)(2 1.6)(2 2)(1.6 2)\
                                    (1.2 2)(1 2.2)(1 2.6))");//(1 3) missing
    test<LS,MP>(l, 0.2, "MULTIPOINT((1.8 1)(2 1.6)(1.6 2)(1 2.2))");//(1 3) missing
    test<LS,MP>(l, 0.4, "MULTIPOINT((2 1.6)(1 2.2))");
    test<LS,MP>(l, 0.5, "MULTIPOINT((2 2)(1 3))");
    test<LS,MP>(l, 0.6, "MULTIPOINT((1.6 2))");
    test<LS,MP>(l, 1, "MULTIPOINT((1 3))");
}

void test_sph()
{
    typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > P;
    typedef bg::model::multi_point<P> MP;
    typedef bg::model::segment<P> S;
    typedef bg::model::linestring<P> LS;

    std::string const s = "SEGMENT(1 1, 2 2)";
    std::string const l = "LINESTRING(1 1, 2 1, 2 2, 1 2, 1 3)";
    std::string const l2 = "LINESTRING(0 2, 5 2, 5 1, 20 1)";

    test<S,P>(s, 0,   "POINT(1 1)");
    test<S,P>(s, 0.5, "POINT(1.4998857365615981 1.5000570914791198)");
    test<S,P>(s, 1,   "POINT(2 2)");

    test<LS,P>(l, 0,   "POINT(1 1)");
    test<LS,P>(l, 0.1, "POINT(1.39998476912905323 1.0000365473536286)");
    test<LS,P>(l, 0.2, "POINT(1.79996953825810646 1.0000243679448551)");
    test<LS,P>(l, 0.3, "POINT(2 1.1999238595669637)");
    test<LS,P>(l, 0.4, "POINT(2 1.5998477098527744)");
    test<LS,P>(l, 0.5, "POINT(2 1.9997715601390484)");
    test<LS,P>(l, 0.6, "POINT(1.6000609543036084 2.0000730473928678)");
    test<LS,P>(l, 0.7, "POINT(1.1998933176222553 2.0000486811516014)");
    test<LS,P>(l, 0.8, "POINT(1 2.2001522994279883)");
    test<LS,P>(l, 0.9, "POINT(1 2.6000761497139444)");
    test<LS,P>(l, 1,   "POINT(1 3)");

    test<LS,MP>(l, 0, "MULTIPOINT((1 1))");
    test<LS,MP>(l, 0.1, "MULTIPOINT((1.39998476912905323 1.0000365473536286)\
                                    (1.79996953825810646 1.0000243679448551)\
                                    (2 1.1999238595669637)\
                                    (2 1.5998477098527744)\
                                    (2 1.9997715601385837)\
                                    (1.6000609543036084 2.0000730473928678)\
                                    (1.1998933176222553 2.0000486811516014)\
                                    (1 2.2001522994279883)\
                                    (1 2.6000761497139444)\
                                    )");//(1,3)
    test<LS,MP>(l, 0.2, "MULTIPOINT((1.79996953825810646 1.0000243679448551)\
                                    (2 1.5998477098527744)\
                                    (1.6000609543036084 2.0000730473928678)\
                                    (1 2.2001522994279883)\
                                    )");//(1,3)
    test<LS,MP>(l, 0.4, "MULTIPOINT((2 1.5998477098527744)(1 2.2001522994279883))");
    test<LS,MP>(l, 0.5, "MULTIPOINT((2 1.9997715601385837)(1 3))");
    test<LS,MP>(l, 0.6, "MULTIPOINT((1.6000609543036084 2.0000730473928678))");
    test<LS,MP>(l, 1, "MULTIPOINT((1 3))");

    test<LS,MP>(l2, 0.3, "MULTIPOINT((5.3014893312120446 1.0006787676128222)\
                                     (11.600850053156366 1.0085030143490989)\
                                     (17.9002174825842 1.0041514208039872))");

}

template <typename Strategy>
void test_geo(Strategy str)
{
    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > P;
    typedef bg::model::multi_point<P> MP;
    typedef bg::model::segment<P> S;
    typedef bg::model::linestring<P> LS;

    std::string const s = "SEGMENT(1 1, 2 2)";
    std::string const l = "LINESTRING(1 1, 2 1, 2 2, 1 2, 1 3)";
    std::string const l2 = "LINESTRING(0 2, 5 2, 5 1, 20 1)";

    test<S,P>(s, 0,   "POINT(1 1)", str);
    test<S,P>(s, 0.5, "POINT(1.4998780900539985 1.5000558288006378)", str);
    test<S,P>(s, 1,   "POINT(2 2)", str);

    test<LS,P>(l, 0,   "POINT(1 1)", str);
    test<LS,P>(l, 0.1, "POINT(1.3986445638301882 1.0000367522730751)", str);
    test<LS,P>(l, 0.2, "POINT(1.79728912766037641 1.0000247772611039)", str);
    test<LS,P>(l, 0.3, "POINT(2 1.1972285554368427)", str);
    test<LS,P>(l, 0.4, "POINT(2 1.598498298996567)", str);
    test<LS,P>(l, 0.5, "POINT(2 1.9997664696834965)", str);
    test<LS,P>(l, 0.6, "POINT(1.6013936980010324 2.0000734568388099)", str);
    test<LS,P>(l, 0.7, "POINT(1.2025664628960846 2.0000494983098767)", str);
    test<LS,P>(l, 0.8, "POINT(1 2.1974612279909937)", str);
    test<LS,P>(l, 0.9, "POINT(1 2.5987263175375022)", str);
    test<LS,P>(l, 1,   "POINT(1 3)", str);

    test<LS,MP>(l, 0, "MULTIPOINT((1 1))", str);
    test<LS,MP>(l, 0.1, "MULTIPOINT((1.3986445638301882 1.0000367522730751)\
                                    (1.79728912766037641 1.0000735036506381)\
                                    (2 1.1972285554368427)\
                                    (2 1.598498298996567)\
                                    (2 1.9997664696834965)\
                                    (1.6013936980010324 2.0000734568388099)\
                                    (1.2025664628960846 2.0001469249038197)\
                                    (1 2.1974612279909937)\
                                    (1 2.5987263175375022)\
                                    (1 3))", str);

    test<LS,MP>(l, 0.2, "MULTIPOINT((1.79728912766037641 1.0000247772613331)\
                                    (2 1.598498298996567)\
                                    (1.6013936980010324 2.0000734568388099)\
                                    (1 2.1974612279909937)\
                                    (1 3))", str);

    test<LS,MP>(l, 0.4, "MULTIPOINT((2 1.598498298996567)(1 2.1974612279909937))", str);
    test<LS,MP>(l, 0.5, "MULTIPOINT((2 1.9997664696834965)(1 3))", str);
    test<LS,MP>(l, 0.6, "MULTIPOINT((1.6013936980010324 2.0000734568388099))", str);
    test<LS,MP>(l, 1, "MULTIPOINT((1 3))", str);

    test<LS,MP>(l2, 0.3, "MULTIPOINT((5.306157814 1.0006937303)\
                                     (11.60351281 1.0091515976)\
                                     (17.90073492 1.0175580865))", str);


}

int test_main(int, char* [])
{
    test_car();
    test_sph();
    //test_geo(bg::strategy::line_interpolate_point::geographic<bg::strategy::andoyer>());
    test_geo(bg::strategy::line_interpolate_point::geographic<bg::strategy::thomas>());
    test_geo(bg::strategy::line_interpolate_point::geographic<bg::strategy::vincenty>());

    //TODO:add distance longer than length
    //& negative distance
    return 0;
}

