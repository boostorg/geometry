// Boost.Geometry

// Copyright (c) 2018 Yaghyavardhan Singh Khangarot, Hyderabad, India.

// Contributed and/or modified by Yaghyavardhan Singh Khangarot, as part of Google Summer of Code 2018 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <vector>

#define BOOST_GEOMETRY_TEST_DEBUG

#include <algorithms/test_frechet_distance.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

using namespace boost::geometry;

template <typename P>
void test_all_cartesian()
{
    typedef model::linestring<P> linestring_2d;
    #ifdef BOOST_GEOMETRY_TEST_DEBUG
    typedef typename coordinate_system<P>::type CordType;
    std::cout << typeid(CordType).name() << std::endl;
    #endif
    
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(3 0,2 1,3 2)","LINESTRING(0 0,3 4,4 3)", 3);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(3 0,2 1,3 2)","LINESTRING(0 0,3 4,4 3)",strategy::distance::pythagoras<>(), 3);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)","LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)","LINESTRING(1 1, 0 1, 0 0, 1 0, 1 1)",sqrt(2.0));
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 0)","LINESTRING(0 0, 1 0, 1 1, 0 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 0)","LINESTRING(1 1, 0 0, 1 0, 1 1)",sqrt(2.0));
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0)","LINESTRING(0 0, 1 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0,3 4,4 3)","LINESTRING(4 3,3 4,0 0)",5);
    
    
}

template <typename P>
void test_all_geographic()
{
    typedef model::linestring<P> linestring_2d;
    
    #ifdef BOOST_GEOMETRY_TEST_DEBUG
    typedef typename coordinate_system<P>::type CordType;
    std::cout << typeid(CordType).name() << std::endl;
    #endif
    
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(3 0,2 1,3 2)","LINESTRING(0 0,3 4,4 3)", 333958);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(3 0,2 1,3 2)","LINESTRING(0 0,3 4,4 3)",strategy::distance::haversine<int>(6371000), 333585);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)","LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)","LINESTRING(1 1, 0 1, 0 0, 1 0, 1 1)",156898);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 0)","LINESTRING(0 0, 1 0, 1 1, 0 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 0)","LINESTRING(1 1, 0 0, 1 0, 1 1)",156898);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0)","LINESTRING(0 0, 1 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0,3 4,4 3)","LINESTRING(4 3,3 4,0 0)",555093);
       
}

template <typename P>
void test_all_spherical_equ()
{
    typedef model::linestring<P> linestring_2d;
    
    #ifdef BOOST_GEOMETRY_TEST_DEBUG
    typedef typename coordinate_system<P>::type CordType;
    std::cout << typeid(CordType).name() << std::endl;
    #endif
    
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(3 0,2 1,3 2)","LINESTRING(0 0,3 4,4 3)", 0.0523);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(3 0,2 1,3 2)","LINESTRING(0 0,3 4,4 3)",strategy::distance::haversine<int>(), 0.0523);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)","LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 1, 0 0)","LINESTRING(1 1, 0 1, 0 0, 1 0, 1 1)",0.02468);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 0)","LINESTRING(0 0, 1 0, 1 1, 0 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0, 1 1, 0 0)","LINESTRING(1 1, 0 0, 1 0, 1 1)",0.02468);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0, 1 0)","LINESTRING(0 0, 1 0)",0);
    test_geometry<linestring_2d,linestring_2d >("LINESTRING(0 0,3 4,4 3)","LINESTRING(4 3,3 4,0 0)",0.08724);
       
}

int main()
{
    //Cartesian Coordinate System
    test_all_cartesian<model::d2::point_xy<int,cs::cartesian> >();
    test_all_cartesian<model::d2::point_xy<float,cs::cartesian> >();
    test_all_cartesian<model::d2::point_xy<double,cs::cartesian> >();

    //Geographic Coordinate System
    test_all_geographic<model::d2::point_xy<int,cs::geographic<degree> > >();
    test_all_geographic<model::d2::point_xy<float,cs::geographic<degree> > >();
    test_all_geographic<model::d2::point_xy<double,cs::geographic<degree> > >();

    //Spherical_Equatorial Coordinate System
    test_all_spherical_equ<model::d2::point_xy<int,cs::spherical_equatorial<degree> > >();
    test_all_spherical_equ<model::d2::point_xy<float,cs::spherical_equatorial<degree> > >();
    test_all_spherical_equ<model::d2::point_xy<double,cs::spherical_equatorial<degree> > >();

    return 0;
}
