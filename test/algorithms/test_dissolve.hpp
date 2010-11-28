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
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif



template <typename GeometryOut, typename Geometry>
void test_dissolve(std::string const& caseid, Geometry const& geometry,
        std::size_t expected_hole_count, std::size_t expected_point_count,
        double expected_length_or_area, double percentage)
{
    typedef typename bg::coordinate_type<Geometry>::type coordinate_type;

    static const bool is_line = bg::geometry_id<GeometryOut>::type::value == 2;

    std::vector<GeometryOut> dissolved_vector;
    bg::dissolve_inserter<GeometryOut>(geometry, std::back_inserter(dissolved_vector));

    typename bg::area_result<Geometry>::type length_or_area = 0;
    //std::size_t holes = 0;
    std::size_t count = 0;

    BOOST_FOREACH(GeometryOut& dissolved, dissolved_vector)
    {
        bg::unique(dissolved);


        length_or_area +=
            is_line ? bg::length(dissolved) : bg::area(dissolved);

        //holes += bg::num_interior_rings(dissolved);
        count += bg::num_points(dissolved);
    }

    BOOST_CHECK_MESSAGE(count == expected_point_count,
            "dissolve: " << caseid
            << " #points expected: " << expected_point_count
            << " detected: " << count
            << " type: " << string_from_type<coordinate_type>::name()
            );


    //BOOST_CHECK_EQUAL(holes, expected_hole_count);
    BOOST_CHECK_CLOSE(length_or_area, expected_length_or_area, percentage);

    // Compile check, it should also compile inplace, outputting to the same geometry
    {
        std::vector<GeometryOut> dissolved;
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

        bg::svg_mapper
            <
                typename bg::point_type<Geometry>::type
            > mapper(svg, 500, 500);
        mapper.add(geometry);

        mapper.map(geometry, "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");
        BOOST_FOREACH(GeometryOut& dissolved, dissolved_vector)
        {
           mapper.map(dissolved, "opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:5");
        }
    }
#endif
}


template <typename Geometry, typename GeometryOut>
void test_one(std::string const& caseid, std::string const& wkt,
        std::size_t expected_hole_count, std::size_t expected_point_count,
        double expected_length_or_area, double percentage = 0.001)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);

    test_dissolve<GeometryOut>(caseid, geometry,
        expected_hole_count, expected_point_count,
        expected_length_or_area, percentage);

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
        test_dissolve<GeometryOut>(out.str(), geometry2, expected_hole_count,
                expected_point_count, expected_length_or_area, percentage);
    } while (std::next_permutation(indices.begin(), indices.end()));
#endif

}



#endif
