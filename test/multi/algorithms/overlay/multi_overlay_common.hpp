#ifndef GEOMETRY_TEST_MULTI_OVERLAY_COMMON_HPP
#define GEOMETRY_TEST_MULTI_OVERLAY_COMMON_HPP


// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/domains/gis/io/wkt/read_wkt_multi.hpp>
//#include <boost/geometry/multi/io/svg/write_svg.hpp>



template <typename P, typename Functor, typename T>
void test_all(std::vector<T> const& expected, double precision = 0.01)
{
    typename boost::range_const_iterator<std::vector<T> >::type iterator
        = boost::begin(expected);

    typedef bg::model::multi_polygon<bg::model::polygon<P> > mp;
    typedef bg::model::box<P> box;

    BOOST_ASSERT(iterator != boost::end(expected));
    test_overlay<mp, mp, Functor>("1", *iterator,
            "MULTIPOLYGON(((0 1,2 5,5 3,0 1)),((1 1,5 2,5 0,1 1)))",
            "MULTIPOLYGON(((3 0,0 3,4 5,3 0)))", precision);
    iterator++;
}


#endif // GEOMETRY_TEST_MULTI_OVERLAY_COMMON_HPP
