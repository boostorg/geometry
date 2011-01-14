// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <boost/detail/iomanip.hpp>
#include <string>


#include <geometry_test_common.hpp>

#include <boost/foreach.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>



#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif

template <typename Geometry>
inline void test_assemble(std::string const& id, Geometry const& p, Geometry const& q)
{

    std::vector<Geometry> u, i;
    bg::union_inserter<Geometry>(p, q, std::back_inserter(u));
    bg::intersection_inserter<Geometry>(p, q, std::back_inserter(i));

    typedef typename bg::coordinate_type<Geometry>::type type;
    type area_p = bg::area(p);
    type area_q = bg::area(q);
    type area_i = 0;
    type area_u = 0;

    BOOST_FOREACH(Geometry const& g, u)
    {
        area_u += bg::area(g);
    }
    BOOST_FOREACH(Geometry const& g, i)
    {
        area_i += bg::area(g);
    }

    double diff = (area_p + area_q) - area_u - area_i;
    BOOST_CHECK_CLOSE(diff, 0.0, 0.0001);
    if (abs(diff) > 0.001)
    {
        std::cout
            << id << std::endl
            << bg::wkt(p) << std::endl
            << bg::wkt(q) << std::endl;
    }

#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "assemble_" << id << ".svg";
        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<typename bg::point_type<Geometry>::type> mapper(svg, 500, 500);
        mapper.add(p);
        mapper.add(q);
        mapper.map(p, "fill-opacity:0.6;stroke-opacity:0.9;fill:rgb(0,0,255);stroke:rgb(0,0,255);stroke-width:2");
        mapper.map(q, "fill-opacity:0.6;stroke-opacity:0.9;fill:rgb(0,255,0);stroke:rgb(0,255,0);stroke-width:2");
        BOOST_FOREACH(Geometry const& geometry, u)
        {
            mapper.map(geometry,
                "stroke-opacity:0.6;fill:none;stroke:rgb(255,0,255);stroke-width:5");
        }
        BOOST_FOREACH(Geometry const& geometry, i)
        {
            mapper.map(geometry,
                "stroke-opacity:0.6;fill:none;stroke:rgb(255,0,0);stroke-width:5");
        }
    }
#endif
}

template <typename Polygon>
inline bool int_ok(Polygon const& poly)
{

    typename bg::point_type<Polygon>::type const& pi =
        bg::interior_rings(poly)[0].front();

    return bg::within(pi, bg::exterior_ring(poly));
}



void generate()
{

    static std::string exteriors[4] = {
            "(0 0,0 10,10 10,10 0,0 0)",
            "(1 1,1 9,8 9,8 1,1 1)",
            "(2 0.5, 0.5 2,0.5 8,2 9.5,6 9.5,8.5 8,8.5 2,7 0.5,2 0.5)",
            "(3 3,3 7,6 7,6 3,3 3)"
    };
    static std::string interiors[4] = {
            "(2 2,2 8,7 8,7 2,2 2)",
            "(8.5 1,8.5 2,9.5 2,9.5 1,8.5 1)",
            "(4 4,4 5,5 5,5 4,4 4)",
            "(6 4,6 5,9 5,9 4,6 4)"
    };
    for (int pe = 0; pe < 4; pe++)
    {
        for (int qe = 0; qe < 4; qe++)
        {
            for (int pi = 0; pi < 4; pi++)
            {
                for (int qi = 0; qi < 4; qi++)
                {
                    std::string ps = "POLYGON(" + exteriors[pe] + "," + interiors[pi] + ")";
                    std::string qs = "POLYGON(" + exteriors[qe] + "," + interiors[qi] + ")";

                    typedef bg::model::d2::point_xy<double> point_type;
                    bg::model::polygon<point_type> p, q;
                    bg::read_wkt(ps, p);
                    bg::read_wkt(qs, q);
                    bg::correct(p);
                    bg::correct(q);
                    if (! bg::intersects(p)
                        && ! bg::intersects(q)
                        && int_ok(p)
                        && int_ok(q)
                        )
                    {
                        std::ostringstream out;
                        out << pe << qe << pi << qi;
                        test_assemble(out.str(), p, q);

                    }
                }
            }
        }
    }
}


#if ! defined(GEOMETRY_TEST_MULTI)
int test_main(int, char* [])
{
    generate();
    return 0;
}
#endif
