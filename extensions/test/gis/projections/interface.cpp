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
#include <boost/geometry/extensions/gis/projections/projection.hpp>


int test_main(int, char*[])
{
    using namespace boost::geometry;
    using namespace boost::geometry::model;
    using namespace boost::geometry::projections;

    typedef point<double, 2, cs::geographic<degree> > point_ll;
    typedef point<double, 2, cs::cartesian> point_xy;

    std::cout << std::setprecision(12);

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        projection<point_ll, point_xy> prj(proj4("+proj=tmerc +ellps=WGS84 +units=m"));
        
        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        projection<point_ll, point_xy> prj(epsg(2000));

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
        projection<point_ll, point_xy, static_proj4<tmerc> > prj;

        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        projection<point_ll, point_xy, static_epsg<2000> > prj;

        std::cout << wkt(pt_ll) << std::endl;
        prj.forward(pt_ll, pt_xy);
        std::cout << wkt(pt_xy) << std::endl;
        prj.inverse(pt_xy, pt_ll);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        // default spheroid and additional parameters
        projection<point_ll, point_xy, static_proj4<tmerc, srs::spheroid<double> > >
            prj2;

        // default spheroid and additional parameters
        projection<point_ll, point_xy, static_proj4<tmerc, srs::spheroid<double> > >
            prj3((static_proj4<tmerc, srs::spheroid<double> >()));

        // passed spheroid and default additional parameters
        projection<point_ll, point_xy, static_proj4<tmerc, srs::spheroid<double> > >
            prj4((static_proj4<tmerc, srs::spheroid<double> >(srs::spheroid<double>())));

        // default spheroid and passed additional parameters
        projection<point_ll, point_xy, static_proj4<tmerc, srs::spheroid<double> > >
            prj5((static_proj4<tmerc, srs::spheroid<double> >("")));

        // passed spheroid and additional parameters
        projection<point_ll, point_xy, static_proj4<tmerc, srs::spheroid<double> > >
            prj6((static_proj4<tmerc, srs::spheroid<double> >(srs::spheroid<double>(), "")));
    }

    return 0;
}
