// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/iterators/circular_iterator.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/geometries/geometries.hpp>


template <typename Geometry, typename CircularIterator>
void test_forward(Geometry const& geometry, CircularIterator end,
        int offset, std::string const& expected)
{
    CircularIterator it(boost::begin(geometry), boost::end(geometry),
                boost::begin(geometry) + offset);

    std::ostringstream out;
    for (; it != end; ++it)
    {
        out << bg::get<0>(*it);
    }
    BOOST_CHECK_EQUAL(out.str(), expected);
}


template <typename Geometry, typename CircularIterator>
void test_backward(Geometry const& geometry, CircularIterator end,
        int offset, std::string const& expected)
{
    CircularIterator it(boost::begin(geometry), boost::end(geometry),
                boost::begin(geometry) + offset);

    std::ostringstream out;
    for (; it != end; --it)
    {
        out << bg::get<0>(*it);
    }
    BOOST_CHECK_EQUAL(out.str(), expected);
}



template <typename G>
void test_geometry(std::string const& wkt)
{
    G geo;
    bg::read_wkt(wkt, geo);
    typedef typename boost::range_const_iterator<G>::type normal_iterator;
    typedef bg::circular_iterator<normal_iterator> circular_iterator;

    circular_iterator end(boost::end(geo));

    // 2: start somewhere in the middle (first == test before)
    test_forward(geo, end, 0, "12345");
    test_forward(geo, end, 1, "23451");
    test_forward(geo, end, 2, "34512");
    test_forward(geo, end, 3, "45123");
    test_forward(geo, end, 4, "51234");

    test_backward(geo, end, 0, "15432");
    test_backward(geo, end, 1, "21543");
    test_backward(geo, end, 2, "32154");
    test_backward(geo, end, 3, "43215");
    test_backward(geo, end, 4, "54321");

    // 4: check copy behaviour
    G copy;

    normal_iterator start = boost::begin(geo) + 2;
    circular_iterator it(boost::begin(geo), boost::end(geo), start);
    std::copy<circular_iterator>(it, end, std::back_inserter(copy));

    std::ostringstream out;
    for (normal_iterator cit = boost::begin(copy); cit != boost::end(copy); ++cit)
    {
        out << bg::get<0>(*cit);
    }
    BOOST_CHECK_EQUAL(out.str(), "34512");
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::linestring<P> >("linestring(1 1,2 2,3 3,4 4,5 5)");
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
