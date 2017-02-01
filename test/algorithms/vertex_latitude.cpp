// Boost.Geometry

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/formulas/vincenty_inverse.hpp>
#include <boost/geometry/formulas/vertex_latitude.hpp>

template <
        template <typename, bool, bool, bool, bool, bool> class Inverse,
        typename P,
        typename CT
>
void test_vertex_lat(P p1, P p2, CT expected_max, CT expected_min,
                     CT expected_max_sph, CT expected_min_sph, CT error = 0.0001)
{
    CT p10 = bg::get_as_radian<0>(p1),
       p11 = bg::get_as_radian<1>(p1),
       p20 = bg::get_as_radian<0>(p2),
       p21 = bg::get_as_radian<1>(p2);

    typename bg::formula::vertex_latitude<CT>::vertex_lat_result p_max
             = bg::formula::vertex_latitude<CT>::template geographic<Inverse>
               (p10, p11, p20, p21, bg::srs::spheroid<CT>());

    CT p_max_degree = p_max.north * 180 / bg::math::pi<CT>();
    BOOST_CHECK_CLOSE(p_max_degree, expected_max, error);

    CT p_min_degree = p_max.south * 180 / bg::math::pi<CT>();
    BOOST_CHECK_CLOSE(p_min_degree, expected_min, error);

    typename bg::formula::vertex_latitude<CT>::vertex_lat_result p_max_sph
             = bg::formula::vertex_latitude<CT>::template spherical
               (p10, p11, p20, p21);

    CT p_max_degree_sph = p_max_sph.north * 180 / bg::math::pi<CT>();
    BOOST_CHECK_CLOSE(p_max_degree_sph, expected_max_sph, error);

    CT p_min_degree_sph = p_max_sph.south * 180 / bg::math::pi<CT>();
    BOOST_CHECK_CLOSE(p_min_degree_sph, expected_min_sph, error);
}


template <typename CT>
void test_all()
{
    typedef bg::model::point<CT, 2, bg::cs::geographic<bg::degree> > Pg;

    // Short segments
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 1), Pg(10, 5), 5.0, 1.0, 5.0, 1.0);

    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 1), Pg(10, 1), 1.0031124506594733, 1.0, 1.0030915676477881, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(-5, 1), Pg(4, 1), 1.0031124506594733, 1.0, 1.0030915676477881, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(175, 1), Pg(184, 1), 1.0031124506594733, 1.0, 1.0030915676477881, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(355, 1), Pg(4, 1), 1.0031124506594733, 1.0, 1.0030915676477881, 1.0);

    // Reverse direction
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 2), Pg(70, 1), 2.0239716998355468, 1.0, 2.0228167431951536, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(70, 1), Pg(1, 2), 2.0239716998351849, 1.0, 2.022816743195063, 1.0);

    // Long segments
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(0, 1), Pg(170, 1), 11.975026023950877, 1.0, 11.325049479775814, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(0, 1), Pg(179, 1), 68.452669316418039, 1.0, 63.437566893227093, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(0, 1), Pg(179.5, 1), 78.84050225214871, 1.0, 75.96516822754981, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(0, 1), Pg(180.5, 1), 78.84050225214871, 1.0, 75.965168227550194, 1.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(0, 1), Pg(180, 1), 90.0, 1.0, 90.0, 1.0);

    // South hemisphere
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, -1), Pg(10, -5), -1.0, -5.0, -1.0, -5.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, -1), Pg(10, -1), -1.0, -1.0031124506594733, -1.0, -1.0030915676477881);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, -1), Pg(170, -1), -1.0, -10.85834257048573, -1.0, -10.321374780571153);

    // Different strategies for inverse
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 1), Pg(10, 1), 1.0031124506594733, 1.0,
             1.0030915676477881, 1.0, 0.00000001);
    test_vertex_lat<bg::formula::andoyer_inverse>
            (Pg(1, 1), Pg(10, 1), 1.0031124504591062, 1.0,
             1.0030915676477881, 1.0, 0.00000001);
    test_vertex_lat<bg::formula::vincenty_inverse>
            (Pg(1, 1), Pg(10, 1), 1.0031124508942098, 1.0,
             1.0030915676477881, 1.0, 0.00000001);

    // Meridian and equator
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 10), Pg(1, -10), 10.0, -10.0, 10.0, -10.0);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 0), Pg(10, 0), 0.0, 0.0, 0.0, 0.0);

    // One endpoint in northern hemisphere and the other in southern hemisphere
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(1, 1), Pg(150, -5), 1.0, -8.1825389632359933, 1.0, -8.0761230625567588);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(150, -5), Pg(1, 1), 1.0, -8.1825389632359933, 1.0, -8.0761230625568015);
    test_vertex_lat<bg::formula::thomas_inverse>
            (Pg(150, 5), Pg(1, -1), 8.1825389632359933, -1.0, 8.0761230625568015, -1.0);
}



int test_main( int , char* [] )
{
    test_all<double>();

    return 0;
}

