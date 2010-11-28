// Boost.Geometry (aka GGL, Generic Geometry Library) Point concept test file
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry/core/cs.hpp>

#include "function_requiring_a_point.hpp"

struct point
{
    point(): x(), y() {}
    float x, y;
};

namespace boost { namespace geometry { namespace traits {

template <>
struct tag<point>
{
    typedef point_tag type;
};

template <>
struct coordinate_type<point>
{
    typedef float type;
};

template <>
struct coordinate_system<point>
{
    typedef bg::cs::cartesian type;
};

template <>
struct dimension<point>
{
    enum { value = 2 };
};

template <>
struct access<point, 0>
{
    static float get(point const& p)
    {
        return p.x;
    }

    static void set(point& p, float value)
    {
        p.x = value;
    }
};

template <>
struct access<point, 1>
{
    static float get(point const& p)
    {
        return p.y;
    }

    static void set(point& p, float value)
    {
        p.y = value;
    }
};


}}} // namespace bg::traits

int main()
{
    point p1;
    const point p2;
    test::function_requiring_a_point(p1, p2);
    return 0;
}
