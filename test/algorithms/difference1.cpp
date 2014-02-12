// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_difference
#endif

#include <boost/test/included/unit_test.hpp>

#include "test_difference1.hpp"

#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/algorithms/difference.hpp>

typedef bg::model::point<double,2,bg::cs::cartesian>  point_type;
typedef bg::model::segment<point_type>                segment_type;
typedef bg::model::linestring<point_type>             linestring_type;
typedef bg::model::multi_linestring<linestring_type>  multi_linestring_type;



//===========================================================================
//===========================================================================
//===========================================================================


BOOST_AUTO_TEST_CASE( test_difference_linestring_linestring )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl << std::endl;
    std::cout << "*** LINESTRING / LINESTRING DIFFERENCE ***" << std::endl;
    std::cout << std::endl;
#endif

    typedef linestring_type L;
    typedef multi_linestring_type ML;

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,5 0)"),
         from_wkt<L>("LINESTRING(3 0,4 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,3 0),(4 0,5 0))"),
         "lldf01");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(3 0,4 0)"),
         from_wkt<L>("LINESTRING(0 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING()"),
         "lldf01-1");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,4 0)"),
         from_wkt<L>("LINESTRING(3 0,6 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,3 0))"),
         "lldf01-2");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(3 0,6 0)"),
         from_wkt<L>("LINESTRING(0 0,4 0)"),
         from_wkt<ML>("MULTILINESTRING((4 0,6 0))"),
         "lldf01-3");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,6 0)"),
         from_wkt<L>("LINESTRING(0 0,4 0)"),
         from_wkt<ML>("MULTILINESTRING((4 0,6 0))"),
         "lldf01-4");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,4 0)"),
         from_wkt<L>("LINESTRING(0 0,6 0)"),
         from_wkt<ML>("MULTILINESTRING()"),
         "lldf01-5");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,20 0)"),
         from_wkt<L>("LINESTRING(0 0,1 1,2 0,3 1,4 0,5 0,6 1,7 -1,8 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,4 0),(5 0,20 0))"),
         "lldf01-6");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-20 0,20 0)"),
         from_wkt<L>("LINESTRING(0 0,1 1,2 0,3 1,4 0,5 0,6 1,7 -1,8 0)"),
         from_wkt<ML>("MULTILINESTRING((-20 0,4 0),(5 0,20 0))"),
         "lldf01-7");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,4 0)"),
         from_wkt<L>("LINESTRING(2 0,4 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0))"),
         "lldf01-8");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(2 0,4 0)"),
         from_wkt<L>("LINESTRING(0 0,4 0)"),
         from_wkt<ML>("MULTILINESTRING()"),
         "lldf01-9");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,2 0)"),
         from_wkt<L>("LINESTRING(4 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0))"),
         "lldf01-10");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,2 0)"),
         from_wkt<L>("LINESTRING(2 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0))"),
         "lldf01-11");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,4 0)"),
         from_wkt<L>("LINESTRING(3 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0,3 0))"),
         "lldf01-11a");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,4 0)"),
         from_wkt<L>("LINESTRING(3 0,4 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0,3 0))"),
         "lldf01-11b");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,6 0)"),
         from_wkt<L>("LINESTRING(2 0,4 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0),(5 0,6 0))"),
         "lldf01-12");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,5 5,10 5,15 0)"),
         from_wkt<L>("LINESTRING(-1 6,0 5,15 5)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0,5 5),(10 5,15 0))"),
         "lldf02");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0)"),
         from_wkt<L>("LINESTRING(-1 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0))"),
         "lldf03");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,5 5,10 5,15 0,20 0)"),
         from_wkt<L>("LINESTRING(-1 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((1 0,5 5,10 5,15 0))"),
         "lldf04");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,25 1)"),
         from_wkt<L>("LINESTRING(-1 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0),\
                      (20 0,25 1))"),
         "lldf05");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,30 0)"),
         from_wkt<L>("LINESTRING(-1 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0))"),
         "lldf05-1");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,31 0)"),
         from_wkt<L>("LINESTRING(-1 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0),\
                      (30 0,31 0))"),
         "lldf06");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,31 0)"),
         from_wkt<L>("LINESTRING(-1 0,25 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0),\
                      (30 0,31 0))"),
         "lldf07");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,31 0)"),
         from_wkt<L>("LINESTRING(-1 0,19 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0),\
                      (30 0,31 0))"),
         "lldf08");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,30 0,31 1)"),
         from_wkt<L>("LINESTRING(-1 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0),\
                      (30 0,31 1))"),
         "lldf09");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,5 5,10 5,15 0,20 0,30 0,31 1)"),
         from_wkt<L>("LINESTRING(-1 -1,0 0,1 0,2 1,3 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(1 0,5 5,10 5,15 0),\
                      (30 0,31 1))"),
         "lldf10");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,4 0,5 5,10 5,15 0,20 0,\
                                 30 0,31 1)"),
         from_wkt<L>("LINESTRING(-1 -1,0 0,1 0,2 0,2.5 1,3 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(2 0,3 0),\
                      (4 0,5 5,10 5,15 0),(30 0,31 1))"),
         "lldf11");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(-1 1,0 0,1 0,4 0,5 5,10 5,15 0,31 0)"),
         from_wkt<L>("LINESTRING(-1 -1,0 0,1 0,2 0,2.5 1,3 0,30 0)"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0),(2 0,3 0),\
                      (4 0,5 5,10 5,15 0),(30 0,31 0))"),
         "lldf11-1");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,2 0,3 1)"),
         from_wkt<L>("LINESTRING(0 0,2 0,3 1)"),
         from_wkt<ML>("MULTILINESTRING()"),
         "lldf12");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,2 0,3 1)"),
         from_wkt<L>("LINESTRING(3 1,2 0,0 0)"),
         from_wkt<ML>("MULTILINESTRING()"),
         "lldf12-1");

   test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,2 1,3 5,4 0)"),
         from_wkt<L>("LINESTRING(1 0,2 1,3 5,4 0,5 10)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0))"),
         "lldf13");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,2 0,2.5 0,3 1)"),
         from_wkt<L>("LINESTRING(0 0,2 0,2.5 0,3 1)"),
         from_wkt<ML>("MULTILINESTRING()"),
         "lldf14");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,2 1,3 5,4 0)"),
         from_wkt<L>("LINESTRING(1 0,2 1,3 5)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(3 5,4 0))"),
         "lldf15");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,2 1,3 2)"),
         from_wkt<L>("LINESTRING(0.5 0,1 0,3 2,4 5)"),
         from_wkt<ML>("MULTILINESTRING((0 0,0.5 0))"),
         "lldf16");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,1 0,2 1,3 2)"),
         from_wkt<L>("LINESTRING(4 5,3 2,1 0,0.5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,0.5 0))"),
         "lldf16-r");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,10 0,20 1,30 1)"),
         from_wkt<L>("LINESTRING(1 1,2 0,3 1,20 1,25 1)"),
         from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(25 1,30 1))"),
         "lldf17");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,10 0,20 1,21 0,30 0)"),
         from_wkt<L>("LINESTRING(1 1,2 0,3 1,20 1,25 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1,21 0,30 0))"),
         "lldf18");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0,5 1)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(5 1,4 0,4 1,20 1,5 0,1 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19-r");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19a");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(4 0,4 1,20 1,5 0,1 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19a-r");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0,5 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19b");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0,5 0,6 1)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19c");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0,3 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19d");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0,3 0,3 1)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19e");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,4 0,5 0,5 1)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19f");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(5 1,5 0,4 0,4 1,20 1,5 0,1 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19f-r");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(1 0,5 0,20 1,4 1,5 0,5 1)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19g");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0)"),
         from_wkt<L>("LINESTRING(5 1,5 0,4 1,20 1,5 0,1 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(5 0,30 0))"),
         "lldf19g-r");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0,30 30,10 30,10 -10,15 0,40 0)"),
         from_wkt<L>("LINESTRING(5 5,10 0,10 30,20 0,25 0,25 25,50 0,35 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,20 0),(25 0,30 0,30 30,10 30),\
                       (10 0,10 -10,15 0,20 0),(25 0,35 0))"),
         "lldf20");

    test_difference_of_linestrings()
        (from_wkt<L>("LINESTRING(0 0,30 0,30 30,10 30,10 -10,15 0,40 0)"),
         from_wkt<L>("LINESTRING(5 5,10 0,10 30,20 0,25 0,25 25,50 0,15 0)"),
         from_wkt<ML>("MULTILINESTRING((0 0,15 0),(30 0,30 30,10 30),\
                       (10 0,10 -10,15 0))"),
         "lldf20a");
}



