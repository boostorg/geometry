// Boost.Geometry (aka GGL, Generic Geometry Library) 
// Unit Test

// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_TEST_BUFFER_HPP
#define BOOST_GEOMETRY_TEST_BUFFER_HPP

#define TEST_WITH_SVG

#include <fstream>
#include <iomanip>

#include <boost/foreach.hpp>
#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/union.hpp>

#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#include <boost/geometry/extensions/algorithms/detail/overlay/dissolver.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/extensions/algorithms/dissolve.hpp>
//#include <boost/geometry/extensions/algorithms/detail/overlay/split_rings.hpp>

//#include <boost/geometry/extensions/algorithms/buffer.hpp>

//#include <boost/geometry/extensions/algorithms/buffer/remove_within_distance.hpp>
#include <boost/geometry/extensions/algorithms/buffer/linestring_buffer.hpp>
#include <boost/geometry/extensions/algorithms/buffer/polygon_buffer.hpp>
//#include <boost/geometry/extensions/algorithms/buffer/unioning_buffer.hpp>
//#include <boost/geometry/extensions/algorithms/buffer/segmenting_buffer.hpp>

#include <boost/geometry/extensions/strategies/buffer.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif


#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
template <typename Geometry, typename Mapper>
void post_map(Geometry const& geometry, Mapper& mapper)
{
    typedef bg::detail::overlay::turn_info
    <
        typename bg::point_type<Geometry>::type
    > turn_info;

    std::vector<turn_info> turns;

    bg::detail::self_get_turn_points::no_interrupt_policy policy;
    bg::self_turns
        <
            bg::detail::overlay::assign_null_policy
        >(geometry, turns, policy);

    BOOST_FOREACH(turn_info const& turn, turns)
    {
        mapper.map(turn.point, "fill:rgb(255,128,0);stroke:rgb(0,0,100);stroke-width:1");
    }
}

template
<
    typename GeometryOut,
    template
        <
            typename
            , typename
#if defined(BOOST_GEOMETRY_DEBUG_WITH_MAPPER)
            , typename
#endif
        > class JoinStrategy,
    typename Geometry
>
void test_buffer(std::string const& caseid, Geometry const& geometry,
            char join,
            double expected_area,
            double distance_left, double distance_right)
{
    namespace bg = boost::geometry;

    typedef typename bg::coordinate_type<Geometry>::type coordinate_type;
    typedef typename bg::point_type<Geometry>::type point_type;
    typedef bg::strategy::buffer::distance_assymetric<coordinate_type> distance;

    typedef typename bg::ring_type<GeometryOut>::type ring_type;

    typedef JoinStrategy
        <
            point_type,
            typename bg::point_type<GeometryOut>::type
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , bg::svg_mapper<point_type>
#endif
        > join_strategy;



    std::vector<GeometryOut> buffered;

#ifdef BOOST_GEOMETRY_TEST_BUFFER_POLYGON
    {
        GeometryOut buffered_step1;
        bg::detail::buffer::polygon_buffer
            <
                Geometry, GeometryOut, join_strategy
            >::apply(geometry, buffered_step1, distance_left, join_strategy());
        buffered.push_back(buffered_step1);
    }
#else
    {
        typedef bg::strategy::buffer::distance_assymetric<coordinate_type> distance;
        typedef bg::detail::buffer::intersecting_inserter
            <
                std::vector<GeometryOut>
            > inserter_type;

        inserter_type inserter(buffered);

        bg::detail::buffer::linestring_buffer
            <
                Geometry, GeometryOut, distance, join_strategy
            >::apply(geometry, inserter, distance(distance_left, distance_left / 2.0), join_strategy());
    }
#endif

    //std::cout << caseid << std::endl;
    //std::cout << "INPUT: " << bg::wkt(geometry) << std::endl;
    //std::cout << "OUTPUT:" << std::endl;
    //BOOST_FOREACH(GeometryOut const& polygon, buffered)
    //{
    //    std::cout << bg::wkt(polygon) << std::endl;
    //}

    {
        std::ostringstream filename;
        filename << "buffer_"
            << (bg::geometry_id<Geometry>::value == 2 ? "line" : "poly") << "_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << "_" << join
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<point_type> mapper(svg, 500, 500);

        BOOST_FOREACH(GeometryOut const& polygon, buffered)
        {
            mapper.add(polygon);
        }

        // Map input geometry in green
        mapper.map(geometry, "opacity:0.5;fill:rgb(0,128,0);stroke:rgb(0,128,0);stroke-width:1");

        BOOST_FOREACH(GeometryOut const& polygon, buffered)
        {
            mapper.map(polygon, "opacity:0.8;fill:none;stroke:rgb(0,0,0);stroke-width:2");
            post_map(polygon, mapper);
        }
    }

}

#ifdef BOOST_GEOMETRY_CHECK_WITH_POSTGIS
static int counter = 0;
#endif

template
<
    typename Geometry,
    template
        <
            typename
            , typename
#if defined(BOOST_GEOMETRY_DEBUG_WITH_MAPPER)
            , typename
#endif
        > class JoinStrategy,
    typename GeometryOut
>
void test_one(std::string const& caseid, std::string const& wkt,
        char join, double expected_area,
        double distance_left, double distance_right = -999)
{
    namespace bg = boost::geometry;
    Geometry g;
    bg::read_wkt(wkt, g);

    typedef typename bg::point_type<Geometry>::type point_type;

    //std::cout << caseid << std::endl;
    if (join == 'm')
    {
        //return;
    }



#ifdef BOOST_GEOMETRY_CHECK_WITH_POSTGIS
    std::cout
        << (counter > 0 ? "union " : "")
        << "select " << counter++
        << ", '" << caseid << "' as caseid"
        << ", ST_Area(ST_Buffer(ST_GeomFromText('" << wkt << "'), "
        << distance_left
        << ", 'endcap=flat join=" << (join == 'm' ? "miter" : "round") << "'))"
        << ", "  << expected_area
        << std::endl;
#endif

    test_buffer<GeometryOut, JoinStrategy>
            (caseid, g, join, expected_area, distance_left, distance_right);
}


#endif
