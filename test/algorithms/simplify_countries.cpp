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
              double expected_area_ratio, double expected_perimeter_ratio)
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

//  To add new tests, this is convenient and write the test itself:
//    std::cout << "test_one<mp>(\"" << caseid << "\", " << caseid
//              << ", " << distance_in_meters
//              << std::setprecision(6)
//              << ", " << area_ratio
//              << ", " << perimeter_ratio
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
    test_one<mp>("gr", gr, 100, 0.999893, 0.999761);
    test_one<mp>("gr", gr, 200, 0.999731, 0.998894);
    test_one<mp>("gr", gr, 500, 0.998969, 0.995778);
    test_one<mp>("gr", gr, 1000, 0.99721, 0.990555);
    test_one<mp>("gr", gr, 2000, 0.993177, 0.975566);
    test_one<mp>("gr", gr, 5000, 0.976362, 0.903488);
    test_one<mp>("gr", gr, 10000, 0.954145, 0.770951);
    test_one<mp>("gr", gr, 20000, 0.94543, 0.611661); // many islands disappeared

    test_one<mp>("it", it, 100, 1.00001, 0.999815);
    test_one<mp>("it", it, 200, 1.00009, 0.999099);
    test_one<mp>("it", it, 500, 1.00021, 0.996854);
    test_one<mp>("it", it, 1000, 1.00049, 0.992692);
    test_one<mp>("it", it, 2000, 1.0009, 0.983258);
    test_one<mp>("it", it, 5000, 1.00185, 0.932119);
    test_one<mp>("it", it, 10000, 1.01091, 0.869922);
    test_one<mp>("it", it, 20000, 1.00959, 0.8281);

    test_one<mp>("nl", nl, 100, 0.999894, 0.999817);
    test_one<mp>("nl", nl, 200, 0.999641, 0.999096);
    test_one<mp>("nl", nl, 500, 0.999115, 0.996286);
    test_one<mp>("nl", nl, 1000, 0.997689, 0.991921);
    test_one<mp>("nl", nl, 2000, 0.995369, 0.982222);
    test_one<mp>("nl", nl, 5000, 0.991539, 0.9332);
    test_one<mp>("nl", nl, 10000, 0.974408, 0.806818);
    test_one<mp>("nl", nl, 20000, 0.979702, 0.752186);

    test_one<mp>("no", no, 100, 0.999967, 0.999976);
    test_one<mp>("no", no, 200, 0.999821, 0.999732);
    test_one<mp>("no", no, 500, 0.999319, 0.998098);
    test_one<mp>("no", no, 1000, 0.998455, 0.99401);
    test_one<mp>("no", no, 2000, 0.9963, 0.984416);
    test_one<mp>("no", no, 5000, 0.988401, 0.957444);
    test_one<mp>("no", no, 10000, 0.974084, 0.90103);
    test_one<mp>("no", no, 20000, 0.953065, 0.784675);

    test_one<mp>("uk", uk, 100, 0.999949, 0.99988);
    test_one<mp>("uk", uk, 200, 0.999854, 0.999289);
    test_one<mp>("uk", uk, 500, 0.999557, 0.996874);
    test_one<mp>("uk", uk, 1000, 0.998942, 0.992074);
    test_one<mp>("uk", uk, 2000, 0.997703, 0.983059);
    test_one<mp>("uk", uk, 5000, 0.990991, 0.942994);
    test_one<mp>("uk", uk, 10000, 0.988484, 0.871371);
    test_one<mp>("uk", uk, 20000, 0.972889, 0.736474);
}

int test_main(int, char* [])
{
    test_all<true, bg::model::point<double, 2, bg::cs::cartesian> >();
    return 0;
}