BOOST_AUTO_TEST_CASE( test_difference_multilinestring_multilinestring )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl << std::endl;
    std::cout << "*** MULTILINESTRING / MULTILINESTRING DIFFERENCE ***"
              << std::endl;
    std::cout << std::endl;
#endif

    typedef multi_linestring_type ML;

    // disjoint linestrings
    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0))"),
         from_wkt<ML>("MULTILINESTRING((1 1,2 2,4 3),(1 1,2 2,5 3))"),
         from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0))"),
         "mlmldf01"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0))"),
         from_wkt<ML>("MULTILINESTRING((1 1,2 0,4 0),(1 1,3 0,4 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0),(4 0,10 0,20 1),\
                      (1 0,2 0),(4 0,7 0))"),
         "mlmldf02"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0))"),
         from_wkt<ML>("MULTILINESTRING((1 1,2 0,4 0),(1 1,3 0,5 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0),(5 0,10 0,20 1),\
                      (1 0,2 0),(5 0,7 0))"),
         "mlmldf03"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0))"),
         from_wkt<ML>("MULTILINESTRING((1 1,2 0,4 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0),(4 0,10 0,20 1),\
                      (1 0,2 0),(4 0,7 0))"),
         "mlmldf04"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0),\
                       (10 10,20 10,30 20))"),
         from_wkt<ML>("MULTILINESTRING((1 1,2 0,4 0),\
                       (10 20,15 10,25 10,30 15))"),
         from_wkt<ML>("MULTILINESTRING((0 0,2 0),(4 0,10 0,20 1),\
                      (1 0,2 0),(4 0,7 0),(10 10,15 10),(20 10,30 20))"),
         "mlmldf05"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,10 0,20 1),(1 0,7 0),\
                       (10 10,20 10,30 20))"),
         from_wkt<ML>("MULTILINESTRING((1 1,2 0,4 0),\
                       (-1 -1,0 0,9 0,11 10,12 10,13 0.3,14 0.4,15 0.5),\
                       (10 20,15 10,25 10,30 15))"),
         from_wkt<ML>("MULTILINESTRING((9 0,10 0,13 0.3),(15 0.5,20 1),\
                      (10 10,11 10),(12 10,15 10),(20 10,30 20))"),
         "mlmldf06"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((-1 -1,1 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0))"),
         "mlmldf07"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((-1 1,0 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((-1 -1,0 0,50 0),\
                      (19 -1,20 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING((-1 1,0 0))"),
         "mlmldf07a"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((-1 -1,0 0,50 0),\
                      (19 -1,20 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING()"),
         "mlmldf07b"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 1,2 0),\
                       (-1 -1,1 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0))"),
         "mlmldf08"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,1 0,2 0.5,3 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 1,2 0.5),\
                       (-1 -1,1 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0,2 0.5,3 0))"),
         "mlmldf09"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,1 0,1.5 0,2 0.5,3 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 1,1 0,2 0.5),\
                       (-1 -1,1 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(1.5 0,2 0.5,3 0))"),
         "mlmldf10"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,1 1,100 1,101 0),\
                       (0 0,101 0))"),
         from_wkt<ML>("MULTILINESTRING((1 0,1 1,2 1,3 0,4 0,5 1,6 1,\
                       7 0,8 0,9 1,10 1,11 0,12 0,13 1,14 1,15 0),\
                       (-1 -1,1 0,101 0,200 -1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 1),(2 1,5 1),(6 1,9 1),\
                       (10 1,13 1),(14 1,100 1,101 0),(0 0,1 0))"),
         "mlmldf11"
        );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                      (1 1,2 0,18 0,19 1),(2 1,3 0,17 0,18 1),\
                      (3 1,4 0,16 0,17 1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf12"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((1 0,19 0,20 1),\
                      (2 0,18 0,19 1),(3 0,17 0,18 1),\
                      (4 0,16 0,17 1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf13"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1,19 1,18 0,2 0,\
                       1 1,2 1,3 0,17 0,18 1,17 1,16 0,4 0,3 1))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf14"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (2 2,4 2,6 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf15"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (6 0,4 2,2 2))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf15a"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (2 2,4 2,5 0,6 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf16"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,20 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (6 0,5 0,4 2,2 2))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,20 0))"),
         "mlmldf16a"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,30 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (2 2,4 0,5 2,20 2,25 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,30 0))"),
         "mlmldf17"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,30 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (2 2,4 0,5 2,20 2,25 0,26 2))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,30 0))"),
         "mlmldf17a"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,30 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (2 2,5 -1,15 2,18 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,30 0))"),
         "mlmldf18"
         );

    test_difference_of_multilinestrings()
        (from_wkt<ML>("MULTILINESTRING((0 0,18 0,19 0,30 0))"),
         from_wkt<ML>("MULTILINESTRING((0 1,1 0,19 0,20 1),\
                       (2 2,5 -1,15 2,18 0))"),
         from_wkt<ML>("MULTILINESTRING((0 0,1 0),(19 0,30 0))"),
         "mlmldf18a"
         );
}
