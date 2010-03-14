// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_DISSOLVE_HPP
#define BOOST_GEOMETRY_TEST_DISSOLVE_HPP

#include <fstream>

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/dissolve.hpp>

// To check results
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif



template <typename Geometry>
void test_dissolve(std::string const& caseid, Geometry const& geometry,
        std::size_t expected_hole_count, std::size_t expected_point_count,
        double expected_area, double percentage)
{
	namespace bg = boost::geometry;
    typedef typename bg::coordinate_type<Geometry>::type coordinate_type;

	std::vector<Geometry> dissolved_vector;
	bg::dissolve_inserter<Geometry>(geometry, std::back_inserter(dissolved_vector));

	typename bg::area_result<Geometry>::type area = 0;
	std::size_t holes = 0;
	std::size_t count = 0;

	BOOST_FOREACH(Geometry& dissolved, dissolved_vector)
	{
	    bg::unique(dissolved);

		area += bg::area(dissolved);
		holes += boost::geometry::num_interior_rings(dissolved);
		count += boost::geometry::num_points(dissolved);
	}

    BOOST_CHECK_MESSAGE(count == expected_point_count,
            "dissolve: " << caseid
            << " #points expected: " << expected_point_count
            << " detected: " << count
            << " type: " << string_from_type<coordinate_type>::name()
            );


    BOOST_CHECK_EQUAL(holes, expected_hole_count);
    BOOST_CHECK_CLOSE(area, expected_area, percentage);

	// Compile check, it should also compile inplace, outputting to the same geometry
	{
		Geometry dissolved;
		bg::dissolve(geometry, dissolved);
	}


#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "dissolve_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        boost::geometry::svg_mapper
            <
                typename boost::geometry::point_type<Geometry>::type
            > mapper(svg, 500, 500);
        mapper.add(geometry);

        mapper.map(geometry, "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");
		BOOST_FOREACH(Geometry& dissolved, dissolved_vector)
		{
		   mapper.map(dissolved, "opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:5");
	    }
	}
#endif
}


template <typename Geometry>
void test_one(std::string const& caseid, std::string const& wkt,
        std::size_t expected_hole_count, std::size_t expected_point_count,
        double expected_area, double percentage = 0.001)
{
    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);

    test_dissolve(caseid, geometry,
        expected_hole_count, expected_point_count,
        expected_area, percentage);
}



#endif
