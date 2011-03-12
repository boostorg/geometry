// Boost.Geometry (aka GGL, Generic Box Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <geometry_test_common.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/iterators/box_iterator.hpp>
#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>

#include <test_geometries/custom_segment.hpp>


template <typename Box>
void test_geometry(std::string const& wkt, std::string const& expected)
{
    Box box;
    bg::read_wkt(wkt, box);

    {
        std::ostringstream out;
        bg::box_iterator<Box> it(box), end(box, true);
        for ( ; it != end; ++it)
        {
            out << " " << bg::get<0>(*it) << bg::get<1>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), expected);
    }
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
