// Boost.Geometry

// Copyright (c) 2017, Ruoyun Jing, Northwest University, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP
#define BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/core/radius.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/formulas/flattening.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#define BOOST_GEOMETRY_EPS 1e-9

namespace bg = boost::geometry;
namespace boost { namespace geometry { namespace formula
{

template 
<
    typename CT,
    typename Geometry
>
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
        
        CT const lon1 = bg::get_as_radian<0>(p1);
        CT const lat1 = bg::get_as_radian<1>(p1);
        CT const lon2 = bg::get_as_radian<0>(p2);    
        CT const lat2 = bg::get_as_radian<1>(p2);    
        CT const lon4 = bg::get_as_radian<0>(p4);
        CT const lat4 = bg::get_as_radian<1>(p4);
       
        CT const distance_result1 = flat_distance(lon1, lat1, lon2, lat2, spheriod);
        CT const distance_result2 = flat_distance(lon1, lat1, lon4, lat4, spheriod);

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
private:
    static inline CT flat_distance(CT const& lo1,
                                   CT const& la1,
                                   CT const& lo2,
                                   CT const& la2,
                                   Geometry const& spher)
    {
        CT const c2 = CT(2);
        CT const spher_r = get_radius<0>(spher); 
        CT const spher_f = formula::flattening<CT, Geometry>(spher);
        CT const spher_e2 = spher_f * (c2 - spher_f); 
        
        CT const dlat = la2 - la1;
        CT const dlon = lo2 - lo1;
        
        CT const R1 = spher_r * (1 - spher_e2) /
                      bg::math::sqrt((1 - spher_e2 * bg::math::sqr(sin(la1)))
                              * (1 - spher_e2 * bg::math::sqr(sin(la1)))
                              * (1 - spher_e2 * bg::math::sqr(sin(la1))));
        CT const R2 = spher_r / bg::math::sqrt(1 - spher_e2 * bg::math::sqr(sin(la1)));
        
        CT const dis_North = R1 * dlat;
        CT const dis_East = R2 * cos(la1) * dlon;
    
        CT const distance_result = bg::math::sqrt(bg::math::sqr(dis_North) 
                                 + bg::math::sqr(dis_East));
        return distance_result;
    }
    
};

}}} // namespace boost::geometry::formula

#endif //BOOST_GEOMETRY_FORMULAS_FLAT_APPROXIMATION_HPP
