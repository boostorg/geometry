// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/ranges/box_range.hpp>
#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>



template <typename Box>
void test_geometry(std::string const& wkt, std::string const& expected)
{

    Box box;
    bg::read_wkt(wkt, box);

    typedef bg::box_range<Box> range_type;
    range_type range(box);

    {
        std::ostringstream out;
        for (typename boost::range_iterator<range_type>::type it = boost::begin(range);
            it != boost::end(range); ++it)
        {
            out << " " << bg::get<0>(*it) << bg::get<1>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), expected);
    }

    {
        // Check forward/backward behaviour
        typename boost::range_iterator<range_type>::type it = boost::begin(range);
        it++;
        it--;
        // Not verified further, same as segment
    }

    {
        // Check random access behaviour
        int const n = boost::size(range);
        BOOST_CHECK_EQUAL(n, 5);
    }

    // Check Boost.Range concept
    BOOST_CONCEPT_ASSERT( (boost::ForwardRangeConcept<range_type>) );
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::box<P> >("polygon((1 1,2 2))", " 11 12 22 21 11");
    test_geometry<bg::model::box<P> >("polygon((3 3,5 5))", " 33 35 55 53 33");
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    return 0;
}
