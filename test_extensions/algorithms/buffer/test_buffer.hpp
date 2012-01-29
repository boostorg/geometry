// Boost.Geometry (aka GGL, Generic Geometry Library) 
// Unit Test

// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_TEST_BUFFER_HPP
#define BOOST_GEOMETRY_TEST_BUFFER_HPP

//#define BOOST_GEOMETRY_DEBUG_WITH_MAPPER
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
        mapper.map(turn.point, "fill:rgb(255,128,0);stroke:rgb(0,0,100);stroke-width:1", 3);
    }
}

template
<
    typename GeometryOut,
    template
        <
            typename
            , typename
            , bool
#if defined(BOOST_GEOMETRY_DEBUG_WITH_MAPPER)
            , typename
#endif
        > class JoinStrategy,
    typename Geometry
>
void test_buffer(std::string const& caseid, Geometry const& geometry,
            char join,
            bool check, double expected_area,
            double distance_left, double distance_right)
{
    namespace bg = boost::geometry;

    typedef typename bg::coordinate_type<Geometry>::type coordinate_type;
    typedef typename bg::point_type<Geometry>::type point_type;
    typedef bg::strategy::buffer::distance_assymetric<coordinate_type> distance;

    typedef typename bg::ring_type<GeometryOut>::type ring_type;

    std::ostringstream complete;
    complete
        << (bg::geometry_id<Geometry>::value == 2 ? "line" : "poly") << "_"
        << caseid << "_"
        << string_from_type<coordinate_type>::name()
        << "_" << join;

    //std::cout << complete.str() << std::endl;

    std::ostringstream filename;
    filename << "buffer_" << complete.str() << ".svg";

    std::ofstream svg(filename.str().c_str());

    bg::svg_mapper<point_type> mapper(svg, 1000, 1000);

    {
        bg::model::box<point_type> box;
        bg::envelope(geometry, box);
        double d = distance_left;
        if (distance_right > 0)
        {
            d += distance_right;
        }

        bg::buffer(box, box, std::abs(d) * 1.1);
        mapper.add(box);
    }



    typedef JoinStrategy
        <
            point_type,
            typename bg::point_type<GeometryOut>::type,
            true
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , bg::svg_mapper<point_type>
#endif
        > join_strategy_type;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    join_strategy_type join_strategy(mapper);
#else
    join_strategy_type join_strategy;
#endif

    typedef bg::strategy::buffer::distance_assymetric<coordinate_type> distance_strategy_type;
    distance_strategy_type distance_strategy(distance_left, distance_left / 2.0); // TODO: distance_right

    std::vector<GeometryOut> buffered;

    GeometryOut buffered_step1;
#ifdef BOOST_GEOMETRY_TEST_BUFFER_POLYGON
    {
        bg::detail::buffer::polygon_buffer
            <
                Geometry, GeometryOut, distance_strategy_type, join_strategy_type
            >::apply(geometry, buffered_step1, 
                            distance_strategy, 
                            join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            , mapper
#endif
                                    );
    }
#else
    {
        bg::detail::buffer::linestring_buffer
            <
                Geometry, GeometryOut, distance_strategy_type, join_strategy_type
            >::apply(geometry, buffered_step1, distance_strategy,
                            join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            , mapper
#endif
                                    );
    }
#endif

    if (boost::contains(complete.str(), "bowl")
     || boost::contains(complete.str(), "aitch")
     || boost::contains(complete.str(), "snake")
     || (boost::contains(complete.str(), "indentation")
            && ! boost::contains(complete.str(), "neg"))
     || (boost::contains(complete.str(), "flower") 
            && boost::contains(complete.str(), "d_r"))
     )
    {
        // We dissolve some controlled cases (already, later we will dissolve all),
        // such that we can detect regressions
        bg::dissolve(buffered_step1, buffered);
        mapper.map(buffered_step1, "opacity:0.3;fill:none;stroke:rgb(0,0,200);stroke-width:2");
    }
    else
    {
        buffered.push_back(buffered_step1);
    }

    double area = 0;
    BOOST_FOREACH(GeometryOut const& polygon, buffered)
    {
        area += bg::area(polygon);
    }

    //std::cout << caseid << " " << distance_left << std::endl;
    //std::cout << "INPUT: " << bg::wkt(geometry) << std::endl;
    //std::cout << "OUTPUT: " << area << std::endl;
    //BOOST_FOREACH(GeometryOut const& polygon, buffered)
    //{
    //    std::cout << bg::wkt(polygon) << std::endl;
    //}


    if (check)
    {
        BOOST_CHECK_MESSAGE
            (
                std::abs(area - expected_area) < 0.01, 
                complete.str() << " not as expected. " 
                << " Expected: "  << expected_area
                << " Detected: "  << area
            );

        // Be sure resulting polygon does not contain
        // self-intersections
        // But indentation5 should contain 1 self-ip
        if (! boost::contains(complete.str(), "indentation5_d_r"))
        {
            BOOST_FOREACH(GeometryOut const& polygon, buffered)
            {
                BOOST_CHECK_MESSAGE
                    (
                        ! bg::intersects(polygon), 
                        complete.str() << " is self-intersecting. " 
                    );
            }
        }
    }



    // Map input geometry in green
    mapper.map(geometry, "opacity:0.5;fill:rgb(0,128,0);stroke:rgb(0,128,0);stroke-width:1");

    BOOST_FOREACH(GeometryOut const& polygon, buffered)
    {
        mapper.map(polygon, "opacity:0.8;fill:none;stroke:rgb(0,0,0);stroke-width:2");
        post_map(polygon, mapper);
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
            , bool
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
            (caseid, g, join, false, expected_area, distance_left, distance_right);
}


template
<
    typename Geometry,
    template
        <
            typename
            , typename
            , bool
#if defined(BOOST_GEOMETRY_DEBUG_WITH_MAPPER)
            , typename
#endif
        > class JoinStrategy,
    typename GeometryOut
>
void test_one(bool check, std::string const& caseid, std::string const& wkt,
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
            (caseid, g, join, check, expected_area, distance_left, distance_right);
}

#endif
