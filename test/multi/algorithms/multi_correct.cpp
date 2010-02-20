// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/correct.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/multi/algorithms/correct.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>


template <typename Geometry>
void test_geometry(std::string const& wkt, std::string const& expected)
{
    Geometry geometry;

    boost::geometry::read_wkt(wkt, geometry);
    boost::geometry::correct(geometry);

    std::ostringstream out;
    out << boost::geometry::wkt(geometry);

    BOOST_CHECK_EQUAL(out.str(), expected);
}

template <typename P>
void test_all()
{
    typedef boost::geometry::multi_polygon<boost::geometry::polygon<P> > cw_type;
    std::string cw_mp =
            "MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)))";
    test_geometry<cw_type>(cw_mp, cw_mp);

    test_geometry<cw_type>("MULTIPOLYGON(((0 0,1 0,1 1,0 1,0 0)))",
        cw_mp);
}

int test_main( int , char* [] )
{
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
