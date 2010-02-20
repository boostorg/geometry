// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_UNION_HPP
#define BOOST_GEOMETRY_TEST_UNION_HPP

#include <fstream>

#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/union.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif



template <typename OutputType, typename G1, typename G2>
void test_union(std::string const& caseid, G1 const& g1, G2 const& g2,
        std::size_t expected_count, std::size_t expected_hole_count,
        std::size_t expected_point_count, double expected_area,
        double percentage)
{
    typedef typename boost::geometry::coordinate_type<G1>::type coordinate_type;
    std::vector<OutputType> clip;
    boost::geometry::union_inserter<OutputType>(g1, g2, std::back_inserter(clip));

    double area = 0;
    std::size_t n = 0;
    std::size_t holes = 0;
    for (typename std::vector<OutputType>::iterator it = clip.begin();
            it != clip.end(); ++it)
    {
        area += boost::geometry::area(*it);
        holes += boost::geometry::num_interior_rings(*it);

        // Get a correct point-count without duplicate points
        // (note that overlay might be adapted to avoid duplicates)
        OutputType simplified;
        boost::geometry::unique(*it);
        n += boost::geometry::num_points(*it);
    }


    /***
    std::cout << "case: " << caseid
        << " n: " << n
        << " area: " << area
        << " polygons: " << boost::size(clip)
        << " holes: " << holes
        << std::endl;
    ***/

    BOOST_CHECK_MESSAGE(int(n) == expected_point_count,
            "union: " << caseid
            << " #points expected: " << expected_point_count
            << " detected: " << n
            << " type: " << string_from_type<coordinate_type>::name()
            );


    BOOST_CHECK_EQUAL(clip.size(), expected_count);
    BOOST_CHECK_EQUAL(holes, expected_hole_count);
    BOOST_CHECK_CLOSE(area, expected_area, percentage);

#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "union_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        svg_mapper<typename boost::geometry::point_type<G2>::type> mapper(svg, 500, 500);
        mapper.add(g1);
        mapper.add(g2);

        mapper.map(g1, "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");
        mapper.map(g2, "opacity:0.6;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");

        for (typename std::vector<OutputType>::const_iterator it = clip.begin();
                it != clip.end(); ++it)
        {
            mapper.map(*it, "opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:5");
        }
    }
#endif
}

template <typename OutputType, typename G1, typename G2>
void test_one(std::string const& caseid, std::string const& wkt1, std::string const& wkt2,
        std::size_t expected_count, std::size_t expected_hole_count,
        std::size_t expected_point_count, double expected_area,
        double percentage = 0.001)
{
    G1 g1;
    boost::geometry::read_wkt(wkt1, g1);

    G2 g2;
    boost::geometry::read_wkt(wkt2, g2);

    test_union<OutputType>(caseid, g1, g2,
        expected_count, expected_hole_count, expected_point_count,
        expected_area, percentage);
}



#endif
