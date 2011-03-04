// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_OVERLAY_P_Q_HPP
#define BOOST_GEOMETRY_TEST_OVERLAY_P_Q_HPP

#include <fstream>
#include <iomanip>

//#define BOOST_GEOMETRY_ROBUSTNESS_USE_DIFFERENCE


// For mixing int/float
#if defined(_MSC_VER)
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#endif


#include <boost/geometry/geometry.hpp>
#include <boost/geometry/multi/multi.hpp>

#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/io/svg/svg_mapper.hpp>

#include <geometry_test_common.hpp>


template <typename OutputType, typename CalculationType, typename G1, typename G2>
static bool test_overlay_p_q(std::string const& caseid,
            G1 const& p, G2 const& q,
            bool svg, double tolerance, bool force_output = false)
{
    bool result = true;

    typedef typename bg::coordinate_type<G1>::type coordinate_type;
    typedef typename bg::point_type<G1>::type point_type;

    bg::model::multi_polygon<OutputType> out_i, out_u, out_d;

    CalculationType area_p = bg::area(p);
    CalculationType area_q = bg::area(q);

    bg::intersection(p, q, out_i);
    CalculationType area_i = abs(bg::area(out_i)); // TEMP abs call, TODO solve this

    bg::union_(p, q, out_u);
    CalculationType area_u = bg::area(out_u);

    double sum = (area_p + area_q) - area_u - area_i;

    bool wrong = std::abs(sum) > tolerance;

#ifdef BOOST_GEOMETRY_ROBUSTNESS_USE_DIFFERENCE
    bg::difference(p, q, out_d);
    CalculationType area_d = bg::area(out_d);
    double sum_d = (area_u - area_q) - area_d;
    bool wrong_d = std::abs(sum_d) > tolerance;
    
    if (wrong_d)
    {
        wrong = true;
    }
#endif


    if (wrong || force_output)
    {
        if (wrong)
        {
            result = false;
            svg = true;
        }

        std::cout
            << "type: " << string_from_type<CalculationType>::name()
            << " id: " << caseid
            << " area i: " << area_i
            << " area u: " << area_u
            << " area p: " << area_p
            << " area q: " << area_q
            << " sum: " << sum
#ifdef BOOST_GEOMETRY_ROBUSTNESS_USE_DIFFERENCE
            << " area d: " << area_d
            << " sum d: " << sum_d
#endif
            << std::endl
            << std::setprecision(20)
            << " p: " << bg::wkt(p) << std::endl
            << " q: " << bg::wkt(q) << std::endl
            ;

    }

    // For POSTGIS output
    /***
    std::cout 
        << "union select " << area_i << " as bg,ST_area(ST_Intersection(ST_GeomFromText('" << bg::wkt(p) << "'), ST_GeomFromText('" << bg::wkt(q) << "'))) as pg" << std::endl
        << "union select " << area_u << " as bg,ST_area(ST_Union(ST_GeomFromText('" << bg::wkt(p) << "'), ST_GeomFromText('" << bg::wkt(q) << "'))) as pg" << std::endl
        << std::endl;
    ***/


    if(svg)
    {
        std::ostringstream filename;
        filename << "overlay_" << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << string_from_type<CalculationType>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<point_type> mapper(svg, 500, 500);

        mapper.add(p);
        mapper.add(q);

        //mapper.add(point_type(0,0));
        //mapper.add(point_type(3,3));

        // Input shapes in green/blue
        mapper.map(p, "fill-opacity:0.5;fill:rgb(153,204,0);"
                "stroke:rgb(153,204,0);stroke-width:3");
        mapper.map(q, "fill-opacity:0.3;fill:rgb(51,51,153);"
                "stroke:rgb(51,51,153);stroke-width:3");

#ifdef BOOST_GEOMETRY_ROBUSTNESS_USE_DIFFERENCE
        for (BOOST_AUTO(it, out_d.begin()); it != out_d.end(); ++it)
        {
            mapper.map(*it, "fill-opacity:0.1;stroke-opacity:0.4;fill:rgb(255,255,0);"
                    "stroke:rgb(255,255,0);stroke-width:4");
        }
#else
        for (BOOST_AUTO(it, out_i.begin()); it != out_i.end(); ++it)
        {
            mapper.map(*it, "fill-opacity:0.1;stroke-opacity:0.4;fill:rgb(255,0,0);"
                    "stroke:rgb(255,0,0);stroke-width:4");
        }
        for (BOOST_AUTO(it, out_u.begin()); it != out_u.end(); ++it)
        {
            mapper.map(*it, "fill-opacity:0.1;stroke-opacity:0.4;fill:rgb(255,0,0);"
                    "stroke:rgb(255,0,255);stroke-width:4");
        }
#endif
    }
    return result;
}

#endif // BOOST_GEOMETRY_TEST_OVERLAY_P_Q_HPP
