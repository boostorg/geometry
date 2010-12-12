// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/util/closeable_view.hpp>
#include <boost/geometry/util/reversible_view.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/util/write_dsv.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>


template <typename View, typename Range>
void test_option(Range const& range, std::string const& expected)
{
    View view(range);

    bool first = true;
    std::ostringstream out;

    typedef typename boost::range_iterator<View const>::type iterator;
    ////std::cout << typeid(iterator).name() << std::endl;
    iterator end = boost::end(view);
    for (iterator it = boost::begin(view); it != end; ++it, first = false)
    {
        out << (first ? "" : " ") << bg::dsv(*it);
    }
    BOOST_CHECK_EQUAL(out.str(), expected);
}

template <bg::closure_selector Closure, bg::iterate_direction Direction, typename Range>
void test_close_reverse(Range const& range, std::string const& expected)
{
    test_option
        <
            typename bg::closeable_view
                <
                    typename bg::reversible_view<Range const, Direction>::type const,
                    Closure
                >::type
        >(range, expected);
}


template <bg::iterate_direction Direction, bg::closure_selector Close, typename Range>
void test_reverse_close(Range const& range, std::string const& expected)
{
    test_option
        <
            typename bg::reversible_view
                <
                    typename bg::closeable_view<Range const, Close>::type const,
                    Direction
                >::type
        >(range, expected);
}


template
<
    bg::iterate_direction Direction1,
    bg::iterate_direction Direction2,
    typename Range
>
void test_reverse_reverse(Range const& range, std::string const& expected)
{
    test_option
        <
            typename bg::reversible_view
                <
                    typename bg::reversible_view<Range const, Direction2>::type const,
                    Direction1
                >::type
        >(range, expected);
}

template
<
    bg::closure_selector Close1,
    bg::closure_selector Close2,
    typename Range
>
void test_close_close(Range const& range, std::string const& expected)
{
    test_option
        <
            typename bg::closeable_view
                <
                    typename bg::closeable_view<Range const, Close2>::type const,
                    Close1
                >::type
        >(range, expected);
}


template <typename Geometry>
void test_geometry(std::string const& wkt,
            std::string const& expected_n,
            std::string const& expected_r,
            std::string const& closing,
            std::string const& rclosing
            )
{
    std::string expected;
    Geometry geo;
    bg::read_wkt(wkt, geo);

    test_close_reverse<bg::closed, bg::iterate_forward>(geo, expected_n);
    test_close_reverse<bg::open, bg::iterate_forward>(geo, expected_n + closing);
    test_close_reverse<bg::closed, bg::iterate_reverse>(geo, expected_r);

#if ! defined(_MSC_VER)
    // 13-12-2010, Currently problematic in MSVC
    test_close_reverse<bg::open, bg::iterate_reverse>(geo, expected_r + rclosing);
#endif

    test_reverse_close<bg::iterate_forward, bg::closed>(geo, expected_n);
    test_reverse_close<bg::iterate_forward, bg::open>(geo, expected_n + closing);
    test_reverse_close<bg::iterate_reverse, bg::closed>(geo, expected_r);

    // first closed, then reversed: 
    expected = boost::trim_copy(closing + " " + expected_r); 
    test_reverse_close<bg::iterate_reverse, bg::open>(geo, expected);

    test_reverse_reverse<bg::iterate_forward, bg::iterate_forward>(geo, expected_n);
    test_reverse_reverse<bg::iterate_reverse, bg::iterate_reverse>(geo, expected_n);
    test_reverse_reverse<bg::iterate_forward, bg::iterate_reverse>(geo, expected_r);
    test_reverse_reverse<bg::iterate_reverse, bg::iterate_forward>(geo, expected_r);

    test_close_close<bg::closed, bg::closed>(geo, expected_n);
    test_close_close<bg::open, bg::closed>(geo, expected_n + closing);
    test_close_close<bg::closed, bg::open>(geo, expected_n + closing);
    test_close_close<bg::open, bg::open>(geo, expected_n + closing + closing);
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::linear_ring<P> >(
            "POLYGON((1 1,1 4,4 4,4 1))",
            "(1, 1) (1, 4) (4, 4) (4, 1)",
            "(4, 1) (4, 4) (1, 4) (1, 1)",
            " (1, 1)", // closing
            " (4, 1)" // rclosing
            );
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<boost::tuple<double, double> >();

    return 0;
}
