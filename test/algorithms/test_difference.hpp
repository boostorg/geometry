// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_DIFFERENCE_HPP
#define BOOST_GEOMETRY_TEST_DIFFERENCE_HPP

#include <fstream>
#include <iomanip>

#include <boost/foreach.hpp>
#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>
#include <boost/geometry/multi/algorithms/difference.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/geometries/geometries.hpp>


#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif




template <typename OutputType, typename CalculationType, typename G1, typename G2>
void test_difference(std::string const& caseid, G1 const& g1, G2 const& g2,
        std::size_t expected_count, std::size_t expected_point_count,
        double expected_area,
        double percentage = 0.0001,
        bool sym = false)
{
    std::vector<OutputType> clip;

    typedef typename bg::coordinate_type<G1>::type coordinate_type;
    typedef typename bg::point_type<G1>::type point_type;

    if (sym)
    {
        bg::sym_difference(g1, g2, clip);
    }
    else
    {
        bg::difference(g1, g2, clip);
    }

    double area = 0;
    std::size_t n = 0;
    for (typename std::vector<OutputType>::iterator it = clip.begin();
            it != clip.end();
            ++it)
    {
        if (expected_point_count > 0)
        {
            // Get a correct point-count without duplicate points
            // (note that overlay might be adapted to avoid duplicates)
            bg::unique(*it);
            n += bg::num_points(*it);
        }

        area += bg::area(*it);
    }


#if ! defined(BOOST_GEOMETRY_NO_BOOST_TEST)
    /*if (expected_point_count > 0)
    {
        BOOST_CHECK_MESSAGE(n == expected_point_count,
                "difference: " << caseid
                << " #points expected: " << expected_point_count
                << " detected: " << n
                << " type: " << string_from_type<coordinate_type>::name()
                );
    }*/

    if (expected_count > 0)
    {
        BOOST_CHECK_MESSAGE(clip.size() == expected_count,
                "difference: " << caseid
                << " #outputs expected: " << expected_count
                << " detected: " << clip.size()
                << " type: " << string_from_type<coordinate_type>::name()
                );
    }

    BOOST_CHECK_CLOSE(area, expected_area, percentage);
#endif


#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "difference_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << string_from_type<CalculationType>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<point_type> mapper(svg, 500, 500);

        mapper.add(g1);
        mapper.add(g2);

        mapper.map(g1, "fill-opacity:0.3;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:3");
        mapper.map(g2, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:3");

        for (typename std::vector<OutputType>::const_iterator it = clip.begin();
                it != clip.end(); ++it)
        {
            mapper.map(*it, 
                //sym ? "fill-opacity:0.2;stroke-opacity:0.4;fill:rgb(255,255,0);stroke:rgb(255,0,255);stroke-width:8" :
                "fill-opacity:0.2;stroke-opacity:0.4;fill:rgb(255,0,0);stroke:rgb(255,0,255);stroke-width:8");
        }
    }
#endif
}


#ifdef BOOST_GEOMETRY_CHECK_WITH_POSTGIS
static int counter = 0;
#endif


template <typename OutputType, typename G1, typename G2>
void test_one(std::string const& caseid,
        std::string const& wkt1, std::string const& wkt2,
        std::size_t expected_count1,
        std::size_t expected_point_count1,
        double expected_area1,

        std::size_t expected_count2,
        std::size_t expected_point_count2,
        double expected_area2,

        double percentage = 0.0001)
{
    G1 g1;
    bg::read_wkt(wkt1, g1);

    G2 g2;
    bg::read_wkt(wkt2, g2);

    bg::correct(g1);
    bg::correct(g2);

    test_difference<OutputType, void>(caseid + "_a", g1, g2,
        expected_count1, expected_point_count1,
        expected_area1, percentage);
    test_difference<OutputType, void>(caseid + "_b", g2, g1,
        expected_count2, expected_point_count2,
        expected_area2, percentage);
    test_difference<OutputType, void>(caseid + "_s", g1, g2,
        expected_count1 + expected_count2,
        expected_point_count1 + expected_point_count2,
        expected_area1 + expected_area2,
        percentage, true);

#ifdef BOOST_GEOMETRY_CHECK_WITH_POSTGIS
    std::cout
        << (counter > 0 ? "union " : "")
        << "select " << counter++
        << ", '" << caseid << "' as caseid"
        << ", ST_NumPoints(ST_Difference(ST_GeomFromText('" << wkt1 << "'), "
        << "      ST_GeomFromText('" << wkt2 << "'))) "
        << ", ST_NumGeometries(ST_Difference(ST_GeomFromText('" << wkt1 << "'), "
        << "      ST_GeomFromText('" << wkt2 << "'))) "
        << ", ST_Area(ST_Difference(ST_GeomFromText('" << wkt1 << "'), "
        << "      ST_GeomFromText('" << wkt2 << "'))) "
        //<< ", " << expected_area1 << " as expected_area_a"
        //<< ", " << expected_count1 << " as expected_count_a"
        << ", ST_NumPoints(ST_Difference(ST_GeomFromText('" << wkt2 << "'), "
        << "      ST_GeomFromText('" << wkt1 << "'))) "
        << ", ST_NumGeometries(ST_Difference(ST_GeomFromText('" << wkt2 << "'), "
        << "      ST_GeomFromText('" << wkt1 << "'))) "
        << ", ST_Area(ST_Difference(ST_GeomFromText('" << wkt2 << "'), "
        << "      ST_GeomFromText('" << wkt1 << "'))) "
        //<< ", " << expected_area2 << " as expected_area_b"
        //<< ", " << expected_count2 << " as expected_count_b"
        << ", ST_NumPoints(ST_SymDifference(ST_GeomFromText('" << wkt1 << "'), "
        << "      ST_GeomFromText('" << wkt2 << "'))) "
        << ", ST_NumGeometries(ST_SymDifference(ST_GeomFromText('" << wkt1 << "'), "
        << "      ST_GeomFromText('" << wkt2 << "'))) "
        << ", ST_Area(ST_SymDifference(ST_GeomFromText('" << wkt1 << "'), "
        << "      ST_GeomFromText('" << wkt2 << "'))) "
        //<< ", " << expected_area1 + expected_area2 << " as expected_area_s"
        //<< ", " << expected_count1 + expected_count2 << " as expected_count_s"
        << std::endl;
#endif

}



#endif
