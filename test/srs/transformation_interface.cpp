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

template <typename T>
void test_geometries()
{
    using namespace boost::geometry;
    using namespace boost::geometry::model;
    using namespace boost::geometry::srs;

    typedef model::point<T, 2, cs::cartesian> point;
    typedef model::segment<point> segment;
    typedef model::linestring<point> linestring;
    typedef model::ring<point> ring;
    typedef model::polygon<point> polygon;
    typedef model::multi_point<point> mpoint;
    typedef model::multi_linestring<linestring> mlinestring;
    typedef model::multi_polygon<polygon> mpolygon;

    std::cout << std::setprecision(12);

    double d2r = math::d2r<T>();
    double r2d = math::r2d<T>();

    std::string from = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    std::string to = "+proj=longlat +ellps=airy +datum=OSGB36 +no_defs";

    {
        point pt(18.5 * d2r, 54.2 * d2r);
        point pt2(0, 0);
        segment seg(pt, pt);
        segment seg2;
        linestring ls; ls.push_back(pt);
        linestring ls2;
        ring rg; rg.push_back(pt);
        ring rg2;
        polygon poly; poly.outer() = rg;
        polygon poly2;
        mpoint mpt; mpt.push_back(pt);
        mpoint mpt2;
        mlinestring mls; mls.push_back(ls);
        mlinestring mls2;
        mpolygon mpoly; mpoly.push_back(poly);
        mpolygon mpoly2;

        transformation<> tr((proj4(from)), (proj4(to)));
        //transformation<> tr((epsg(4326)), (epsg(25832)));       

        tr.forward(pt, pt2);
        tr.forward(seg, seg2);
        tr.forward(ls, ls2);
        tr.forward(rg, rg2);
        tr.forward(poly, poly2);
        tr.forward(mpt, mpt2);
        tr.forward(mls, mls2);
        tr.forward(mpoly, mpoly2);

        std::cout << wkt(pt2) << std::endl;
        std::cout << wkt(seg2) << std::endl;
        std::cout << wkt(ls2) << std::endl;
        std::cout << wkt(rg2) << std::endl;
        std::cout << wkt(poly2) << std::endl;
        std::cout << wkt(mpt2) << std::endl;
        std::cout << wkt(mls2) << std::endl;
        std::cout << wkt(mpoly2) << std::endl;
        
        tr.inverse(pt2, pt);
        tr.inverse(seg2, seg);
        tr.inverse(ls2, ls);
        tr.inverse(rg2, rg);
        tr.inverse(poly2, poly);
        tr.inverse(mpt2, mpt);
        tr.inverse(mls2, mls);
        tr.inverse(mpoly2, mpoly);

        std::cout << wkt(pt) << std::endl;
        std::cout << wkt(seg) << std::endl;
        std::cout << wkt(ls) << std::endl;
        std::cout << wkt(rg) << std::endl;
        std::cout << wkt(poly) << std::endl;
        std::cout << wkt(mpt) << std::endl;
        std::cout << wkt(mls) << std::endl;
        std::cout << wkt(mpoly) << std::endl;
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
}

template <typename P1, typename P2, typename Tr>
inline void test_combination(Tr const& tr, double lon, double lat)
{
    using namespace boost::geometry;

    P1 pt(lon, lat);
    P2 pt2;

    tr.forward(pt, pt2);

    std::cout << wkt(pt) << " -> " << wkt(pt2) << std::endl;
}

void test_combinations(std::string const& from, std::string const& to)
{
    using namespace boost::geometry;
    using namespace boost::geometry::model;
    using namespace boost::geometry::srs;

    typedef model::point<double, 2, cs::cartesian> xy;
    typedef model::point<double, 2, cs::geographic<degree> > ll_d;
    typedef model::point<double, 2, cs::geographic<radian> > ll_r;
    typedef model::point<double, 3, cs::cartesian> xyz;
    typedef model::point<double, 3, cs::geographic<degree> > llz_d;
    typedef model::point<double, 3, cs::geographic<radian> > llz_r;

    std::cout << std::setprecision(12);

    double d2r = math::d2r<double>();
    double r2d = math::r2d<double>();

    transformation<> tr((proj4(from)), (proj4(to)));

    double lon_d = 18.5;
    double lat_d = 54.2;
    double lon_r = lon_d * d2r;
    double lat_r = lat_d * d2r;

    test_combination<xy, xy>(tr, lon_r, lat_r);
    test_combination<xy, ll_r>(tr, lon_r, lat_r);
    test_combination<xy, ll_d>(tr, lon_r, lat_r);
    test_combination<ll_r, xy>(tr, lon_r, lat_r);
    test_combination<ll_r, ll_r>(tr, lon_r, lat_r);
    test_combination<ll_r, ll_d>(tr, lon_r, lat_r);
    test_combination<ll_d, xy>(tr, lon_d, lat_d);
    test_combination<ll_d, ll_r>(tr, lon_d, lat_d);
    test_combination<ll_d, ll_d>(tr, lon_d, lat_d);
}

int test_main(int, char*[])
{
    test_geometries<double>();
    test_geometries<float>();
    
    test_combinations("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                      "+proj=longlat +ellps=airy +datum=OSGB36 +no_defs");
    test_combinations("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
                      "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +no_defs");

    return 0;
}
