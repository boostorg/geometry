// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_TEST_EMPTY_GEOMETRY_HPP
#define BOOST_GEOMETRY_TEST_EMPTY_GEOMETRY_HPP

template <typename Point, typename Geometry1, typename Geometry2,
          typename Strategy>
void test_empty_input(Geometry1 const& geometry1,
                      Geometry2 const& geometry2,
                      Strategy const& strategy)
{
    bg::model::segment<Point> out_seg;

    try
    {
        bg::closest_points(geometry1, geometry2, out_seg);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");

    try
    {
        bg::closest_points(geometry2, geometry1, out_seg);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");

    try
    {
        bg::closest_points(geometry1, geometry2, out_seg, strategy);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");

    try
    {
        bg::closest_points(geometry2, geometry1, out_seg, strategy);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");
}


template <typename Point, typename Strategy>
void test_more_empty_input_pointlike_pointlike(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::multi_point<Point> multipoint_empty;

    Point point = from_wkt<Point>("point(0 0)");

    // 1st geometry is empty
    test_empty_input<Point>(multipoint_empty, point, strategy);

    // 2nd geometry is empty
    test_empty_input<Point>(point, multipoint_empty, strategy);

    // both geometries are empty
    test_empty_input<Point>(multipoint_empty, multipoint_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}


template <typename Point, typename Strategy>
void test_more_empty_input_pointlike_linear(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::linestring<Point> line_empty;
    bg::model::multi_point<Point> multipoint_empty;
    bg::model::multi_linestring<bg::model::linestring<Point> > multiline_empty;

    Point point = from_wkt<Point>("POINT(0 0)");
    bg::model::linestring<Point> line =
        from_wkt<bg::model::linestring<Point> >("LINESTRING(0 0,1 1,2 2)");

    // 1st geometry is empty
    test_empty_input<Point>(multipoint_empty, line, strategy);

    // 2nd geometry is empty
    test_empty_input<Point>(line, multipoint_empty, strategy);
    test_empty_input<Point>(point, line_empty, strategy);
    test_empty_input<Point>(point, multiline_empty, strategy);

    // both geometries are empty
    test_empty_input<Point>(multipoint_empty, line_empty, strategy);
    test_empty_input<Point>(multipoint_empty, multiline_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}


template <typename Point, typename Strategy>
void test_more_empty_input_pointlike_areal(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::multi_point<Point> multipoint_empty;

    bg::model::polygon<Point> polygon_empty;
    bg::model::multi_polygon<bg::model::polygon<Point> > multipolygon_empty;

    Point point = from_wkt<Point>("POINT(0 0)");
    bg::model::polygon<Point> polygon =
        from_wkt<bg::model::polygon<Point> >("POLYGON((0 0,1 0,1 1,0 1,0 0))");

    // 1st geometry is empty
    test_empty_input<Point>(multipoint_empty, polygon, strategy);
    test_empty_input<Point>(polygon_empty, point, strategy);
    test_empty_input<Point>(multipolygon_empty, point, strategy);

    // 2nd geometry is empty
    test_empty_input<Point>(point, polygon_empty, strategy);
    test_empty_input<Point>(point, multipolygon_empty, strategy);
    test_empty_input<Point>(polygon, multipoint_empty, strategy);

    // both geometries are empty
    test_empty_input<Point>(multipoint_empty, polygon_empty, strategy);
    test_empty_input<Point>(multipoint_empty, multipolygon_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}


template <typename Point, typename Strategy>
void test_more_empty_input_linear_linear(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::linestring<Point> line_empty;
    bg::model::multi_linestring<bg::model::linestring<Point> > multiline_empty;

    bg::model::linestring<Point> line =
        from_wkt<bg::model::linestring<Point> >("LINESTRING(0 0,1 1)");

    // 1st geometry is empty
    test_empty_input<Point>(line_empty, line, strategy);
    test_empty_input<Point>(multiline_empty, line, strategy);

    // 2nd geometry is empty
    test_empty_input<Point>(line, line_empty, strategy);
    test_empty_input<Point>(line, multiline_empty, strategy);

    // both geometries are empty
    test_empty_input<Point>(line_empty, line_empty, strategy);
    test_empty_input<Point>(multiline_empty, line_empty, strategy);
    test_empty_input<Point>(multiline_empty, multiline_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}


template <typename Point, typename Strategy>
void test_more_empty_input_linear_areal(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::linestring<Point> line_empty;
    bg::model::multi_linestring<bg::model::linestring<Point> > multiline_empty;

    bg::model::polygon<Point> polygon_empty;
    bg::model::multi_polygon<bg::model::polygon<Point> > multipolygon_empty;

    bg::model::linestring<Point> line =
        from_wkt<bg::model::linestring<Point> >("LINESTRING(0 0,1 1)");
    bg::model::polygon<Point> polygon =
        from_wkt<bg::model::polygon<Point> >("POLYGON((0 0,1 0,1 1,0 1,0 0))");

    // 1st geometry is empty
    test_empty_input<Point>(line_empty, polygon, strategy);
    test_empty_input<Point>(multiline_empty, polygon, strategy);
    test_empty_input<Point>(polygon_empty, line, strategy);
    test_empty_input<Point>(multipolygon_empty, line, strategy);

    // 2nd geometry is empty
    test_empty_input<Point>(line, polygon_empty, strategy);
    test_empty_input<Point>(line, multipolygon_empty, strategy);
    test_empty_input<Point>(polygon, line_empty, strategy);
    test_empty_input<Point>(polygon, multiline_empty, strategy);

    // both geometries are empty
    test_empty_input<Point>(line_empty, polygon_empty, strategy);
    test_empty_input<Point>(line_empty, multipolygon_empty, strategy);
    test_empty_input<Point>(multiline_empty, polygon_empty, strategy);
    test_empty_input<Point>(multiline_empty, multipolygon_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}


template <typename Point, typename Strategy>
void test_more_empty_input_areal_areal(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::polygon<Point> polygon_empty;
    bg::model::multi_polygon<bg::model::polygon<Point> > multipolygon_empty;

    bg::model::polygon<Point> polygon =
        from_wkt<bg::model::polygon<Point> >("POLYGON((0 0,1 0,1 1,0 1,0 0))");

    // 1st geometry is empty
    test_empty_input<Point>(polygon_empty, polygon, strategy);
    test_empty_input<Point>(multipolygon_empty, polygon, strategy);

    // 2nd geometry is empty
    test_empty_input<Point>(polygon, polygon_empty, strategy);
    test_empty_input<Point>(polygon, multipolygon_empty, strategy);

    // both geometries are empty
    test_empty_input<Point>(polygon_empty, multipolygon_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}


#endif // BOOST_GEOMETRY_TEST_EMPTY_GEOMETRY_HPP
