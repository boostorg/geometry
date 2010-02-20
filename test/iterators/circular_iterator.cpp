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

#include <ggl_test_common.hpp>

#include <boost/geometry/iterators/circular_iterator.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>

template <typename G>
void test_geometry(const std::string& wkt)
{
    G geo;
    boost::geometry::read_wkt(wkt, geo);
    typedef typename boost::range_const_iterator<G>::type normal_iterator;
    typedef boost::geometry::circular_iterator<normal_iterator> circular_iterator;

    circular_iterator end(boost::end(geo));

    {
        // 1: normal start position
        normal_iterator start = boost::begin(geo);

        std::ostringstream out;
        for (circular_iterator it(boost::begin(geo), boost::end(geo), start);
             it != end; ++it)
        {
            out << boost::geometry::get<0>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), "12345");
    }

    {
        // 2: start somewhere in the middle
        normal_iterator start = boost::begin(geo) + 2;

        std::ostringstream out;
        for (circular_iterator it(boost::begin(geo), boost::end(geo), start);
             it != end; ++it)
        {
            out << boost::geometry::get<0>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), "34512");
    }

    {
        // 3: start at end position
        normal_iterator start = boost::begin(geo) + boost::size(geo) - 1;

        std::ostringstream out;
        for (circular_iterator it(boost::begin(geo), boost::end(geo), start);
             it != end; ++it)
        {
            out << boost::geometry::get<0>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), "51234");
    }

    {
        // 4: check copy behaviour
        G copy;

        {
            normal_iterator start = boost::begin(geo) + 2;
            circular_iterator it(boost::begin(geo), boost::end(geo), start);
            std::copy<circular_iterator>(it, end, std::back_inserter(copy));
        }

        std::ostringstream out;
        for (normal_iterator cit = boost::begin(copy); cit != boost::end(copy); ++cit)
        {
            out << boost::geometry::get<0>(*cit);
        }
        BOOST_CHECK_EQUAL(out.str(), "34512");
    }
}

template <typename P>
void test_all()
{
    test_geometry<boost::geometry::linestring<P> >("linestring(1 1,2 2,3 3,4 4,5 5)");
}

int test_main(int, char* [])
{
    test_all<boost::geometry::point_2d>();

    return 0;
}
