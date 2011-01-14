// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_BUFFER_HPP
#define BOOST_GEOMETRY_TEST_BUFFER_HPP


//#define BOOST_GEOMETRY_DEBUG_WITH_MAPPER
//#define BOOST_GEOMETRY_DEBUG_SPLIT_RINGS

//#define BOOST_GEOMETRY_CHECK_WITH_POSTGIS
//#define BOOST_GEOMETRY_DEBUG_ASSEMBLE
//#define BOOST_GEOMETRY_DEBUG_IDENTIFIER

//#undef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
//#undef TEST_WITH_SVG


#if defined(BOOST_GEOMETRY_DEBUG_WITH_MAPPER)
#define TEST_WITH_SVG
#endif

#include <fstream>
#include <boost/detail/iomanip.hpp>

#include <boost/foreach.hpp>
#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/union.hpp>


#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/dissolver.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/dissolve.hpp>
#include <boost/geometry/algorithms/detail/overlay/split_rings.hpp>

#include <boost/geometry/algorithms/buffer.hpp>

#include <boost/geometry/extensions/algorithms/buffer/remove_within_distance.hpp>
#include <boost/geometry/extensions/algorithms/buffer/linestring_buffer.hpp>
#include <boost/geometry/extensions/algorithms/buffer/polygon_buffer.hpp>
//#include <boost/geometry/extensions/algorithms/buffer/unioning_buffer.hpp>
#include <boost/geometry/extensions/algorithms/buffer/segmenting_buffer.hpp>

#include <boost/geometry/strategies/buffer.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>



#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif



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


    typedef bg::detail::buffer::intersecting_inserter
        <
            std::vector<GeometryOut>
        > inserter_type;


    /***
    typedef bg::model::box<point_type> box_type;
    typedef bg::sections<box_type, 1> sections_type;

    sections_type sections;
    bg::sectionalize(geometry, sections);

    std::vector<GeometryOut> sections_buffered;


    // Buffer all sections separately
    BOOST_FOREACH(typename sections_type::value_type const& section, sections)
    {
        if (! section.duplicate)
        {
            typedef typename boost::range_iterator
                <
                    typename bg::range_type<Geometry>::type const
                >::type iterator_type;


            inserter_type inserter(sections_buffered);

            iterator_type begin, end;
            typedef std::pair<iterator_type, iterator_type> section_range;
            bg::get_section(geometry, section, begin, end);
            bg::detail::buffer::linestring_buffer
                <
                    section_range, ring_type, distance, join_strategy
                >::apply(std::make_pair(begin, end), inserter,
                            distance(distance_left, distance_left / 2.0), // two times left
                            join_strategy());
        }
    }

    std::vector<GeometryOut> sections_buffered_unioned;
    BOOST_FOREACH(GeometryOut const& p, sections_buffered)
    {
        if (sections_buffered_unioned.empty())
        {
            bg::union_inserter<GeometryOut>(geometry, p, std::back_inserter(sections_buffered_unioned));
        }
        else if (boost::size(sections_buffered_unioned) == 1)
        {
            std::vector<GeometryOut> step;
            bg::union_inserter<GeometryOut>(sections_buffered_unioned.front(), p, std::back_inserter(step));
            step.swap(sections_buffered_unioned);
        }
        else
        {
            std::cout << "nyi" << std::endl;
            BOOST_FOREACH(GeometryOut const& sbu, sections_buffered_unioned)
            {
                bg::union_inserter<GeometryOut>(p, sbu, sections_buffered_unioned);
            }
        }
    }
    ***/


    std::vector<GeometryOut> buffered;
    inserter_type inserter(buffered);


#if ! defined(TEST_WITH_SVG)

    #if defined(BOOST_GEOMETRY_TEST_BUFFER_POLYGON)
    GeometryOut buffered_step1;
    bg::detail::buffer::polygon_buffer
        <
            Geometry, Geometry, join_strategy
        >::apply(geometry, buffered_step1, distance_left, join_strategy());

    //bg::dissolve_inserter<GeometryOut>(buffered_step1, std::back_inserter(buffered));
    buffered.push_back(buffered_step1);
    #else
    /*bg::detail::buffer::linestring_buffer
        <
            Geometry, GeometryOut, distance, join_strategy
        >::apply(geometry, inserter,
                    distance(distance_left, distance_right),
                    join_strategy());
    */
    #endif

