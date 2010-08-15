// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Examples, for e.g. email formal review

#include <boost/foreach.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>



void example_distance()
{
    int a[2] = {1,2};
    int b[2] = {3,4};
    double d = boost::geometry::distance(a, b);
    std::cout << d << std::endl;
}

void example_length1()
{
    std::vector<boost::tuple<double, double, double> > line;
    line.push_back(boost::make_tuple(1, 2, 3));
    line.push_back(boost::make_tuple(4, 5, 6));
    line.push_back(boost::make_tuple(7, 8, 9));
    double length = boost::geometry::length(line);
    std::cout << length << std::endl;
}

void example_length2()
{
    std::vector<boost::tuple<double, double> > line;
    line.push_back(boost::make_tuple(1.1, 2.2));
    line.push_back(boost::make_tuple(3.3, 4.4));
    line.push_back(boost::make_tuple(5.5, 6.6));
    std::cout << boost::geometry::length(
                std::make_pair(boost::begin(line), boost::end(line) + -1)
            )
        << std::endl;
}

void example_less()
{
    typedef boost::tuple<double, double> P;
    std::vector<P> line;
    line.push_back(boost::make_tuple(8.1, 1.9));
    line.push_back(boost::make_tuple(4.2, 7.5));
    line.push_back(boost::make_tuple(2.3, 3.6));
    std::sort(line.begin(), line.end(), boost::geometry::less<P>());

    // Display ordered points
    BOOST_FOREACH(P const& p, line)
    {
        std::cout << boost::geometry::dsv(p) << std::endl;
    }
}



int main(void)
{
    example_distance();
    example_length1();
    example_length2();
    example_less();
    return 0;
}
