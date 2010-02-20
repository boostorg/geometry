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

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

#include <geometry_test_common.hpp>


template <typename OutputType, typename CalculationType, typename G1, typename G2>
static bool test_overlay_p_q(std::string const& caseid, G1 const& p, G2 const& q, bool svg, double tolerance)
{
    bool result = true;

    typedef typename boost::geometry::coordinate_type<G1>::type coordinate_type;
    typedef typename boost::geometry::point_type<G1>::type point_type;
    typedef boost::geometry::detail::intersection::intersection_point<point_type> ip_type;

    typedef boost::geometry::strategy_intersection
        <
            typename boost::geometry::cs_tag<point_type>::type,
            G1,
            G2,
            ip_type,
            CalculationType
        > strategy;

    std::vector<OutputType> out_i, out_u;

    CalculationType area_p = boost::geometry::area(p);
    CalculationType area_q = boost::geometry::area(q);

    CalculationType area_i = 0;
    CalculationType area_u = 0;

    boost::geometry::intersection_inserter<OutputType>(p, q, std::back_inserter(out_i), strategy());
    for (typename std::vector<OutputType>::iterator it = out_i.begin();
            it != out_i.end();
            ++it)
    {
        area_i += boost::geometry::area(*it);
    }
    boost::geometry::union_inserter<OutputType>(p, q, std::back_inserter(out_u), strategy());
    for (typename std::vector<OutputType>::iterator it = out_u.begin();
            it != out_u.end();
            ++it)
    {
        area_u += boost::geometry::area(*it);
    }

    double diff = (area_p + area_q) - area_u - area_i;
    if (std::abs(diff) > tolerance
        // for creating SVG-selection: || area_i > 2.0e+006
        )
    {
        result = false;
        svg = true;

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
            << " p: " << boost::geometry::wkt(p) << std::endl
            << " q: " << boost::geometry::wkt(q) << std::endl
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

        svg_mapper<point_type> mapper(svg, 500, 500);

        mapper.add(p);
        mapper.add(q);

        mapper.map(p, "opacity:0.6;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");
        mapper.map(q, "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:1");

        for (typename std::vector<OutputType>::const_iterator it = out_i.begin();
                it != out_i.end(); ++it)
        {
            mapper.map(*it, "opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:3");
        }
        for (typename std::vector<OutputType>::const_iterator it = out_u.begin();
                it != out_u.end(); ++it)
        {
            mapper.map(*it, "opacity:0.6;fill:none;stroke:rgb(255,0,255);stroke-width:3");
        }
    }
    return result;
}

#endif // BOOST_GEOMETRY_TEST_OVERLAY_P_Q_HPP