#else

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

        //inserter_type inserter(buffered);

        // Display including a margin
        bg::model::box<point_type> extent;
        bg::envelope(geometry, extent);
        bg::buffer(extent, extent, distance_left * 1.01);
        mapper.add(extent);


        std::vector<GeometryOut> sections_buffered_unioned;


#if defined(BOOST_GEOMETRY_TEST_BUFFER_POLYGON)
/*
        bg::detail::buffer::unioning_buffer(geometry, sections_buffered_unioned,
                distance(distance_left, distance_left / 2.0)
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            , join_strategy(mapper), mapper
#else
                            , join_strategy()
#endif
                );
*/

        Geometry buffered_step1;
        bg::detail::buffer::polygon_buffer
            <
                Geometry, Geometry, join_strategy
            >::apply(geometry, buffered_step1, distance_left
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                        , join_strategy(mapper), mapper
#else
                        , join_strategy()
#endif
                        );

        //bg::dissolve_inserter<GeometryOut>(buffered_step1, std::back_inserter(buffered));
        buffered.push_back(buffered_step1);

        #else


        bg::detail::buffer::segmenting_buffer(geometry, sections_buffered_unioned,
                distance(distance_left, distance_right)
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            , join_strategy(mapper), mapper
#else
                            , join_strategy()
#endif
                );

#ifdef OLD_APPROACH
        bg::detail::buffer::linestring_buffer
            <
                Geometry, GeometryOut, distance, join_strategy
            >::apply(geometry, inserter,
                        distance(distance_left, distance_right)
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                        , join_strategy(mapper), mapper
#else
                        , join_strategy()
#endif
                        );
#endif
        #endif

        // Map input geometry in green
        mapper.map(geometry, "opacity:0.5;fill:rgb(0,255,0);stroke:rgb(0,255,0);stroke-width:1");

#ifdef OLD_APPROACH
        std::vector<ring_type> rings;
        BOOST_FOREACH(GeometryOut const& polygon, buffered)
        {
//std::cout << bg::wkt(polygon) << " ; POLYGON" << std::endl;
            bg::split_rings(polygon, rings);
        }

/*
        BOOST_FOREACH(ring_type const& ring, rings)
        {
            mapper.map(ring,
                bg::area(ring) > 0
                ? "opacity:0.5;fill:none;stroke:rgb(255,0,0);stroke-width:8"
                : "opacity:0.5;fill:none;stroke:rgb(0,0,255);stroke-width:8"
                );
std::cout << bg::wkt(ring)
    << " ; " << bg::area(ring)
    << " " << ring.size()
    << std::endl;
        }
*/

        std::vector<GeometryOut> buffered_and_unioned;
        bg::dissolver(rings, buffered_and_unioned);

        std::vector<GeometryOut> buffered_and_assembled;
        bg::detail::overlay::assemble<GeometryOut>(buffered_and_unioned,
                std::map<bg::ring_identifier, int>(),
                buffered_and_unioned[0], buffered_and_unioned[0], 1, true, true,
                std::back_inserter(buffered_and_assembled));

        // Map buffer in green
        BOOST_FOREACH(GeometryOut const& p, buffered_and_assembled)
        {
            mapper.map(p, "opacity:0.8;fill:none;stroke:rgb(0,64,0);stroke-width:2");
        }
        buffered.swap(buffered_and_assembled);
#endif


        BOOST_FOREACH(GeometryOut const& p, sections_buffered_unioned)//sections_buffered)
        {
            mapper.map(p, "opacity:0.5;fill:rgb(255,255,128);stroke:rgb(0,64,0);stroke-width:2");
        }



}
#endif

    /***
    coordinate_type a = coordinate_type();
    BOOST_FOREACH(GeometryOut const& polygon, buffered)
    {
        a += bg::area(polygon);
    }
    BOOST_CHECK_CLOSE(a, expected_area, join == 'r'
        ? coordinate_type(0.1)
        : coordinate_type(0.001));
    ***/
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
