// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/detail/sections/sectionalize.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>

#include <test_common/test_point.hpp>

template <int DimensionCount, typename G>
void test_sectionalize(G const& g, std::size_t section_count,
        std::string const& index_check, std::string const& dir_check,
        bool sort = false)
{
    typedef typename boost::geometry::point_type<G>::type point;
    typedef boost::geometry::box<point> box;
    typedef boost::geometry::sections<box, DimensionCount> sections;

    sections s;
    boost::geometry::sectionalize(g, s);

    BOOST_CHECK_EQUAL(s.size(), section_count);

    // Check if sections are consecutive and consistent
    int previous_index = -1;
    BOOST_FOREACH(typename sections::value_type const& sec, s)
    {
        if (sec.begin_index > 0)
        {
            BOOST_CHECK_EQUAL(previous_index, sec.begin_index);
        }
        BOOST_CHECK_EQUAL(sec.count, sec.end_index - sec.begin_index);
        previous_index = sec.end_index;
    }

    // Output streams for sections, boxes, other
    std::ostringstream out_sections;
    std::ostringstream out_boxes;
    std::ostringstream out_dirs;


    for (typename sections::size_type i = 0; i < s.size(); i++)
    {
        box const& b = s[i].bounding_box;

        if (i > 0)
        {
            out_sections << "|";
            out_dirs << "|";
            out_boxes << "|";
        }

        out_sections << s[i].begin_index << ".." << s[i].end_index;
        out_boxes << boost::geometry::get<0,0>(b) << " " << boost::geometry::get<0,1>(b)
            << ".." << boost::geometry::get<1,0>(b) << " " << boost::geometry::get<1,1>(b);
        for (int d = 0; d < DimensionCount; d++)
        {
            out_dirs << (d == 0 ? "" : " ");
            switch(s[i].directions[d])
            {
                case -99: out_dirs << "DUP"; break;
                case -1 : out_dirs << "-"; break;
                case  0 : out_dirs << "."; break;
                case +1 : out_dirs << "+"; break;
            }
        }
    }

    if (! index_check.empty())
    {
        BOOST_CHECK_EQUAL(out_sections.str(), index_check);
    }
    if (! dir_check.empty())
    {
        BOOST_CHECK_EQUAL(out_dirs.str(), dir_check);
    }
    else
    {
        if (out_sections.str().length() < 80)
        {
            std::cout << std::endl << boost::geometry::wkt(g) << std::endl;
            std::cout << out_sections.str() << std::endl;
            //std::cout << out_boxes.str() << std::endl;
        }
        std::cout << out_dirs.str() << std::endl;
    }
}

template <typename G>
void test_sectionalize(std::string const& wkt,
        std::size_t count2, std::string const& s2, std::string const d2,
        std::size_t count1, std::string const& s1, std::string const d1,
        bool sort = false)
{
    G g;
    boost::geometry::read_wkt(wkt, g);
    test_sectionalize<2>(g, count2, s2, d2, sort);
    test_sectionalize<1>(g, count1, s1, d1, sort);
}

