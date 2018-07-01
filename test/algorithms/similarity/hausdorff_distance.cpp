// Boost.Geometry

// Copyright (c) 2018 Yaghyavardhan Singh Khangarot, Hyderabad, India.

// Contributed and/or modified by Yaghyavardhan Singh Khangarot, as part of Google Summer of Code 2018 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <vector>


#include "./test_hausdorff_distance.hpp"

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/include/size.hpp>



#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
BOOST_GEOMETRY_REGISTER_LINESTRING_TEMPLATED(std::vector)
BOOST_GEOMETRY_REGISTER_LINESTRING_TEMPLATED(std::deque)
using namespace boost::geometry;
template <typename P>
void test_all()
{
	typedef model::linestring<P> linestring_2d;
    typedef bg::model::multi_linestring<linestring_2d> mlinestring_t;
    typedef bg::model::multi_point<P> mpoint_t;
    
    P p1;
    p1.set<0>(1.0);
    p1.set<1>(0.0);

    #ifdef BOOST_GEOMETRY_DETAIL_DEBUG
    typedef coordinate_system<point_2d>::type CordType;
    std::cout << typeid(CordType).name() << std::endl;
    std::cout << dsv(ls1) << std::endl;
    std::cout << dsv(ls2) << std::endl;
    std::cout << dsv(mpt1) << std::endl;
    std::cout << dsv(mpt2) << std::endl;
    std::cout << dsv(mls1) << std::endl;
    std::cout << dsv(mls2) << std::endl;
    double l = boost::size(mls1);
	std::cout << l << std::endl;
    #endif
    
    //test_hausdorff_distance(ls1,ls2,strategy::distance::pythagoras<>(),0);
    test_geometry<P,mpoint_t>("POINT(0 0)","MULTIPOINT(0 0,3 4,4 3)", 0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0,3 4,4 3)","LINESTRING(0 0,3 4,4 3)", 0);
    test_geometry<mpoint_t,mpoint_t>("MULTIPOINT(0 0,3 4,4 3)","MULTIPOINT(0 0,3 4,4 3)", 0);
    test_geometry<linestring_2d,mlinestring_t >("LINESTRING(0 0,3 4,4 3)","MULTILINESTRING((0 0,3 4,4 3),(0 0,3 4,4 3))", 0);
    test_geometry<mlinestring_t,mlinestring_t >("MULTILINESTRING((0 0,3 4,4 3),(0 0,3 4,4 3))","MULTILINESTRING((0 0,3 4,4 3),(0 0,3 4,4 3))", 0);

}

int main()
{
	
    typedef model::d2::point_xy<int> point_2d;
    test_all<point_2d>();
    test_all<model::d2::point_xy<float> >();
    test_all<model::d2::point_xy<double> >();

    return 0;
}
