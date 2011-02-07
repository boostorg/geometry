// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>


#include<boost/geometry/geometry.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/point.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/box.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/ring.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/polygon.hpp>
#include<boost/geometry/extensions/gis/io/wkt/wkt.hpp>
#include<iostream>


int test_main(int, char* [])
{
    // 1a: Check if Boost.Polygon's point fulfills Boost.Geometry's point concept
    bg::concept::check<boost::polygon::point_data<double> >();

    // 1b: use a Boost.Polygon point in Boost.Geometry, calc. distance with two point types
    boost::polygon::point_data<double> bpol_point(1, 2);

    typedef bg::model::point<double, 2, bg::cs::cartesian> bg_point_type;
    bg_point_type bgeo_point(3, 4);
    BOOST_CHECK_CLOSE(bg::distance(bpol_point, bgeo_point), 2 * std::sqrt(2.0), 0.001);

    // 2a: Check if Boost.Polygon's box fulfills Boost.Geometry's box concept
    bg::concept::check<boost::polygon::rectangle_data<double> >();

    // 2b: use a Boost.Polygon rectangle in Boost.Geometry, compare with boxes
    boost::polygon::rectangle_data<double> bpol_box;
    bg::model::box<bg_point_type> bgeo_box;

    bg::assign(bpol_box, 0, 1, 5, 6);
    bg::assign(bgeo_box, 0, 1, 5, 6);
    double a1 = bg::area(bpol_box);
    double a2 = bg::area(bgeo_box);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);

    // 3a: Check if Boost.Polygon's polygon fulfills Boost.Geometry's ring concept
    bg::concept::check<boost::polygon::polygon_data<double> >();

    // 3b: use a Boost.Polygon polygon (ring) 
    boost::polygon::polygon_data<double> bpol_ring;
    {
        // Filling it is a two-step process using Boost.Polygon
        std::vector<boost::polygon::point_data<double> > point_vector;
        point_vector.push_back(boost::polygon::point_data<double>(0, 0));
        point_vector.push_back(boost::polygon::point_data<double>(0, 3));
        point_vector.push_back(boost::polygon::point_data<double>(4, 0));
        point_vector.push_back(boost::polygon::point_data<double>(0, 0));
        bpol_ring.set(point_vector.begin(), point_vector.end());
    }

    // Boost-geometry ring
    bg::model::ring<bg_point_type> bgeo_ring;
    {
        bgeo_ring.push_back(bg_point_type(0, 0));
        bgeo_ring.push_back(bg_point_type(0, 3));
        bgeo_ring.push_back(bg_point_type(4, 0));
        bgeo_ring.push_back(bg_point_type(0, 0));
    }
    a1 = bg::area(bpol_ring);
    a2 = bg::area(bgeo_ring);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);

    // Check mutable ring
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0))", bpol_ring);
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0))", bgeo_ring);
    a1 = bg::area(bpol_ring);
    a2 = bg::area(bgeo_ring);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);



    // 4a: Boost.Polygon's polygon with holes
    boost::polygon::polygon_with_holes_data<double> bpol_polygon;
    {
        std::vector<boost::polygon::point_data<double> > point_vector;
        point_vector.push_back(boost::polygon::point_data<double>(0, 0));
        point_vector.push_back(boost::polygon::point_data<double>(0, 10));
        point_vector.push_back(boost::polygon::point_data<double>(10, 10));
        point_vector.push_back(boost::polygon::point_data<double>(10, 0));
        point_vector.push_back(boost::polygon::point_data<double>(0, 0));
        bpol_polygon.set(point_vector.begin(), point_vector.end());
    }

    {
        // Fill the holes (we take two)
        std::vector<boost::polygon::polygon_data<double> > holes;
        holes.resize(2);

        {
            std::vector<boost::polygon::point_data<double> > point_vector;
            point_vector.push_back(boost::polygon::point_data<double>(1, 1));
            point_vector.push_back(boost::polygon::point_data<double>(2, 1));
            point_vector.push_back(boost::polygon::point_data<double>(2, 2));
            point_vector.push_back(boost::polygon::point_data<double>(1, 2));
            point_vector.push_back(boost::polygon::point_data<double>(1, 1));
            holes[0].set(point_vector.begin(), point_vector.end());
        }

        {
            std::vector<boost::polygon::point_data<double> > point_vector;
            point_vector.push_back(boost::polygon::point_data<double>(3, 3));
            point_vector.push_back(boost::polygon::point_data<double>(4, 3));
            point_vector.push_back(boost::polygon::point_data<double>(4, 4));
            point_vector.push_back(boost::polygon::point_data<double>(3, 4));
            point_vector.push_back(boost::polygon::point_data<double>(3, 3));
            holes[1].set(point_vector.begin(), point_vector.end());
        }

        bpol_polygon.set_holes(holes.begin(), holes.end());
    }

    // Using Boost.Polygon
    a1 = bg::area(bpol_polygon);
    a2 = boost::polygon::area(bpol_polygon);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);

    bg::model::polygon<bg_point_type> bgeo_polygon;
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0),(1 1,2 1,2 2,1 2,1 1),(3 3,4 3,4 4,3 4,3 3))", bgeo_polygon);

    a2 = bg::area(bgeo_polygon);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);

    {
        /*
        boost::polygon::polygon_with_holes_data<double> const& pc = bpol_polygon;

        BOOST_AUTO(bh, pc.begin_holes());
        std::cout << typeid(bh).name() << std::endl;
        std::cout << typeid(*bh).name() << std::endl;
        BOOST_AUTO(hole1, *bh);
        BOOST_AUTO(bp, (*hole1.begin()));
        std::cout << typeid(bp).name() << std::endl;

        BOOST_AUTO(e, bg::exterior_ring(pc));
        BOOST_AUTO(b, *boost::begin(e));
        std::cout << typeid(e).name() << std::endl;
        std::cout << typeid(b).name() << std::endl;
        std::cout << bg::area(e) << std::endl;

        BOOST_AUTO(i, bg::interior_rings(pc));
        BOOST_AUTO(bi, *boost::begin(i));
        std::cout << typeid(i).name() << std::endl;
        std::cout << typeid(bi).name() << std::endl;
        BOOST_AUTO(bip, *boost::begin(bi));
        std::cout << typeid(bip).name() << std::endl;
        //std::cout << bg::area(e) << std::endl;


        BOOST_AUTO(inc, bg::interior_rings(bpol_polygon));
        BOOST_AUTO(binc, boost::begin(inc));
        binc++;
        std::cout << typeid(inc).name() << std::endl;
        std::cout << typeid(binc).name() << std::endl;
        */
        /*
        BOOST_AUTO(e0, bg::interior_rings(pc));
        BOOST_AUTO(e1, bg::traits::interior_rings<boost::polygon::polygon_with_holes_data<double> >::get(pc));
        BOOST_AUTO(e2, 
            (bg::core_dispatch::interior_rings<bg::polygon_tag, 
                    boost::polygon::polygon_with_holes_data<double> const>::apply(pc)));
                    */
                    

        /*
        std::cout << typeid(e).name() << std::endl;
        std::cout << typeid(e0).name() << std::endl;
        std::cout << typeid(e1).name() << std::endl;
        std::cout << typeid(e2).name() << std::endl;
        */
    }

    // Test adaption to mutable concept
    bg::clear(bpol_polygon);
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0),(1 1,2 1,2 2,1 2,1 1),(3 3,4 3,4 4,3 4,3 3))", bpol_polygon);

    return 0;
}

