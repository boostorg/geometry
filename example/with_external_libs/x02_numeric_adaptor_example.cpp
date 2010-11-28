// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Numeric Adaptor Example

// The Numeric Adaptor is introduced to the Boost mailing list
// It is a proxy to high precision arithmetic libraries such as gmp
// However, it might be that the same effect can be used using the
// Boost.Math bindings.

// To build and run this example:
// 1) download gmp, it should be somewhere in the include path
// 2) download numeric_adaptor from the Boost.Sandbox

#include <iostream>
#include <iomanip>
#include <typeinfo>

#include <boost/numeric_adaptor/numeric_adaptor.hpp>
#include <boost/numeric_adaptor/gmp_value_type.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>

template <typename Type, typename AssignType>
void calculate(AssignType const& x1,
               AssignType const& y1,
               AssignType const& x2,
               AssignType const& y2,
               AssignType const& x3,
               AssignType const& y3
               )
{
    // gmp can be used instead of "double" for any point type
    typedef boost::geometry::point_xy<Type> point_type;

    point_type a, b, c;
    boost::geometry::assign(a, boost::to<Type>(x1), boost::to<Type>(y1));
    boost::geometry::assign(b, boost::to<Type>(x2), boost::to<Type>(y2));
    boost::geometry::assign(c, boost::to<Type>(x3), boost::to<Type>(y3));

    boost::geometry::linear_ring<point_type> r;
    r.push_back(a);
    r.push_back(b);
    r.push_back(c);
    r.push_back(a);

    // What also is possible is define point coordinates using IEEE double,
    // but doing calculations using the gmp type.
    // To do that, specify the strategy explicitly
    Type ab = boost::geometry::distance(a, b);
    Type bc = boost::geometry::distance(b, c);
    Type ca = boost::geometry::distance(c, a);

    std::cout << std::endl << typeid(Type).name() << std::endl;

    std::cout << "a-b: " << ab << std::endl;
    std::cout << "b-c: " << bc << std::endl;
    std::cout << "c-a: " << ca << std::endl;

    std::cout << "area:  " << boost::geometry::area(r,
            boost::geometry::strategy::area::by_triangles<point_type, Type>())
        << std::endl;

    // Heron formula is "famous" for its imprecision. It should give
    // same result as area, but is sensible for rounding errors.
    Type s = ab + bc + ca;
    s /= 2.0;
    Type ar = boost::sqrt(s * (s - ab) * (s - bc) * (s - ca));
    std::cout << "heron: " << ar << std::endl;

    // Area's given:
    // float:       740.74078369140625
    // double:      740.74073407406990554591
    // long double: 740.74073407406991376156
    // GMP:        0.74074073407407e3 (right!)

    // SQL Server:  740.740724252642
    // Postgis:     740.74073407407 (might be rounded from (long) double)
    // MySQL:       740.74073407406991000000
}

int main(int argc, char *argv[])
{
    typedef boost::numeric_adaptor::gmp_value_type type;
    //typedef long double type;

    // gmp can be used instead of "double" for any point type
    typedef boost::geometry::point_xy<type> point_type;

    // Points, polygons or other geometries are gmp now
    point_type p;

    // They can be used normally
    boost::geometry::set<0>(p, 123456.78900001);
    std::cout << "x coordinate: " << boost::geometry::get<0>(p) << std::endl;

    // But the value above cannot be expressed with that precision in IEEE 64 bits.
    // Points can therefore also be assigned by string with boost::geometry::set
    // (boost::to is a converser included in the Numeric Adaptor sources)
    boost::geometry::set<0>(p, boost::to<type>(std::string("123456.78900001")));

    // and streamed (as a string representation)
    std::cout << "x coordinate: " << boost::geometry::get<0>(p) << std::endl;


    // The boost::geometry::assign function also supports custom numeric types
    point_type p1, p2;
    boost::geometry::assign(p1,
        boost::to<type>(std::string("123456.78900001")),
        boost::to<type>(std::string("234567.89100001")));
    boost::geometry::assign(p2,
        boost::to<type>(std::string("987654.32100001")),
        boost::to<type>(std::string("876543.21900001")));

    type d = boost::geometry::distance(p1, p2);
    std::cout << "Exact distance: " << d << std::endl;
    // It gives:          0.1076554 54858339556783e7
    // my calculator gives: 1076554.5485833955678294387789057
    // CLN gives          : 1076554.5485833955

    // All algorithms will automatically use the gmp-type
    // We show and compare that in the calculate function, with type and
    // assigning type as template parameters

    std::cout << std::fixed << std::setprecision(20);
    calculate<float>(0.0, 0.0, 0.0, 0.0012, 1234567.89012345, 0.0);
    calculate<double>(0.0, 0.0, 0.0, 0.0012, 1234567.89012345, 0.0);
    calculate<long double>(0.0, 0.0, 0.0, 0.0012, 1234567.89012345, 0.0);
    calculate<type, std::string>("0.0", "0.0", "0.0", "0.0012", "1234567.89012345", "0.0");

    return 0;
}

