// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <test_common/test_point.hpp>


template <typename P>
void test_all()
{
    typedef typename boost::geometry::coordinate_type<P>::type coordinate_type;

    P p1(0, 0);
    P p2(2, 2);
    boost::geometry::box<P> b1(p1, p2);

    boost::geometry::box<P> b2;
    boost::geometry::buffer(b1, b2, coordinate_type(2));

    // TODO: Check if buffer is correct
    // using boost::geometry::equals to compare boxes
    // (TODO: implement that)
}

int test_main(int, char* [])
{
    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
