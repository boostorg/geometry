// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Linestring Example

#include <algorithm> // for reverse, unique
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
// Optional includes to handle c-arrays as points, std::vectors as linestrings
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>



template<typename P>
inline void translate_function(P& p)
{
        p.x(p.x() + 100.0);
}

template<typename P>
struct scale_functor
{
    inline void operator()(P& p)
    {
        p.x(p.x() * 1000.0);
        p.y(p.y() * 1000.0);
    }
};

int main(void)
{
    using namespace boost::geometry;

    // Define a linestring, which is a vector of points, and add some points
    // (we add them deliberately in different ways)
    linestring_2d ls;

    // points can be created using "make" and added to a linestring using the std:: "push_back"
    ls.push_back(make<point_2d>(1.1, 1.1));

    // points can also be assigned using "assign" and added to a linestring using "append"
    point_2d lp;
    assign(lp, 2.5, 2.1);
    append(ls, lp);

    // Lines can be streamed using DSV (delimiter separated values)
    std::cout << boost::geometry::dsv(ls) << std::endl;

    // The bounding box of linestrings can be calculated
    box_2d b;
    envelope(ls, b);
    std::cout << boost::geometry::dsv(b) << std::endl;

    // The length of the line can be calulated
    std::cout << "length: " << length(ls) << std::endl;

    // All things from std::vector can be called, because a linestring is a vector
    std::cout << "number of points 1: " << ls.size() << std::endl;

    // All things from boost ranges can be called because a linestring is considered as a range
    std::cout << "number of points 2: " << boost::size(ls) << std::endl;

    // Generic function from geometry/OGC delivers the same value
    std::cout << "number of points 3: " << num_points(ls) << std::endl;

    // The distance from a point to a linestring can be calculated
    point_2d p(1.9, 1.2);
    std::cout << "distance of " << boost::geometry::dsv(p)
        << " to line: " << distance(p, ls) << std::endl;

    // A linestring is a vector. However, some algorithms consider "segments",
    // which are the line pieces between two points of a linestring.
    double d = distance(p, segment<point_2d >(ls.front(), ls.back()));
    std::cout << "distance: " << d << std::endl;

    // Add some three points more, let's do it using a classic array.
    // (See documentation for picture of this linestring)
    const double c[][2] = { {3.1, 3.1}, {4.9, 1.1}, {3.1, 1.9} };
    append(ls, c);
    std::cout << "appended: " << boost::geometry::dsv(ls) << std::endl;

    // Output as iterator-pair on a vector
    {
        std::vector<point_2d> v;
        std::copy(ls.begin(), ls.end(), std::back_inserter(v));

        std::cout
            << "as vector: "
            << boost::geometry::dsv(v)
            << std::endl;

        std::cout
            << "as it-pair: "
            << boost::geometry::dsv(std::make_pair(v.begin(), v.end()))
            << std::endl;
    }

    // All algorithms from std can be used: a linestring is a vector
    std::reverse(ls.begin(), ls.end());
    std::cout << "reversed: " << boost::geometry::dsv(ls) << std::endl;
    std::reverse(boost::begin(ls), boost::end(ls));

    // The other way, using a vector instead of a linestring, is also possible
    std::vector<point_2d> pv(ls.begin(), ls.end());
    std::cout << "length: " << length(pv) << std::endl;

    // If there are double points in the line, you can use unique to remove them
    // So we add the last point, print, make a unique copy and print
    {
        // (sidenote, we have to make copies, because
        // ls.push_back(ls.back()) often succeeds but
        // IS dangerous and erroneous!
        point_2d last = ls.back(), first = ls.front();
        ls.push_back(last);
        ls.insert(ls.begin(), first);
    }
    std::cout << "extra duplicate points: " << boost::geometry::dsv(ls) << std::endl;

    {
        linestring_2d ls_copy;
        std::unique_copy(ls.begin(), ls.end(), std::back_inserter(ls_copy),
            boost::geometry::equal_to<point_2d>());
        ls = ls_copy;
        std::cout << "uniquecopy: " << boost::geometry::dsv(ls) << std::endl;
    }

    // Lines can be simplified. This removes points, but preserves the shape
    linestring_2d ls_simplified;
    simplify(ls, ls_simplified, 0.5);
    std::cout << "simplified: " << boost::geometry::dsv(ls_simplified) << std::endl;


    // for_each:
    // 1) Lines can be visited with std::for_each
    // 2) for_each_point is also defined for all geometries
    // 3) for_each_segment is defined for all geometries to all segments
    // 4) loop is defined for geometries to visit segments
    //    with state apart, and to be able to break out (not shown here)
    {
        linestring_2d lscopy = ls;
        std::for_each(lscopy.begin(), lscopy.end(), translate_function<point_2d>);
        for_each_point(lscopy, scale_functor<point_2d>());
        for_each_point(lscopy, translate_function<point_2d>);
        std::cout << "modified line: " << boost::geometry::dsv(lscopy) << std::endl;
    }

    // Lines can be clipped using a clipping box. Clipped lines are added to the output iterator
    box_2d cb(point_2d(1.5, 1.5), point_2d(4.5, 2.5));

    std::vector<linestring_2d> clipped;
    intersection_inserter<linestring_2d> (cb, ls, std::back_inserter(clipped));

    // Also possible: clip-output to a vector of vectors
    std::vector<std::vector<point_2d> > vector_out;
    intersection_inserter<std::vector<point_2d> >(cb, ls, std::back_inserter(vector_out));

    std::cout << "clipped output as vector:" << std::endl;
    for (std::vector<std::vector<point_2d> >::const_iterator it
            = vector_out.begin(); it != vector_out.end(); ++it)
    {
        std::cout << boost::geometry::dsv(*it) << std::endl;
    }

    // Calculate the convex hull of the linestring
    polygon_2d hull;
    convex_hull(ls, hull);
    std::cout << "Convex hull:" << boost::geometry::dsv(hull) << std::endl;

    // All the above assumed 2D Cartesian linestrings. 3D is possible as well
    // Let's define a 3D point ourselves, this time using 'float'
    typedef point<float, 3, cs::cartesian> point_type;
    typedef linestring<point_type> line_type;
    line_type line3d;
    line3d.push_back(make<point_type>(1,2,3));
    line3d.push_back(make<point_type>(4,5,6));
    line3d.push_back(make<point_type>(7,8,9));

    // Not all algorithms work on 3d lines. For example convex hull does NOT.
    // But, for example, length, distance, simplify, envelope and stream do.
    std::cout << "3D: length: " << length(line3d) << " line: " << boost::geometry::dsv(line3d) << std::endl;

    // With DSV you can also use other delimiters, e.g. JSON style
    std::cout << "JSON: "
        << boost::geometry::dsv(ls, ", ", "[", "]", ", ", "[ ", " ]")
        << std::endl;

    return 0;
}
