// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>
#include <boost/geometry/extensions/algorithms/connect.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>
#include <boost/geometry/multi/algorithms/area.hpp>
#include <boost/geometry/multi/algorithms/length.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/unique.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/strategies/strategies.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/multi/algorithms/envelope.hpp>
#  include <boost/geometry/multi/core/is_multi.hpp>
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif


template <typename GeometryOut, typename Geometry>
void test_connect(std::string const& caseid, Geometry const& geometry,
        std::size_t expected_point_count,
        double expected_length, double percentage)
{
	namespace bg = boost::geometry;
    typedef typename bg::coordinate_type<Geometry>::type coordinate_type;

	std::vector<GeometryOut> connected_vector;
	bg::connect(geometry, connected_vector);

	typename bg::length_result<Geometry>::type length = 0;
	std::size_t count = 0;

	BOOST_FOREACH(GeometryOut& connected, connected_vector)
	{
	    bg::unique(connected);
		length += bg::length(connected);
		count += bg::num_points(connected);
	}

    BOOST_CHECK_MESSAGE(count == expected_point_count,
            "connect: " << caseid
            << " #points expected: " << expected_point_count
            << " detected: " << count
            << " type: " << string_from_type<coordinate_type>::name()
            );


    //BOOST_CHECK_EQUAL(holes, expected_hole_count);
    BOOST_CHECK_CLOSE(length, expected_length, percentage);


#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "connect_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper
            <
                typename bg::point_type<Geometry>::type
            > mapper(svg, 500, 500);
        mapper.add(geometry);

        mapper.map(geometry, "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");
		BOOST_FOREACH(GeometryOut& connected, connected_vector)
		{
		   mapper.map(connected, "opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:5");
	    }
	}
#endif
}


template <typename Geometry, typename GeometryOut>
void test_one(std::string const& caseid, std::string const& wkt,
        std::size_t expected_point_count,
        double expected_length, double percentage = 0.001)
{
    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);

    test_connect<GeometryOut>(caseid, geometry,
        expected_point_count,
        expected_length, percentage);

#ifdef BOOST_GEOMETRY_TEST_MULTI_PERMUTATIONS
    // Test different combinations of a multi-polygon

    int n = geometry.size();

    // test them in all orders
    std::vector<int> indices;
    for (int i = 0; i < n; i++)
    {
        indices.push_back(i);
    }
    int permutation = 0;
    do
    {
        std::ostringstream out;
        out << caseid;
        Geometry geometry2;
        for (int i = 0; i < n; i++)
        {
            int index = indices[i];
            out << "_" << index;
            geometry2.push_back(geometry[index]);
        }
        test_connect<GeometryOut>(out.str(), geometry2, 
                expected_point_count, expected_length, percentage);
    } while (std::next_permutation(indices.begin(), indices.end()));
#endif

}




template <typename P>
void test_all()
{
    namespace bg = boost::geometry;


    typedef bg::linestring<P> linestring;
    typedef bg::multi_linestring<linestring> multi_linestring;

    test_one<multi_linestring, linestring>("ls_simplex",
        "MULTILINESTRING((0 0,1 1),(1 1,2 2))",
        3, 2 * std::sqrt(2.0));
    
    // Opposites, forming one line
    test_one<multi_linestring, linestring>("ls_simplex_opposite_to",
        "MULTILINESTRING((0 0,1 1),(2 2,1 1))",
        3, 2 * std::sqrt(2.0));
    test_one<multi_linestring, linestring>("ls_simplex_opposite_from",
        "MULTILINESTRING((1 1,0 0),(1 1,2 2))",
        3, 2 * std::sqrt(2.0));

    // Two output linestrings
    test_one<multi_linestring, linestring>("ls_simplex_two",
        "MULTILINESTRING((0 0,1 1),(1 1,2 2),(3 3,4 4),(4 4,5 5))",
        6, 4 * std::sqrt(2.0));

    // Linestrings forming a ring
    test_one<multi_linestring, linestring>("ls_simplex_ring",
        "MULTILINESTRING((0 0,0 1),(1 1,1 0),(0 1,1 1),(1 0,0 0))",
        5, 4.0);

    // disconnected ring
    test_one<multi_linestring, linestring>("ls_disconnected_ring",
        "MULTILINESTRING((0 0,0 1.01),(1.02 1.03,0.99 0),(0 0.98,1.001 1),(1.01 0,0 0))",
        5, 4.0);
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
