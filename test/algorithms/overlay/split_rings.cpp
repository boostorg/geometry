// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/detail/overlay/split_rings.hpp>
#include <boost/geometry/algorithms/area.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif


struct test_split_rings
{
    template <typename Geometry>
    static void apply(std::string const& id,
            Geometry const& geometry, 
            int expected_ring_count,
            double precision)
    {
        namespace bg = boost::geometry;

        typedef typename bg::ring_type<Geometry>::type ring_type;
        std::vector<ring_type> rings;
        bg::split_rings(geometry, rings);

        BOOST_CHECK_EQUAL(rings.size(), expected_ring_count);


#if defined(TEST_WITH_SVG)
        {
            std::ostringstream filename;
            filename << "split_rings_" << id << ".svg";

            std::ofstream svg(filename.str().c_str());

            bg::svg_mapper<typename bg::point_type<Geometry>::type> mapper(svg, 500, 500);
            mapper.add(geometry);

            mapper.map(geometry, "fill:rgb(255,255,128);stroke:rgb(0,0,0);stroke-width:1");

            BOOST_FOREACH(ring_type const& ring, rings)
            {
                std::string style = "opacity:0.6;fill:rgb";
                std::string color = bg::area(ring) > 0 ? "(255,0,0)" : "(0,0,255)";
                mapper.map(ring, style + color + ";stroke:rgb(128,128,128);stroke-width:3");
            }
        }
#endif
    }
};


template <typename Geometry>
void test_geometry(std::string const& caseid, 
            std::string const& wkt, 
            int expected_ring_count, double precision = 0.001)
{
    if (wkt.empty())
    {
        return;
    }

    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);

    test_split_rings::apply(caseid, geometry, expected_ring_count, precision);
}


#if ! defined(GEOMETRY_TEST_MULTI)

template <typename P>
void test_self_all()
{
    typedef boost::geometry::polygon<P> polygon;

    // Simplex
    
    test_geometry<polygon>("1", "POLYGON((0 0,0 10,10 10,5 2,5 8,10 0,0 0))", 2);

    // From "mill" (flower)
    test_geometry<polygon>("2", 
        "POLYGON((5 6.10557,3.64223 8.82111,4.28284 8.71716,2.28284 6.71716,2.17889 7.35777,4.89443 6,2.17889 4.64223,2.28284 5.28284,4.28284 3.28284,3.62547 3.14045,5 6.10557))", 
        5);

    test_geometry<polygon>("3", 
        "POLYGON((0.6 10.4,9.4 10.4,9.4 -0.4,7.59 -0.4,4.59 5.33667,4.25383 5.6797,2.67526 4.89042,4.28284 3.28284,3.62547 3.14045,5.15 7.20587,5.15 -0.4,0.6 -0.4,0.6 10.4))", 
        4);
    
    test_geometry<polygon>("4", 
        "POLYGON((0.6 10.4,9.4 10.4,9.4 -0.4,4.59 -0.4,4.59 5.5,4.70431 5.22003,3.93716 6.00284,5 7.06569,6.06284 6.00284,5.29569 5.22003,5.41 5.5,5.41 0.01,5.01 0.41,5.25 0.41,4.85 0.01,4.85 7.20587,6.37453 3.14045,5.71716 3.28284,7.71716 5.28284,7.82111 4.64223,5.10557 6,7.82111 7.35777,7.71716 6.71716,5.71716 8.71716,6.35777 8.82111,5 6.10557,3.64223 8.82111,4.28284 8.71716,2.28284 6.71716,2.17889 7.35777,4.89443 6,2.17889 4.64223,2.28284 5.28284,4.28284 3.28284,3.62547 3.14045,5.15 7.20587,5.15 -0.4,0.6 -0.4,0.6 10.4))", 
        21);
}
#endif



#if ! defined(GEOMETRY_TEST_MULTI)
int test_main(int, char* [])
{
    test_self_all<boost::geometry::point_xy<double> >();
    return 0;
}
#endif
