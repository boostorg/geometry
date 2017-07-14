// Boost.Geometry

// Copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP
#define BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#define BOOST_GEOMETRY_EPS 1e-9

namespace bg = boost::geometry;
namespace boost { namespace geometry { namespace formula
{

template <typename CT>
class flat_approximation
{
    
public:
    typedef int result_type;
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
        
        CT const c2 = CT(2);
        CT const earth_r = CT(6317.0);
        CT const earth_f = formula::flattening<CT>(spheriod);
        CT const earth_e2 = earth_f * (c2 - earth_f); 

        CT const lon1 = bg::get_as_radian<0>(p1);
        CT const lat1 = bg::get_as_radian<1>(p1);
        CT const lon2 = bg::get_as_radian<0>(p2);    
        CT const lat2 = bg::get_as_radian<1>(p2);    
        CT const lon4 = bg::get_as_radian<0>(p4);
        CT const lat4 = bg::get_as_radian<1>(p4);
        
        CT dlat = lat2 - lat1;
        CT dlon = lon2 - lon1;
        
        CT R1 = earth_r * (1 - earth_e2) /
                      bg::math::sqrt((1 - earth_e2 * bg::math::sqr(sin(lat1)))
                              * (1 - earth_e2 * bg::math::sqr(sin(lat1)))
                              * (1 - earth_e2 * bg::math::sqr(sin(lat1))));
        CT R2 = earth_r / bg::math::sqrt(1 - earth_e2 * bg::math::sqr(sin(lat1)));
        
        CT dis_North = R1 * dlat;
        CT dis_East = R2 * cos(lat1) * dlon;
    
        CT const distance_result1 = bg::math::sqrt(bg::math::sqr(dis_North) + bg::math::sqr(dis_East));
        
        dlat = lat4 - lat1;
        dlon = lon4 - lon1;
       
        R1 = earth_r * (1 - earth_e2) / 
             bg::math::sqrt((1 - earth_e2 * bg::math::sqr(sin(lat1)))
                     * (1 - earth_e2 * bg::math::sqr(sin(lat1)))
                     * (1 - earth_e2 * bg::math::sqr(sin(lat1))));
        R2 = earth_r / bg::math::sqrt(1 - earth_e2 * bg::math::sqr(sin(lat1)));
       
        dis_North = R1 * dlat;
        dis_East = R2 * cos(lat1) * dlon;
        
        CT const distance_result2 = bg::math::sqrt(bg::math::sqr(dis_North) + bg::math::sqr(dis_East));
   
        CT const sub = distance_result1 - distance_result2;
    
        if (sub < -BOOST_GEOMETRY_EPS)
        {
            result = 1;
        }
        else if (sub > BOOST_GEOMETRY_EPS)
        {
            result = 2;
        }
        else if (fabs(sub) < BOOST_GEOMETRY_EPS)
        {
            result = 3;
        }  
        return result; 
    }
};

}}} // namespace boost::geometry::formula

#endif //BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP
