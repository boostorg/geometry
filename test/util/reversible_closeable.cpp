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

//// BSG 12-10-2010
//// Currently not compiles because of changes in ring behaviour.
//// TODO: fix this.


template <typename View, typename Range>
void test_option(Range const& range, std::string const& expected)
{

    View view(range);

    bool first = true;
    std::ostringstream out;

    typedef typename boost::range_iterator<View const>::type iterator;
    iterator end = boost::end(view);
    for (iterator it = boost::begin(view); it != end; ++it, first = false)
    {
        out << (first ? "" : " ") << bg::dsv(*it);
    }
    BOOST_CHECK_EQUAL(out.str(), expected);
}

////
/*
template <bool Close, bg::iterate_direction Direction, typename Range>
void test_close_reverse(Range const& range, std::string const& expected)
{
    test_option
        <
            bg::closeable_view
                <
                    bg::reversible_view<Range const, Direction> const,
                    Close
                >
        >(range, expected);
}
*/
////

/*

This should NOT compile, or at least not instantiate
Use the code as above, so reversible_view should be nested inside closeable_view

template <bg::iterate_direction Direction, bool Close, typename Range>
void test_reverse_close(Range const& range, std::string const& expected)
{
    test_option
        <
            bg::reversible_view
                <
                    bg::closeable_view<Range const, Close> const,
                    Direction
                >
        >(range, expected);
}
*/

////
/*template
<
    bg::iterate_direction Direction1,
    bg::iterate_direction Direction2,
    typename Range
>
void test_reverse_reverse(Range const& range, std::string const& expected)
{
    test_option
        <
            bg::reversible_view
                <
                    bg::reversible_view<Range const, Direction2> const,
                    Direction1
                >
        >(range, expected);
}*/
////

template
<
    bool Close1,
    bool Close2,
    typename Range
>
void test_close_close(Range const& range, std::string const& expected)
{
    test_option
        <
            bg::closeable_view
                <
                    bg::closeable_view<Range const, Close2> const,
                    Close1
                >
        >(range, expected);
}


template <typename Geometry>
void test_geometry(std::string const& wkt,
            std::string const& expected_1,
            std::string const& expected_2,
            std::string const& expected_3,
            std::string const& expected_4,
            std::string const& expected_cc
            )
{
    Geometry geo;
    bg::read_wkt(wkt, geo);

    ////test_close_reverse<false, bg::iterate_forward>(geo, expected_1);
    ////test_close_reverse<true, bg::iterate_forward>(geo, expected_2);
    ////test_close_reverse<false, bg::iterate_reverse>(geo, expected_3);
    ////test_close_reverse<true, bg::iterate_reverse>(geo, expected_4);

    /*
    This should NOT compile on purpose
    Because the closing_iterator can only be used forward
    test_reverse_close<bg::iterate_forward, false>(geo, expected_1);
    test_reverse_close<bg::iterate_forward, true>(geo, expected_2);
    test_reverse_close<bg::iterate_reverse, false>(geo, expected_3);
    test_reverse_close<bg::iterate_reverse, true>(geo, expected_4);
    */

    ////test_reverse_reverse<bg::iterate_forward, bg::iterate_forward>(geo, expected_1);
    ////test_reverse_reverse<bg::iterate_reverse, bg::iterate_reverse>(geo, expected_1);

    ////test_close_close<false, false>(geo, expected_1);
    ////test_close_close<true, false>(geo, expected_2);
    ////test_close_close<false, true>(geo, expected_2);

    // Does not compile - no assignment operator
    // Ranges basically support nesting, but the closing iterator does not.
    // It is not necessary for the closing iterator to do so.

    //test_close_close<true, true>(geo, expected_cc);
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::linear_ring<P> >(
            "POLYGON((1 1,1 4,4 4,4 1))",
            "(1, 1) (1, 4) (4, 4) (4, 1)",
            "(1, 1) (1, 4) (4, 4) (4, 1) (1, 1)",
            "(4, 1) (4, 4) (1, 4) (1, 1)",
            "(4, 1) (4, 4) (1, 4) (1, 1) (4, 1)",
            "(1, 1) (1, 4) (4, 4) (4, 1) (1, 1) (1, 1)" // closed twice, not used
            );
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<boost::tuple<double, double> >();

    return 0;
}