template <typename P>
void test_all()
{
    test_sectionalize<boost::geometry::linestring<P> >(
        "LINESTRING(1 1,2 2,3 0,5 0,5 8)",
        4, "0..1|1..2|2..3|3..4", "+ +|+ -|+ .|. +",
        2, "0..3|3..4", "+|.");

    test_sectionalize<boost::geometry::polygon<P> >(
        "POLYGON((0 0,0 7,4 2,2 0,0 0))",
        4, "0..1|1..2|2..3|3..4", ". +|+ -|- -|- .",
        //            .   +   -   -   -> 3 sections
        3, "0..1|1..2|2..4", ".|+|-");

    test_sectionalize<boost::geometry::polygon<P> >
        ("polygon((2.0 1.3, 2.4 1.7, 2.8 1.8, 3.4 1.2, 3.7 1.6,3.4 2.0, 4.1 3.0, 5.3 2.6, 5.4 1.2, 4.9 0.8, 2.9 0.7,2.0 1.3))",
        8, "0..2|2..3|3..4|4..5|5..6|6..8|8..10|10..11", "+ +|+ -|+ +|- +|+ +|+ -|- -|- +",
        4, "0..4|4..5|5..8|8..11", "+|-|+|-");


    test_sectionalize<boost::geometry::polygon<P> >(
        "POLYGON((3 1,2 2,1 3,2 4,3 5,4 4,5 3,4 2,3 1))",
        4, "0..2|2..4|4..6|6..8", "- +|+ +|+ -|- -",
        //        -   -   -   +   +   +   +   -   - -> 3 sections
        3, "0..2|2..6|6..8", "-|+|-");

    // With holes
    test_sectionalize<boost::geometry::polygon<P> >(
        "POLYGON((3 1,2 2,1 3,2 4,3 5,4 4,5 3,4 2,3 1), (3 2,2 2,3 4,3 2))",
        7, "0..2|2..4|4..6|6..8|0..1|1..2|2..3", "- +|+ +|+ -|- -|- .|+ +|. -",
        //        -   -   -   +   +   +   +   -   -          -   +   . -> 6 sections
        6, "0..2|2..6|6..8|0..1|1..2|2..3", "-|+|-|-|+|.");

    // With duplicates
    test_sectionalize<boost::geometry::linestring<P> >(
        "LINESTRING(1 1,2 2,3 0,3 0,5 0,5 8)",
        5, "0..1|1..2|2..3|3..4|4..5", "+ +|+ -|DUP DUP|+ .|. +",
        4, "0..2|2..3|3..4|4..5", "+|DUP|+|.");
    // With two subsequent duplicate segments
    test_sectionalize<boost::geometry::linestring<P> >(
        "LINESTRING(1 1,2 2,3 0,3 0,3 0,5 0,5 0,5 0,5 0,5 8)",
        6, "0..1|1..2|2..4|4..5|5..8|8..9", "+ +|+ -|DUP DUP|+ .|DUP DUP|. +",
        5, "0..2|2..4|4..5|5..8|8..9", "+|DUP|+|DUP|.");


    typedef boost::geometry::box<P> B;
    test_sectionalize<2, B>(boost::geometry::make<B>(0,0,4,4),
            4, "0..1|1..2|2..3|3..4", ". +|+ .|. -|- .");
    test_sectionalize<1, B>(boost::geometry::make<B>(0,0,4,4),
            4, "0..1|1..2|2..3|3..4", ".|+|.|-");

    return;
    // Buffer-case
    test_sectionalize<boost::geometry::polygon<P> >(
    "POLYGON((-1.1713 0.937043,2.8287 5.93704,2.90334 6.02339,2.98433 6.10382,2.98433 6.10382,3.07121 6.17786,3.16346 6.24507,3.16346 6.24507,3.16346 6.24507,3.26056 6.30508,3.36193 6.35752,3.36193 6.35752,3.46701 6.40211,3.57517 6.43858,3.57517 6.43858,3.57517 6.43858,3.57517 6.43858,3.68579 6.46672,3.79822 6.48637,3.79822 6.48637,3.91183 6.49741,4.02595 6.49978,4.02595 6.49978,4.02595 6.49978,4.13991 6.49346,4.25307 6.4785,4.25307 6.4785,4.36476 6.45497,4.47434 6.42302,4.47434 6.42302,4.47434 6.42302,4.47434 6.42302,7.47434 5.42302,6.84189 3.52566,4.39043 4.68765,0.390434 -0.312348,-1.1713 0.937043))",
        8, "0..2|2..3|3..4|4..5|5..6|6..8|8..10|10..11", "+ +|+ -|+ +|- +|+ +|+ -|- -|- +",
        4, "0..4|4..5|5..8|8..11", "+|-|+|-");
}

int test_main(int, char* [])
{
    //test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
