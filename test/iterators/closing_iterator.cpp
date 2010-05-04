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

#include <boost/geometry/iterators/closing_iterator.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>







template <typename Geometry>
void test_geometry(std::string const& wkt)
{
    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);
    typedef boost::geometry::closing_iterator<Geometry const> closing_iterator;


    // 1. Test normal behaviour
    {
        closing_iterator it(geometry);
        closing_iterator end(geometry, true);

        std::ostringstream out;
        for (; it != end; ++it)
        {
            out << " " << boost::geometry::get<0>(*it) << boost::geometry::get<1>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), " 11 14 44 41 11");

        // All the following does NOT compile, and should NOT, 
        // 1) Because it is forward only:
        //it--;
        //--it;
        // 2) Because it is not random access:
        //it += 2;
        //it = boost::begin(geometry);
    }


    // 2: check copy behaviour
    {
        typedef typename boost::range_iterator<Geometry const>::type normal_iterator;
        Geometry copy;

        std::copy<closing_iterator>(
            closing_iterator(geometry),
            closing_iterator(geometry, true), 
            std::back_inserter(copy));

        std::ostringstream out;
        for (normal_iterator cit = boost::begin(copy); cit != boost::end(copy); ++cit)
        {
                out << " " << boost::geometry::get<0>(*cit) << boost::geometry::get<1>(*cit);
        }
        BOOST_CHECK_EQUAL(out.str(), " 11 14 44 41 11");
    }
}


template <typename P>
void test_all()
{
    test_geometry<boost::geometry::linear_ring<P> >("POLYGON((1 1,1 4,4 4,4 1))");
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_2d>();

    return 0;
}
