// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef PERIMETER_POLYGON_CASES_HPP
#define PERIMETER_POLYGON_CASES_HPP

#include <string>

static std::string Poly_data_geo_1[] = {
    "POLYGON((0 90,1 80,1 70))",
    "POLYGON((0 90,1 80,1 80,1 80,1 70,1 70))",
    "POLYGON((0 90,1 80,1 79,1 78,1 77,1 76,1 75,1 74,\
              1 73,1 72,1 71,1 70))"
};

std::vector<std::string> Poly_data_geo(Poly_data_geo_1, Poly_data_geo_1 +
                             sizeof(Poly_data_geo_1) / sizeof(std::string));


static std::string Poly_data_sph_1[] = {
    "POLYGON((0 0,180 0,0 0))",
    "POLYGON((0 0,180 0,180 0,180 0,180 180,0 0))",
    "POLYGON((0 0,180 0,180 10,180 20,180 30,180 40,180 50,180 60,\
              180 70,180 80,180 90,180 100,180 110,180 120,180 130,\
              180 140,180 150,180 160,180 170,180 180,0 0))"
};

std::vector<std::string> Poly_data_sph(Poly_data_sph_1, Poly_data_sph_1 +
                              sizeof(Poly_data_sph_1) / sizeof(std::string));


static std::string Multipoly_data_1[] = {
    "MULTIPOLYGON(((0 0,180 0,0 0)), ((0 0, 0 90, 90 90, 0 0)))"
};

std::vector<std::string> Multipoly_data(Multipoly_data_1, Multipoly_data_1 +
                               sizeof(Multipoly_data_1) / sizeof(std::string));

#endif // PERIMETER_POLYGON_CASES_HPP
