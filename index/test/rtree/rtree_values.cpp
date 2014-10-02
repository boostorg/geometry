// Boost.Geometry Index
// Unit Test

// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/test_rtree.hpp>


#include <boost/geometry/geometries/register/point.hpp>

struct point
{
    point(double xx = 0, double yy = 0) : x(xx), y(yy) {}
    double x, y;
};

BOOST_GEOMETRY_REGISTER_POINT_2D(point, double, bg::cs::cartesian, x, y)


template <typename Box, typename Params>
void test_pair()
{
    typedef std::pair<Box, std::size_t> Value;

    typename boost::remove_const<Box>::type box;
    bg::assign_zero(box);

    Value val(box, 0);

    // sanity check
    std::vector<Value> vec;
    vec.push_back(val);
    vec.push_back(std::make_pair(box, 0));
    vec.push_back(std::make_pair(box, (unsigned short)0));

    bgi::rtree<Value, Params> rt;
    rt.insert(val);
    rt.insert(std::make_pair(box, 0));
    rt.insert(std::make_pair(box, (unsigned short)0));
    BOOST_CHECK( rt.size() == 3 );

    BOOST_CHECK( rt.count(val) == 3 );
    BOOST_CHECK( rt.count(std::make_pair(box, 0)) == 3 );
    BOOST_CHECK( rt.count(std::make_pair(box, (unsigned short)0)) == 3 );
    BOOST_CHECK( rt.count(box) == 3 );

    BOOST_CHECK( rt.remove(val) == 1 );
    BOOST_CHECK( rt.remove(std::make_pair(box, 0)) == 1 );
    BOOST_CHECK( rt.remove(std::make_pair(box, (unsigned short)0)) == 1 );
    BOOST_CHECK( rt.size() == 0 );
}

template <typename Params>
void test_point()
{
    bgi::rtree<point, Params> rt;
    
    rt.insert(0.0);
    BOOST_CHECK( rt.size() == 1 );
    BOOST_CHECK( rt.remove(0.0) == 1 );
}

int test_main(int, char* [])
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> Pt;
    typedef bg::model::box<Pt> Box;

    test_pair< Box, bgi::linear<16> >();
    test_pair< Box, bgi::quadratic<4> >();
    test_pair< Box, bgi::rstar<4> >();
    //test_rtree< Box const, bgi::linear<16> >();
    //test_rtree< Box const, bgi::quadratic<4> >();
    //test_rtree< Box const, bgi::rstar<4> >();

    test_point< bgi::linear<16> >();
    test_point< bgi::quadratic<4> >();
    test_point< bgi::rstar<4> >();

    return 0;
}
