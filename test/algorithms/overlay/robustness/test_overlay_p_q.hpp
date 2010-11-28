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

// For mixing int/float
#if defined(_MSC_VER)
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#endif


#include <boost/geometry/geometry.hpp>

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
    typedef bg::detail::overlay::turn_info<point_type> turn_type;

    typedef bg::strategy_intersection
        <
            typename bg::cs_tag<point_type>::type,
            G1,
            G2,
            turn_type,
            CalculationType
        > strategy;

    std::vector<OutputType> out_i, out_u;

    CalculationType area_p = bg::area(p);
    CalculationType area_q = bg::area(q);

    CalculationType area_i = 0;
    CalculationType area_u = 0;

    bg::intersection_inserter<OutputType>(p, q, std::back_inserter(out_i), strategy());
    for (typename std::vector<OutputType>::iterator it = out_i.begin();
            it != out_i.end();
            ++it)
    {
        area_i += bg::area(*it);
    }
    bg::union_inserter<OutputType>(p, q, std::back_inserter(out_u), strategy());
    for (typename std::vector<OutputType>::iterator it = out_u.begin();
            it != out_u.end();
            ++it)
    {
        area_u += bg::area(*it);
    }

    double diff = (area_p + area_q) - area_u - area_i;
    bool wrong = std::abs(diff) > tolerance;
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
            << " diff: " << diff
            << std::endl
            << std::setprecision(20)
            << " p: " << bg::wkt(p) << std::endl
            << " q: " << bg::wkt(q) << std::endl
            ;

    }


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

        // Input shapes in green/blue
        mapper.map(p, "fill-opacity:0.5;fill:rgb(153,204,0);"
                "stroke:rgb(153,204,0);stroke-width:3");
        mapper.map(q, "fill-opacity:0.3;fill:rgb(51,51,153);"
                "stroke:rgb(51,51,153);stroke-width:3");

        for (typename std::vector<OutputType>::const_iterator it = out_i.begin();
                it != out_i.end(); ++it)
        {
            mapper.map(*it, "fill-opacity:0.1;stroke-opacity:0.4;fill:rgb(255,0,0);"
                    "stroke:rgb(255,0,0);stroke-width:4");
        }
        for (typename std::vector<OutputType>::const_iterator it = out_u.begin();
                it != out_u.end(); ++it)
        {
            mapper.map(*it, "fill-opacity:0.1;stroke-opacity:0.4;fill:rgb(255,0,0);"
                    "stroke:rgb(255,0,255);stroke-width:4");

        }
    }
    return result;
}

#endif // BOOST_GEOMETRY_TEST_OVERLAY_P_Q_HPP
