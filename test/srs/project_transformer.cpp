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
#include <boost/geometry/strategies/transform/project_transformer.hpp>

#include <boost/geometry/io/wkt/read.hpp>


int test_main(int, char*[])
{
    using namespace boost::geometry;
    using namespace boost::geometry::model;
    using namespace boost::geometry::srs;
    using namespace bg::strategy::transform;

    typedef point<double, 2, cs::geographic<degree> > point_ll;
    typedef point<double, 2, cs::cartesian> point_xy;
    typedef polygon<point_ll> polygon_ll;
    typedef polygon<point_xy> polygon_xy;

    std::cout << std::setprecision(12);

    {
        point_ll pt_ll(1, 1);
        point_xy pt_xy(0, 0);

        project_transformer<> strategy = proj4("+proj=tmerc +ellps=WGS84 +units=m");
        project_forward_transformer<> strategy_f = proj4("+proj=tmerc +ellps=WGS84 +units=m");
        project_inverse_transformer<> strategy_i = proj4("+proj=tmerc +ellps=WGS84 +units=m");

        std::cout << wkt(pt_ll) << std::endl;
        
        bg::transform(pt_ll, pt_xy, strategy);
        std::cout << wkt(pt_xy) << std::endl;

        bg::transform(pt_ll, pt_xy, strategy_f);
        std::cout << wkt(pt_xy) << std::endl;
        
        bg::transform(pt_xy, pt_ll, strategy);
        std::cout << wkt(pt_ll) << std::endl;

        bg::transform(pt_xy, pt_ll, strategy_i);
        std::cout << wkt(pt_ll) << std::endl;
    }

    {
        polygon_ll poly_ll;
        bg::read_wkt("POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))", poly_ll);

        polygon_xy poly_xy;
        polygon_ll poly_ll2;
        
        project_transformer<> strategy = proj4("+proj=tmerc +ellps=WGS84 +units=m");

        std::cout << wkt(poly_ll) << std::endl;
        
        bg::transform(poly_ll, poly_xy, strategy);
        std::cout << wkt(poly_xy) << std::endl;

        bg::transform(poly_xy, poly_ll2, strategy);
        std::cout << wkt(poly_ll2) << std::endl;
    }

    {
        project_transformer<> strategy = epsg(2000);
        project_forward_transformer<> strategy_f = epsg(2000);
        project_inverse_transformer<> strategy_i = epsg(2000);
    }

    {
        project_transformer<static_proj4<proj::tmerc, ellps::WGS84> > strategy;
        project_forward_transformer<static_proj4<proj::tmerc, ellps::WGS84> > strategy_f;
        project_inverse_transformer<static_proj4<proj::tmerc, ellps::WGS84> > strategy_i;
    }

    {
        project_transformer<static_epsg<2000> > strategy;
        project_forward_transformer<static_epsg<2000> > strategy_f;
        project_inverse_transformer<static_epsg<2000> > strategy_i;
    }

    return 0;
}
