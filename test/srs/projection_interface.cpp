// Boost.Geometry
// Unit Test

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/srs/projection.hpp>


int test_main(int, char*[])
{
    using namespace boost::geometry;
    using namespace boost::geometry::model;
    using namespace boost::geometry::srs;

    typedef point<double, 2, cs::geographic<degree> > point_ll;
    typedef point<double, 2, cs::cartesian> point_xy;

    std::cout << std::setprecision(12);

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        projection<> prj = proj4("+proj=tmerc +ellps=WGS84 +units=m");
        
        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        projection<> prj = epsg(2000);

        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        // default WGS84 spheroid and additional parameters
        projection<static_proj4<proj::tmerc> > prj;

        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        projection<static_epsg<2000> > prj;

        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        // default spheroid and additional parameters
        projection<static_proj4<proj::tmerc, ellps::WGS84> >
            prj2;

        // default spheroid and additional parameters
        projection<static_proj4<proj::tmerc, ellps::WGS84> >
            prj3 = static_proj4<proj::tmerc, ellps::WGS84>();

        // passed spheroid and default additional parameters
        projection<static_proj4<proj::tmerc, ellps::WGS84> >
            prj4 = static_proj4<proj::tmerc, ellps::WGS84>(ellps::WGS84());

        // default spheroid and passed additional parameters
        projection<static_proj4<proj::tmerc, ellps::WGS84> >
            prj5 = static_proj4<proj::tmerc, ellps::WGS84>("");

        // passed spheroid and additional parameters
        projection<static_proj4<proj::tmerc, ellps::WGS84> >
            prj6 = static_proj4<proj::tmerc, ellps::WGS84>(ellps::WGS84(), "");
    }

    // compile-time errors
    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        //projection<static_proj4<int> > prj1;
        //projection<int> prj2;

        projection<static_proj4<proj::bacon> > prj3;
        //prj3.inverse(pt_xy, pt_ll);
    }

    // run-time errors
    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        try {
            projection<> prj1 = proj4("");
        } catch (exception & e) {
            std::cerr << e.what() << std::endl;
        }

        try {
            projection<> prj2 = proj4("+proj=abcd");
        } catch (exception & e) {
            std::cerr << e.what() << std::endl;
        }

        try {
            projection<> prj3 = proj4("+proj=bacon +a=6400000");
            prj3.inverse(pt_xy, pt_ll);
        } catch (exception & e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}
