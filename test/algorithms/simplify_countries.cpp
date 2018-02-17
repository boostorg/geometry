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
    test_one<mp>("gr", gr, 100, 0.999891, 0.999756, 68, 0, 2519);
    test_one<mp>("gr", gr, 200, 0.999733, 0.998884, 68, 0, 2021);
    test_one<mp>("gr", gr, 500, 0.999055, 0.995767, 68, 0, 1470);
    test_one<mp>("gr", gr, 1000, 0.997767, 0.991097, 68, 0, 1139);
    test_one<mp>("gr", gr, 2000, 0.994679, 0.976902, 65, 0, 852);
    test_one<mp>("gr", gr, 5000, 0.980616, 0.89164, 45, 0, 450);
    test_one<mp>("gr", gr, 10000, 0.96915, 0.767563, 25, 0, 229);
    test_one<mp>("gr", gr, 20000, 0.937931, 0.579678, 8, 0, 90);  // Many islands disappear

    test_one<mp>("it", it, 100, 1, 0.999812, 22, 1, 1781);
    test_one<mp>("it", it, 200, 1.00009, 0.999088, 22, 1, 1398);
    test_one<mp>("it", it, 500, 1.0002, 0.996893, 22, 1, 1005);
    test_one<mp>("it", it, 1000, 1.00053, 0.993039, 22, 1, 747);
    test_one<mp>("it", it, 2000, 1.00087, 0.984379, 22, 1, 547);
    test_one<mp>("it", it, 5000, 1.00174, 0.928495, 8, 1, 272);
    test_one<mp>("it", it, 10000, 1.01316, 0.880268, 4, 1, 151);
    test_one<mp>("it", it, 20000, 1.00764, 0.824566, 4, 0, 87); // San Marino disappears

    test_one<mp>("nl", nl, 100, 0.999889, 0.999814, 8, 0, 796);
    test_one<mp>("nl", nl, 200, 0.999627, 0.999074, 8, 0, 631);
    test_one<mp>("nl", nl, 500, 0.999208, 0.996389, 8, 0, 434);
    test_one<mp>("nl", nl, 1000, 0.998292, 0.991971, 8, 0, 329);
    test_one<mp>("nl", nl, 2000, 0.995699, 0.981222, 8, 0, 235);
    test_one<mp>("nl", nl, 5000, 0.993008, 0.915048, 6, 0, 135);
    test_one<mp>("nl", nl, 10000, 0.978084, 0.80839, 3, 0, 69);
    test_one<mp>("nl", nl, 20000, 0.976826, 0.73938, 3, 0, 41);

    test_one<mp>("no", no, 100, 0.999966, 0.999976, 95, 0, 7653);
    test_one<mp>("no", no, 200, 0.999819, 0.999732, 95, 0, 6527);
    test_one<mp>("no", no, 500, 0.999303, 0.99808, 95, 0, 4725);
    test_one<mp>("no", no, 1000, 0.998466, 0.993979, 95, 0, 3517);
    test_one<mp>("no", no, 2000, 0.996415, 0.985417, 92, 0, 2572);
    test_one<mp>("no", no, 5000, 0.990913, 0.962711, 86, 0, 1671);
    test_one<mp>("no", no, 10000, 0.975975, 0.900398, 66, 0, 1045);
    test_one<mp>("no", no, 20000, 0.954832, 0.767641, 33, 0, 575);

    test_one<mp>("uk", uk, 100, 0.999948, 0.999879, 48, 0, 3214);
    test_one<mp>("uk", uk, 200, 0.99985, 0.999288, 48, 0, 2614);
    test_one<mp>("uk", uk, 500, 0.999535, 0.996913, 48, 0, 1882);
    test_one<mp>("uk", uk, 1000, 0.998906, 0.991917, 48, 0, 1383);
    test_one<mp>("uk", uk, 2000, 0.998059, 0.984074, 48, 0, 1042);
    test_one<mp>("uk", uk, 5000, 0.991533, 0.941306, 36, 0, 608);
    test_one<mp>("uk", uk, 10000, 0.990376, 0.87048, 23, 0, 361);
    test_one<mp>("uk", uk, 20000, 0.977764, 0.719399, 10, 0, 182);

}

int test_main(int, char* [])
{
    test_all<true, bg::model::point<double, 2, bg::cs::cartesian> >();
    return 0;
}

