// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/detail/iomanip.hpp>
#include <string>

#define BOOST_GEOMETRY_REPORT_OVERLAY_ERROR
#define BOOST_GEOMETRY_NO_BOOST_TEST


#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include <test_overlay_p_q.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/multi/multi.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/io/svg/svg_mapper.hpp>


template <typename Polygon, typename Generator>
inline void make_polygon(Polygon& polygon, Generator& generator)
{
    typedef typename bg::point_type<Polygon>::type point_type;
    typedef typename bg::coordinate_type<Polygon>::type coordinate_type;

    coordinate_type x, y;
    x = generator();
    y = generator();

    typename bg::ring_type<Polygon>::type& ring = bg::exterior_ring(polygon);

    point_type p;
    bg::set<0>(p, x); bg::set<1>(p, y); ring.push_back(p);
    bg::set<0>(p, x); bg::set<1>(p, y + 1); ring.push_back(p);
    bg::set<0>(p, x + 1); bg::set<1>(p, y + 1); ring.push_back(p);
    bg::set<0>(p, x + 1); bg::set<1>(p, y); ring.push_back(p);
    bg::set<0>(p, x); bg::set<1>(p, y); ring.push_back(p);

    if (true)
    {
        // Remove a point depending on generator
        int c = generator() % 4;
        if (c >= 1 && c <= 3)
        {
            ring.erase(ring.begin() + c);
        }
    }
}



template <typename MultiPolygon, typename Generator>
bool test_recursive_boxes(MultiPolygon& result, int& index,
            Generator& generator,
            bool svg, int level = 3)
{
    MultiPolygon p, q;

    // Generate two boxes
    if (level == 0)
    {
        p.resize(1);
        q.resize(1);
        make_polygon(p.front(), generator);
        make_polygon(q.front(), generator);
        bg::correct(p);
        bg::correct(q);
    }
    else
    {
        bg::correct(p);
        bg::correct(q);
        if (! test_recursive_boxes(p, index, generator, svg, level - 1)
            || ! test_recursive_boxes(q, index, generator, svg, level - 1))
        {
            return false;
        }
    }

    typedef typename boost::range_value<MultiPolygon>::type polygon;

    std::ostringstream out;
    out << "recursive_box_" << index++ << "_" << level;
    if (! test_overlay_p_q
        <
            polygon,
            typename bg::coordinate_type<MultiPolygon>::type
        >(out.str(), p, q, svg, 0.001))
    {
        return false;
    }

    MultiPolygon mp;
    bg::union_inserter
        <
            polygon
        >(p, q, std::back_inserter(mp));

    bg::unique(mp);
    bg::simplify(mp, result, 0.01);
    bg::correct(mp);
    return true;
}


template <typename T, bool Clockwise, bool Closed>
void test_all(int seed, int count, int fieldsize, bool svg, int level)
{
    boost::timer t;

    typedef boost::minstd_rand base_generator_type;

    base_generator_type generator(seed);

    boost::uniform_int<> random_coordinate(0, fieldsize);
    boost::variate_generator<base_generator_type&, boost::uniform_int<> >
        coordinate_generator(generator, random_coordinate);

    typedef bg::model::polygon
        <
            bg::model::d2::point_xy<T>, Clockwise, Closed
        > polygon;
    typedef bg::model::multi_polygon<polygon> mp;


    int index = 0;
    for(int i = 0; i < count; i++)
    {
        mp p;
        test_recursive_boxes<mp>(p, index, coordinate_generator, svg, level);
    }
    std::cout
        << "boxes " << index
        << " type: " << string_from_type<T>::name()
        << " time: " << t.elapsed()  << std::endl;
}

int main(int argc, char** argv)
{
    try
    {
        // Arguments:
        // {count} {seed} {"svg"} {level}
        int count = argc > 1 ? boost::lexical_cast<int>(argv[1]) : 10;
        int seed = (argc > 2 && std::string(argv[2]) != std::string("#"))
            ? boost::lexical_cast<int>(argv[2])
            : static_cast<unsigned int>(std::time(0));
        bool svg = argc > 3 && std::string(argv[3]) == std::string("svg");
        int level = argc > 4 && std::string(argv[4]) != std::string("#")
            ? boost::lexical_cast<int>(argv[4]): 3;

        int fieldsize = argc > 5 && std::string(argv[5]) != std::string("#")
            ? boost::lexical_cast<int>(argv[5]): 9;

        test_all<double, true, true>(seed, count, fieldsize, svg, level);

#if defined(HAVE_TTMATH)
        // test_all<ttmath_big, true, true>(seed, count, max, svg, level);
#endif
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
