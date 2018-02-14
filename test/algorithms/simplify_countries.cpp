// Boost.Geometry (aka GGL, Generic Geometry Library)
// (Unit) Test

// Copyright (c) 2018 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry.hpp>
#include <geometry_test_common.hpp>

#include <boost/foreach.hpp>

#include <iomanip>

#if defined(TEST_WITH_SVG)
#  include <boost/geometry/io/svg/svg_mapper.hpp>
#endif



template <typename MultiPolygon>
std::string read_from_file(std::string const& filename)
{
    MultiPolygon mp;
    std::ifstream in(filename.c_str());
    while (in.good())
    {
        std::string line;
        std::getline(in, line);
        if (! line.empty())
        {
            typename boost::range_value<MultiPolygon>::type pol;
            bg::read_wkt(line, pol);
            mp.push_back(pol);
        }
    }
    std::ostringstream out;
    if (! mp.empty())
    {
        out << std::fixed << std::setprecision(19) << bg::wkt(mp);
    }

    BOOST_CHECK(! out.str().empty());

    return out.str();
}


template <typename MultiPolygon>
void test_one(std::string const& caseid, std::string const& wkt,
              double distance_in_meters,
              double expected_area_ratio, double expected_perimeter_ratio,
              std::size_t expected_polygon_count = 0,
              std::size_t expected_interior_count = 0,
              std::size_t expected_point_count = 0)
{
    boost::ignore_unused(caseid);

    MultiPolygon geometry, simplified;
    bg::read_wkt(wkt, geometry);
    bg::correct(geometry);
    bg::simplify(geometry, simplified, distance_in_meters);

    double const area_ratio = bg::area(simplified) / bg::area(geometry);
    double const perimeter_ratio = bg::perimeter(simplified) / bg::perimeter(geometry);

    BOOST_CHECK_CLOSE(perimeter_ratio, expected_perimeter_ratio, 0.01);
    BOOST_CHECK_CLOSE(area_ratio, expected_area_ratio, 0.01);
    BOOST_CHECK_EQUAL(expected_polygon_count, boost::size(simplified));
    BOOST_CHECK_EQUAL(expected_interior_count, bg::num_interior_rings(simplified));
    BOOST_CHECK_EQUAL(expected_point_count, bg::num_points(simplified));

//  To add new tests, this is convenient and write the test itself:
//    std::cout << "test_one<mp>(\"" << caseid << "\", " << caseid
//              << ", " << distance_in_meters
//              << std::setprecision(6)
//              << ", " << area_ratio
//              << ", " << perimeter_ratio
//              << ", " << boost::size(simplified)
//              << ", " << bg::num_interior_rings(simplified)
//              << ", " << bg::num_points(simplified)
//              << ");"
//              << std::endl;

#if defined(TEST_WITH_SVG)
    {
        typedef typename boost::range_value<MultiPolygon>::type polygon;

        std::ostringstream filename;
        filename << "simplify_" << caseid << "_" << distance_in_meters << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper
            <
                typename bg::point_type<MultiPolygon>::type
            > mapper(svg, 1200, 800);
        mapper.add(geometry);
        mapper.add(simplified);

        mapper.map(geometry, "fill-opacity:0.5;fill:rgb(153,204,0);"
                "stroke:rgb(153,204,0);stroke-width:1");
        BOOST_FOREACH(polygon const& pol, simplified)
        {
            mapper.map(pol,
                       bg::area(pol) > 0 ? "fill:none;stroke:rgb(255,0,0);stroke-width:1"
                       : "fill:none;stroke:rgb(255,0,255);stroke-width:1");
        }
    }
#endif
}


