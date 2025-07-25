// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2012-2021 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015-2021.
// Modifications copyright (c) 2015-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_BOOST_TEST

#ifndef BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE
#define BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE
#endif

#if defined(_MSC_VER)
#  pragma warning( disable : 4244 )
#  pragma warning( disable : 4267 )
#endif

#include <chrono>
#include <fstream>
#include <sstream>

#include <boost/program_options.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/geometry/algorithms/detail/buffer/buffer_inserter.hpp>

#include <boost/geometry/strategies/buffer.hpp>

#include <geometry_test_common.hpp>
#include <geometry_to_crc.hpp>
#include <robustness/common/common_settings.hpp>
#include <robustness/common/make_random_generator.hpp>
#include <robustness/common/make_square_polygon.hpp>


struct buffer_settings : public common_settings
{
    int join_code{1};
    double distance{1.0};
    int points_per_circle{32}; // MySQL also uses 32 points in a circle for buffer
};

namespace bg = boost::geometry;

template <typename Geometry1, typename Geometry2>
void create_svg(std::string const& filename
                , Geometry1 const& mp
                , Geometry2 const& buffer)
{
    typedef typename boost::geometry::point_type<Geometry1>::type point_type;

    std::ofstream svg(filename.c_str());
    boost::geometry::svg_mapper<point_type> mapper(svg, 800, 800);

    boost::geometry::model::box<point_type> box;
    bg::envelope(mp, box);
    bg::buffer(box, box, 1.0);
    mapper.add(box);

    if (! bg::is_empty(buffer))
    {
        bg::envelope(buffer, box);
        bg::buffer(box, box, 1.0);
        mapper.add(box);
    }

    mapper.map(mp, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:3");
    mapper.map(buffer, "stroke-opacity:0.9;stroke:rgb(0,0,0);fill:none;stroke-width:1");
}

template <typename Geometry, typename Buffer>
bool verify_buffer(Geometry const& geometry, Buffer const& buffer, std::string& reason)
{
    if (buffer.empty())
    {
        reason = "Buffer is empty";
        return false;
    }

    // Area of buffer must be larger than of original polygon
    auto const area_mp = bg::area(geometry);
    auto const area_buf = bg::area(buffer);
    if (area_buf < area_mp)
    {
        reason = "Buffer area is smaller than input area";
        return false;    
    }

    // Verify if all points are IN the buffer
    bool all_within = true;
    bg::for_each_point(geometry, [&all_within, &buffer](auto const& point)
    {
            if (! bg::within(point, buffer))
            {
                all_within = false;
            }
        }
    );

    if (! all_within)
    {
        reason = "Not all points are within buffer";
        return false;
    }

    return bg::is_valid(buffer, reason);
}

template <typename MultiPolygon, typename Generator, typename Settings>
bool test_buffer(MultiPolygon& result, int& index,
            Generator& generator,
            int level, Settings const& settings)
{
    MultiPolygon p, q;

    // Generate two boxes
    if (level == 0)
    {
        p.resize(1);
        q.resize(1);
        make_square_polygon(p.front(), generator, settings);
        make_square_polygon(q.front(), generator, settings);
        bg::correct(p);
        bg::correct(q);
    }
    else
    {
        // Recursive call
        bg::correct(p);
        bg::correct(q);
        if (! test_buffer(p, index, generator, level - 1, settings)
            || ! test_buffer(q, index, generator, level - 1, settings))
        {
            return false;
        }
    }

    typedef typename boost::range_value<MultiPolygon>::type polygon;

    MultiPolygon mp;
    bg::detail::union_::union_insert
        <
            polygon
        >(p, q, std::back_inserter(mp));

    bg::unique(mp);
    bg::unique(mp);
    bg::correct(mp);
    result = mp;


    typedef typename bg::coordinate_type<MultiPolygon>::type coordinate_type;
    typedef bg::strategy::buffer::distance_asymmetric<coordinate_type> distance_strategy_type;
    distance_strategy_type distance_strategy(settings.distance, settings.distance);

    MultiPolygon buffered;

    std::ostringstream out;
    out << "recursive_polygons_buffer_" << index++ << "_" << level;

    bg::strategy::buffer::end_round end_strategy;
    bg::strategy::buffer::point_circle point_strategy;
    bg::strategy::buffer::side_straight side_strategy;
    bg::strategy::buffer::join_round join_round_strategy(settings.points_per_circle);
    bg::strategy::buffer::join_miter join_miter_strategy;
    
    try
    {
        switch(settings.join_code)
        {
            case 1 :
                bg::buffer(mp, buffered,
                                distance_strategy, side_strategy,
                                join_round_strategy,
                                end_strategy, point_strategy);
                break;
            case 2 :
                bg::buffer(mp, buffered,
                                distance_strategy, side_strategy,
                                join_miter_strategy,
                                end_strategy, point_strategy);
                break;
            default :
                return false;
        }
    }
    catch(std::exception const& e)
    {
        MultiPolygon empty;
        std::cout << out.str() << std::endl;
        std::cout << "Exception " << e.what() << std::endl;
        return false;
    }

    if (settings.verbose)
    {
        std::cout << " [" << bg::area(mp) << " " << bg::area(buffered) << "]";
    }

    std::string verification_message;
    if (verify_buffer(mp, buffered, verification_message))
    {
        if (settings.svg)
        {
            create_svg(out.str() + ".svg", mp, buffered);
        }
        return true;
    }

    std::string filename;

    {
        // Generate a unique name
        std::ostringstream out;
        out << "rec_pol_buffer_" << geometry_to_crc(mp)
            << "_" << string_from_type<typename bg::coordinate_type<MultiPolygon>::type>::name()
            << ".";
        filename = out.str();
    }

    std::cout << "Failure" << " " << filename << " : " << verification_message << std::endl;
    std::ofstream stream(filename + "wkt");
    // Stream input WKT
    stream << bg::wkt(mp) << std::endl;
    // If you need the output WKT, then stream bg::wkt(buffer)

    return false;
}


template <typename T, bool Clockwise, bool Closed, typename Settings>
void test_all(int seed, int count, int level, Settings const& settings)
{
    auto const t0 = std::chrono::high_resolution_clock::now();

    auto coordinate_generator = make_int_generator(seed, settings.field_size - 1);

    typedef bg::model::polygon
        <
            bg::model::d2::point_xy<T>, Clockwise, Closed
        > polygon;
    typedef bg::model::multi_polygon<polygon> mp;

    int index = 0;
    int errors = 0;
    for(int i = 0; i < count; i++)
    {
        mp p;
        if (! test_buffer<mp>(p, index, coordinate_generator, level, settings))
        {
            errors++;
        }
    }

    auto const t = std::chrono::high_resolution_clock::now();
    auto const elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count();
    std::cout << std::endl
        << "geometries: " << index
        << " errors: " << errors
        << " type: " << string_from_type<T>::name()
        << " time: " << elapsed_ms / 1000.0 << std::endl;
}

int main(int argc, char** argv)
{
    BoostGeometryWriteTestConfiguration();
    try
    {
        namespace po = boost::program_options;
        po::options_description description("=== recursive_polygons_buffer ===\nAllowed options");

        int count = 1;
        int seed = -1;
        int level = 3;
        bool ccw = false;
        bool open = false;
        buffer_settings settings;
        std::string form = "box";
        std::string join = "round";

        description.add_options()
            ("help", "Help message")
            ("seed", po::value<int>(&seed), "Initialization seed for random generator")
            ("count", po::value<int>(&count), "Number of tests")
            ("validity", po::value<bool>(&settings.check_validity)->default_value(true), "Include testing on validity")
            ("level", po::value<int>(&level), "Level to reach (higher -> slower)")
            ("distance", po::value<double>(&settings.distance)->default_value(1.0), "Distance (1.0)")
            ("ppc", po::value<int>(&settings.points_per_circle)->default_value(32), "Points per circle (32)")
            ("form", po::value<std::string>(&form)->default_value("box"), "Form of the polygons (box, triangle)")
            ("join", po::value<std::string>(&join)->default_value("round"), "Form of the joins (round, miter)")
            ("ccw", po::value<bool>(&ccw), "Counter clockwise polygons")
            ("open", po::value<bool>(&open), "Open polygons")
            ("size", po::value<int>(&settings.field_size)->default_value(10), "Size of the field")
            ("verbose", po::value<bool>(&settings.verbose), "Verbose")
            ("wkt", po::value<bool>(&settings.wkt), "Create a WKT of the inputs, for all tests")
            ("svg", po::value<bool>(&settings.svg), "Create a SVG for all tests")
        ;

        po::variables_map varmap;
        po::store(po::parse_command_line(argc, argv, description), varmap);
        po::notify(varmap);

        if (varmap.count("help")
            || (form != "box" && form != "triangle")
            || (join != "round" && join != "miter")
            )
        {
            std::cout << description << std::endl;
            return 1;
        }

        settings.triangular = form != "box";
        settings.join_code = join == "round" ? 1 : 2;

        if (ccw && open)
        {
            test_all<default_test_type, false, false>(seed, count, level, settings);
        }
        else if (ccw)
        {
            test_all<default_test_type, false, true>(seed, count, level, settings);
        }
        else if (open)
        {
            test_all<default_test_type, true, false>(seed, count, level, settings);
        }
        else
        {
            test_all<default_test_type, true, true>(seed, count, level, settings);
        }
    }
    catch(std::exception const& e)
    {
        std::cout << "Exception " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "Other exception" << std::endl;
    }

    return 0;
}
