// Boost.Geometry

// Copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_DISTANCE_HAVERSINE_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_DISTANCE_HAVERSINE_HPP

#include <boost/geometry/core/radian.hpp>
#include <boost/geometru/core/access.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/conditon.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/formulas/result_compare_distance.hpp>

#define BOOST_GEOMETRY_EPS 1e-9
namespace boost { namespace geometry { namespace formula
{

template <typename CT>
class compare_length_haversine
{
    
public:
    typedef result_compare_distance<CT> result_type;
    template 
    <
        typename T,
        typename Spheriod
    >
    static inline result_type apply(T const& p1,
                                    T const& p2,
                                    T const& p3,
                                    T const& p4,
                                    Spheriod const& spheriod)
    {
        result_type result;

        T distance_result1;
        T distance_result2;
        T lon1, lat1, lon2, lat2, lon3, lat3, lon4, lat4;

        lon1 = bg::get_as_radian<0>(p1);
        lat1 = bg::get_as_radian<1>(p1);
        lon2 = bg::get_as_radian<0>(p2);    
        lat2 = bg::get_as_radian<1>(p2);    
        lon3 = bg::get_as_radian<0>(p3);
        lat3 = bg::get_as_radian<1>(p3);
        lon4 = bg::get_as_radian<0>(p4);
        lat4 = bg::get_as_radian<1>(p4);

        distance_result1 = bg::math::hav(lat2 - lat1)
            + cos(lat1) * cos(lat2) * bg::math::hav(lon2 - lon1);
        distance_result2 = bg::math::hav(lat4 - lat3)
            + cos(lat3) * cos(lat4) * bg::math::hav(lon4 - lon3);
    
        T sub = distance_result1 - distance_result2;
    
        if (sub < -EPS)
        {
            result.value = 1;
        }
        else if (sub > EPS)
        {
            result.value = 2;
        }
        else if (fabs(sub) < EPS)
        {
            result.value = 3;
        }   
    }
}
#endif //BOOST_GEOMETRY_FORMULAS_COMPARE_DISTANCE_HAVERSINE_HPP
