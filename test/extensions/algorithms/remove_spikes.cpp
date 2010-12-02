// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/algorithms/remove_spikes.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>



#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif

template <typename Geometry>
inline void test_remove_spikes(std::string const& id,
            Geometry const& geometry,
            int expected_count)
{
    typedef typename bg::point_type<Geometry>::type point_type;

    Geometry processed = geometry;
    //bg::remove_spikes(processed, bg::remove_elongated_spikes<point_type>());
    bg::remove_spikes(processed, bg::remove_by_normalized<point_type>());
    bg::unique(processed);


    int detected = bg::num_points(processed);

    BOOST_CHECK_MESSAGE(detected == expected_count,
        "remove_spikes: " << bg::wkt(geometry)
        << " -> Expected: " << expected_count
        << " detected: " << detected);


#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "remove_spikes_" << id << ".svg";
        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<typename bg::point_type<Geometry>::type> mapper(svg, 500, 500);
        mapper.add(geometry);
        mapper.map(geometry, "opacity:0.6;fill:rgb(0,0,255);stroke:rgb(0,0,255);stroke-width:1");
        mapper.map(processed, "opacity:0.6;fill:rgb(0,255,0);stroke:rgb(0,255,0);stroke-width:1");
    }
#endif
}

template <typename Geometry>
void test_geometry(std::string const& id, std::string const& wkt, int expected_count)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    test_remove_spikes(id, geometry, expected_count);
}


#if ! defined(GGL_TEST_MULTI)

template <typename P>
void test_all()
{
    typedef bg::model::linear_ring<P> ring;
    typedef bg::model::polygon<P> polygon;

    test_geometry<polygon>("box",
            "POLYGON((0 0,0 4,4 4,4 0,0 0))", 5);
    test_geometry<polygon>("spike_right",
            "POLYGON((0 0,0 4,4 4,4 2,6 2,4 2,4 0,0 0))", 6);
    test_geometry<polygon>("spike_at_first",
            "POLYGON((0 0,-1 3,0 0,0 4,4 4,4 0,0 0))", 5);
    test_geometry<polygon>("spike_at_closing",
            "POLYGON((-1 0,0 0,0 4,4 4,4 0,0 0,-1 0))", 5);
    test_geometry<polygon>("double_spike",
            "POLYGON((0 0,0 4,4 4,4 2,6 2,5 2,4 2,4 0,0 0))", 6);
    test_geometry<polygon>("three_double_spike",
            "POLYGON((0 0,0 4,4 4,4 2,6 2,5 2,4.5 2,4 2,4 0,0 0))", 6);
}

int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    return 0;
}
#endif
