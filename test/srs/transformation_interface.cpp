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
#include <boost/geometry/srs/transformation.hpp>

//#include <proj_api.h>

int test_main(int, char*[])
{
    using namespace boost::geometry;
    using namespace boost::geometry::model;
    using namespace boost::geometry::srs;

    //typedef point<double, 2, cs::geographic<radian> > point_ll;
    typedef point<double, 2, cs::cartesian> point_xy;

    std::cout << std::setprecision(12);

    double d2r = math::d2r<double>();
    double r2d = math::r2d<double>();

    //std::string from = "+proj=longlat +datum=WGS84 +no_defs";
    //std::string to = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +no_defs";

    std::string from = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    std::string to = "+proj=longlat +ellps=airy +datum=OSGB36 +no_defs";

    {
        point_xy pt_xy(18.5 * d2r, 54.2 * d2r);
        point_xy pt_xy2(0, 0);

        transformation<> tr((proj4(from)), (proj4(to)));
        //transformation<> tr((epsg(4326)), (epsg(25832)));
        
        //std::cout << wkt(pt_xy) << std::endl;
        tr.forward(pt_xy, pt_xy2);
        //std::cout << wkt(pt_xy2) << std::endl;
        std::cout << get<0>(pt_xy2) * r2d << " " << get<1>(pt_xy2) * r2d << std::endl;
        //tr.inverse(pt_xy2, pt_xy);
        //std::cout << wkt(pt_xy) << std::endl;
    }

    /*{
        projPJ pj_from, pj_to;

        pj_from = pj_init_plus(from.c_str());
        pj_to = pj_init_plus(to.c_str());
        
        double x = get<0>(pt_xy);
        double y = get<1>(pt_xy);
        pj_transform(pj_from, pj_to, 1, 0, &x, &y, NULL );

        std::cout << x * r2d << " " << y * r2d << std::endl;
    }*/

    return 0;
}