template <bool Clockwise, typename P>
void test_all()
{
    typedef bg::model::polygon<P, Clockwise> polygon;
    typedef bg::model::multi_polygon<polygon> mp;

    // The unit test uses countries originally added for buffer unit test
    std::string base_folder = "buffer/data/";

    // Verify for Greece, Italy, Netherlands, Norway and UK
    std::string gr = read_from_file<mp>(base_folder + "gr.wkt");
    std::string it = read_from_file<mp>(base_folder + "it.wkt");
    std::string nl = read_from_file<mp>(base_folder + "nl.wkt");
    std::string no = read_from_file<mp>(base_folder + "no.wkt");
    std::string uk = read_from_file<mp>(base_folder + "uk.wkt");

    // Gradually simplify more aggresively.
    // Area ratio (first) can increase or decrease
    // Perimeter ratio (second) should decrease.
    // Polygons, interior rings, points should decrease
    test_one<mp>("gr", gr, 100, 0.999893, 0.999761, 68, 0, 2523);
    test_one<mp>("gr", gr, 200, 0.999731, 0.998894, 68, 0, 2025);
    test_one<mp>("gr", gr, 500, 0.998969, 0.995778, 68, 0, 1472);
    test_one<mp>("gr", gr, 1000, 0.99721, 0.990555, 68, 0, 1130);
    test_one<mp>("gr", gr, 2000, 0.993177, 0.975566, 65, 0, 839);
    test_one<mp>("gr", gr, 5000, 0.976362, 0.903488, 51, 0, 479);
    test_one<mp>("gr", gr, 10000, 0.954145, 0.770951, 30, 0, 246);
    test_one<mp>("gr", gr, 20000, 0.94543, 0.611661, 11, 0, 101); // Many islands disappear

    test_one<mp>("it", it, 100, 1.00001, 0.999815, 22, 1, 1783);
    test_one<mp>("it", it, 200, 1.00009, 0.999099, 22, 1, 1402);
    test_one<mp>("it", it, 500, 1.00021, 0.996854, 22, 1, 1006);
    test_one<mp>("it", it, 1000, 1.00049, 0.992692, 22, 1, 750);
    test_one<mp>("it", it, 2000, 1.0009, 0.983258, 22, 1, 543);
    test_one<mp>("it", it, 5000, 1.00185, 0.932119, 9, 1, 280);
    test_one<mp>("it", it, 10000, 1.01091, 0.869922, 4, 0, 139); // San Marino disappears
    test_one<mp>("it", it, 20000, 1.00959, 0.8281, 4, 0, 85);

    test_one<mp>("nl", nl, 100, 0.999894, 0.999817, 8, 0, 798);
    test_one<mp>("nl", nl, 200, 0.999641, 0.999096, 8, 0, 633);
    test_one<mp>("nl", nl, 500, 0.999115, 0.996286, 8, 0, 432);
    test_one<mp>("nl", nl, 1000, 0.997689, 0.991921, 8, 0, 328);
    test_one<mp>("nl", nl, 2000, 0.995369, 0.982222, 8, 0, 237);
    test_one<mp>("nl", nl, 5000, 0.991539, 0.9332, 7, 0, 138);
    test_one<mp>("nl", nl, 10000, 0.974408, 0.806818, 4, 0, 71);
    test_one<mp>("nl", nl, 20000, 0.979702, 0.752186, 3, 0, 45);

    test_one<mp>("no", no, 100, 0.999967, 0.999976, 95, 0, 7655);
    test_one<mp>("no", no, 200, 0.999821, 0.999732, 95, 0, 6529);
    test_one<mp>("no", no, 500, 0.999319, 0.998098, 95, 0, 4740);
    test_one<mp>("no", no, 1000, 0.998455, 0.99401, 95, 0, 3525);
    test_one<mp>("no", no, 2000, 0.9963, 0.984416, 91, 0, 2568);
    test_one<mp>("no", no, 5000, 0.988401, 0.957444, 88, 0, 1656);
    test_one<mp>("no", no, 10000, 0.974084, 0.90103, 72, 0, 1070);
    test_one<mp>("no", no, 20000, 0.953065, 0.784675, 41, 0, 603);

    test_one<mp>("uk", uk, 100, 0.999949, 0.99988, 48, 0, 3219);
    test_one<mp>("uk", uk, 200, 0.999854, 0.999289, 48, 0, 2615);
    test_one<mp>("uk", uk, 500, 0.999557, 0.996874, 48, 0, 1880);
    test_one<mp>("uk", uk, 1000, 0.998942, 0.992074, 48, 0, 1384);
    test_one<mp>("uk", uk, 2000, 0.997703, 0.983059, 48, 0, 1028);
    test_one<mp>("uk", uk, 5000, 0.990991, 0.942994, 37, 0, 611);
    test_one<mp>("uk", uk, 10000, 0.988484, 0.871371, 24, 0, 365);
    test_one<mp>("uk", uk, 20000, 0.972889, 0.736474, 12, 0, 191);
}

int test_main(int, char* [])
{
    test_all<true, bg::model::point<double, 2, bg::cs::cartesian> >();
    return 0;
}

