// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#define BOOST_GEOMETRY_NO_BOOST_TEST

// For mixing int/float
#if defined(_MSC_VER)
#pragma warning( disable : 4267 )
#endif



#include <algorithms/test_intersection.hpp>
#include <algorithms/test_overlay.hpp>

#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>

#include <test_common/test_point.hpp>
#include <test_common/with_pointer.hpp>



template <typename Polygon>
inline void make_star(Polygon& polygon,
    int count, double factor1, double factor2, long double offset = 0)
{
    typedef typename bg::point_type<Polygon>::type p;
    typedef typename bg::select_most_precise
        <
            typename bg::coordinate_type<Polygon>::type,
            long double
        >::type coordinate_type;

    // Create star
    coordinate_type cx = 25.0;
    coordinate_type cy = 25.0;

    coordinate_type dx = 50.0;
    coordinate_type dy = 50.0;

    coordinate_type half = 0.5;
    coordinate_type two = 2.0;

    coordinate_type a1 = coordinate_type(factor1) * half * dx;
    coordinate_type b1 = coordinate_type(factor1) * half * dy;
    coordinate_type a2 = coordinate_type(factor2) * half * dx;
    coordinate_type b2 = coordinate_type(factor2) * half * dy;

    coordinate_type pi = boost::math::constants::pi<long double>();
    coordinate_type delta = pi * two / coordinate_type(count - 1);
    coordinate_type angle = coordinate_type(offset) * delta;
    std::cout << "Start with angle: " << angle << std::endl;
    for (int i = 0; i < count - 1; i++, angle += delta)
    {
        bool even = i % 2 == 0;
        coordinate_type s = sin(angle);
        coordinate_type c = cos(angle);
        coordinate_type x = cx + (even ? a1 : a2) * s;
        coordinate_type y = cy + (even ? b1 : b2) * c;
        bg::exterior_ring(polygon).push_back(bg::make<p>(x, y));

    }
    bg::exterior_ring(polygon).push_back(bg::exterior_ring(polygon).front());
}


template <typename T, typename CalculationType>
void test_star(std::string const& p_type, std::string const& c_type,
    std::string const& p_selection, std::string const& c_selection,
    int min_points, int max_points, T rotation)
{
    if ((! p_selection.empty() && p_selection != p_type)
        || (! c_selection.empty() && c_selection != c_type))
    {
        return;
    }

    std::cout << "TESTING case "
        << p_type << " (" << sizeof(T) << ") "
        << c_type << " (" << sizeof(CalculationType) << ")"
        << std::endl;
    boost::timer t;
    typedef bg::model::d2::point_xy<T> point_type;
    typedef bg::model::polygon<point_type> polygon;

    T area_i = 0;
    T area_a = 0;
    T area_b = 0;
    //T area_u = 0;

    for (int p = min_points; p <= max_points; p++)
    {
        std::ostringstream out;
        out << "_" << p_type << "_"  << c_type << "_" << p << "_int";

        polygon a;
        make_star(a, p * 2 + 1, 0.5, 1.0);
        polygon b;
        make_star(b, p * 2 + 1, 0.5, 1.0, rotation);

        area_i += test_intersection<polygon, CalculationType>(out.str(), a, b);
        area_a += bg::area(a);
        area_b += bg::area(b);
    }

    std::cout
        << " time: " << t.elapsed()  << std::endl
        << " area: " << area_i << std::endl
        << " area a: " << area_a << std::endl
        << " area b: " << area_b  << std::endl
        << " diff: " << (area_i - area_a)  << std::endl
        ;
}

template <typename T>
void test_all(std::string const& p_type,
        std::string const& p_selection,
        std::string const& c_selection,
        int min_points, int max_points, long double rotation)
{
    test_star<T, float>(p_type, "f", p_selection, c_selection,
                min_points, max_points, rotation);
    test_star<T, double>(p_type, "d", p_selection, c_selection,
                min_points, max_points, rotation);
    test_star<T, long double>(p_type, "e", p_selection, c_selection,
                min_points, max_points, rotation);

#if defined(HAVE_TTMATH)
    test_star<T, ttmath_big>(p_type, "t",
                p_selection, c_selection, min_points, max_points, rotation);
#endif
}

int main(int argc, char** argv)
{
    try
    {
        std::string p_selection = argc > 1 ? argv[1] : "";
        std::string c_selection = argc > 2 ? argv[2] : "";
        int min_points = argc > 3 ? boost::lexical_cast<int>(argv[3]) : 9;
        int max_points = argc > 4 ? boost::lexical_cast<int>(argv[4]) : min_points;
        long double rotation = argc > 5 ? boost::lexical_cast<long double>(argv[5]) : 1e-15;

        test_all<float>("f", p_selection, c_selection, min_points, max_points, rotation);
        test_all<double>("d", p_selection, c_selection, min_points, max_points, rotation);
        test_all<long double>("e", p_selection, c_selection, min_points, max_points, rotation);

#if defined(HAVE_TTMATH)
        test_all<ttmath_big>("t",
                    p_selection, c_selection, min_points, max_points, rotation);
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
