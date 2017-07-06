// Boost.Geometry

// Copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP
#define BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP

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
class flat_approximation
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
                                    T const& p4,
                                    Spheriod const& spheriod)
    {
        result_type result;
        
        CT distance_result1;
        CT distance_result2;
        CT lon1, lat1, lon2, lat2, lon4, lat4;
        CT dlat, dlon, R1, R2, dis_North, dis_East;

        lon1 = bg::get_as_radian<0>(p1);
        lat1 = bg::get_as_radian<1>(p1);
        lon2 = bg::get_as_radian<0>(p2);    
        lat2 = bg::get_as_radian<1>(p2);    
        lon4 = bg::get_as_radian<0>(p4);
        lat4 = bg::get_as_radian<1>(p4);
    
        dlat = lat2 - lat1;
        dlon = lon2 - lon1;
    
        R1 = earth_r * (1 - earth_e2) /
             bg::math::sqrt((1 - earth_e2 * bg::math::sqr(sin(lat1)))
                          * (1 - earth_e2 * bg::math::sqr(sin(lat1)))
                          * (1 - earth_e2 * bg::math::sqr(sin(lat1))));
        R2 = earth_r / bg::math::sqrt(1 - earth_e2 * bg::math::sqr(sin(lat1)));
    
        dis_North = R1 * dlat;
        dis_East = R2 * cos(lat1) * dlon;
    
        distance_result1 = bg::math::sqrt(bg::math::sqr(dis_North) + bg::math::sqr(dis_East));
    
        dlat = lat2 - lat1;
        dlon = lon2 - lon1;
    
        R1 = earth_r * (1 - earth_e2) / 
             bg::math::sqrt((1 - earth_e2 * bg::math::sqr(sin(lat1)))
                          * (1 - earth_e2 * bg::math::sqr(sin(lat1)))
                          * (1 - earth_e2 * bg::math::sqr(sin(lat1))));
        R2 = earth_r / bg::math::sqrt(1 - earth_e2 * bg::math::sqr(sin(lat1)));
    
        dis_North = R1 * dlat;
        dis_East = R2 * cos(lat1) * dlon;
    
        distance_result2 = bg::math::sqrt(bg::math::sqr(dis_North) + bg::math::sqr(dis_East));
        CT sub = distance_result1 - distance_result2;
    
        if (sub < -BOOST_GEOMETRY_EPS)
        {
            result.value = 1;
        }
        else if (sub > BOOST_GEOMETRY_EPS)
        {
            result.value = 2;
        }
        else if (fabs(sub) < BOOST_GEOMETRY_EPS)
        {
            result.value = 3;
        }   
    }
private:
    double earth_f = 1 / 298.257223563;
    double earth_e2 = earth_f * (2 - earth_f);
    double earth_r = 6317.0;

};

}}} // namespace boost::geometry::formula

#endif //BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP
