// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/util/write_dsv.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>


template <typename Geometry>
void test_geometry(std::string const& wkt, std::string const& expected)
{
    Geometry geometry;

    boost::geometry::read_wkt(wkt, geometry);
    boost::geometry::correct(geometry);

    std::ostringstream out;
    out << boost::geometry::wkt(geometry);

    BOOST_CHECK_EQUAL(out.str(), expected);
}


// Note: 3D/box test cannot be done using WKT because:
// -> wkt-box does not exist
// -> so it is converted to a ring
// -> ring representation of 3d-box is not supported, nor feasible
// -> so it uses DSV which can represent a box
template <typename Geometry>
void test_geometry_dsv(std::string const& wkt, std::string const& expected)
{
    Geometry geometry;

    boost::geometry::read_wkt(wkt, geometry);
    boost::geometry::correct(geometry);

    std::ostringstream out;
    out << boost::geometry::dsv(geometry);

    BOOST_CHECK_EQUAL(out.str(), expected);
}






template <typename P>
void test_all()
{
    // Define clockwise and counter clockwise polygon
    std::string cw_ring =
            "POLYGON((0 0,0 1,1 1,1 0,0 0))";
    std::string ccw_ring =
            "POLYGON((0 0,1 0,1 1,0 1,0 0))";

    // already cw_ring
    test_geometry<boost::geometry::linear_ring<P> >(cw_ring, cw_ring);

    // wrong order
    test_geometry<boost::geometry::linear_ring<P> >(ccw_ring, cw_ring);

    // not closed
    test_geometry<boost::geometry::linear_ring<P> >(
            "POLYGON((0 0,1 0,1 1,0 1))",
            cw_ring);

    // counter clockwise, cw_ring
    test_geometry<boost::geometry::linear_ring<P, std::vector, false> >
            (ccw_ring, ccw_ring);

    test_geometry<boost::geometry::linear_ring<P, std::vector, false> >
            (cw_ring, ccw_ring);


    // polygon: cw_ring
    test_geometry<boost::geometry::polygon<P> >(cw_ring, cw_ring);
    // wrong order
    test_geometry<boost::geometry::polygon<P> >(
            "POLYGON((0 0,1 0,1 1,0 1,0 0))",
            cw_ring);
    // wrong order & not closed
    test_geometry<boost::geometry::polygon<P> >(
            "POLYGON((0 0,1 0,1 1,0 1))",
            cw_ring);


    std::string cw_holey_polygon =
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,2 1,2 2,1 2,1 1))";
    std::string ccw_holey_polygon =
            "POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,1 2,2 2,2 1,1 1))";

    // with holes: cw_ring
    test_geometry<boost::geometry::polygon<P> >(
            cw_holey_polygon,
            cw_holey_polygon);
    // wrong order of main
    test_geometry<boost::geometry::polygon<P> >(
            "POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1))",
            cw_holey_polygon);
    // wrong order of hole
    test_geometry<boost::geometry::polygon<P> >(
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 2,2 2,2 1,1 1))",
            cw_holey_polygon);

    // wrong order of main and hole
    test_geometry<boost::geometry::polygon<P> >(ccw_holey_polygon, cw_holey_polygon);

    // test the counter-clockwise
    test_geometry<boost::geometry::polygon<P, std::vector, std::vector, false> >(
            ccw_holey_polygon, ccw_holey_polygon);

    // Boxes
    std::string proper_box = "POLYGON((0 0,0 2,2 2,2 0,0 0))";
    test_geometry<boost::geometry::box<P> >(proper_box, proper_box);
    test_geometry<boost::geometry::box<P> >("BOX(0 0,2 2)", proper_box);
    test_geometry<boost::geometry::box<P> >("BOX(2 2,0 0)", proper_box);
    test_geometry<boost::geometry::box<P> >("BOX(0 2,2 0)", proper_box);

    // Cubes
    typedef boost::geometry::box<boost::geometry::point<double, 3, boost::geometry::cs::cartesian> > box3d;
    std::string proper_3d_dsv_box = "((0, 0, 0), (2, 2, 2))";
    test_geometry_dsv<box3d>("BOX(0 0 0,2 2 2)", proper_3d_dsv_box);
    test_geometry_dsv<box3d>("BOX(2 2 2,0 0 0)", proper_3d_dsv_box);
    test_geometry_dsv<box3d>("BOX(0 2 2,2 0 0)", proper_3d_dsv_box);
    test_geometry_dsv<box3d>("BOX(2 0 2,0 2 0)", proper_3d_dsv_box);
    test_geometry_dsv<box3d>("BOX(0 0 2,2 2 0)", proper_3d_dsv_box);
}


int test_main(int, char* [])
{
    //test_all<int[2]>();
    //test_all<float[2]>(); not yet because cannot be copied, for polygon
    //test_all<double[2]>();

    //test_all<point_xy<int> >();
    //test_all<point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
