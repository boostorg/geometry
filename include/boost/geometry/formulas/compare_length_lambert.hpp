// Boost.Geometry

// Copyright (c) 2017, Ruoyun Jing, Northwest University, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_LAMBERT_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_LAMBERT_HPP

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/core/radius.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/formulas/flattening.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#define BOOST_GEOMETRY_EPS 1e-9

namespace boost { namespace geometry { namespace formula
{

template
<
    typename CT
>
class compare_length_lambert
{

public:
    typedef int result_type;
    template
    <
        typename T,
        typename Geometry
    >
    static inline result_type apply(T const& p1,
                                   T const& p2,
                                   T const& p3,
                                   T const& p4,
                                   Geometry const& spheriod)
    {
        result_type result;

        CT const lon1 = bg::get_as_radian<0>(p1);
        CT const lat1 = bg::get_as_radian<1>(p1);
        CT const lon2 = bg::get_as_radian<0>(p2);    
        CT const lat2 = bg::get_as_radian<1>(p2);    
        CT const lon3 = bg::get_as_radian<0>(p3);
        CT const lat3 = bg::get_as_radian<1>(p3);
        CT const lon4 = bg::get_as_radian<0>(p4);
        CT const lat4 = bg::get_as_radian<1>(p4);
    
        CT const distance_result1 = lambert_distance(lon1, lat1, lon2, lat2, spheriod);
        CT const distance_result2 = lambert_distance(lon3, lat3, lon4, lat4, spheriod);

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
    template<typename Geometry>
    static inline CT lambert_distance(CT const& lo1,
                                      CT const& la1,
                                      CT const& lo2,
                                      CT const& la2,
                                      Geometry const& spher)
    {   
        CT const c4 = CT(4);
        CT const c1 = CT(1);
        CT const spher_f = formula::flattening<CT, Geometry>(spher);
        CT const spher_a = get_radius<0>(spher);
        
        CT const sin_lat1 = sin(la1);
        CT const cos_lat1 = cos(la1);
        CT const sin_lat2 = sin(la2);
        CT const cos_lat2 = cos(la2);

        CT const phi_1 = atan((c1 - spher_f) * tan(la1));
        CT const phi_2 = atan((c1 - spher_f) * tan(la2));
        CT const P = math::sqr(sin_lat1 + sin_lat2);
        CT const Q = math::sqr(sin_lat1 - sin_lat2);
        
        CT const theta = acos(sin_lat1 * sin_lat2 +
                              cos_lat1 * cos_lat2 * cos(lo1 - lo2));
        CT const X = (theta - sin(theta)) / 
                     c4 * (c1 + cos(theta));
        CT const Y = (theta + sin(theta)) / 
                     c4 * (c1 - cos(theta));

        return spher_a * (theta - spher_f * (P * X + Q * Y));
    }
};

}}} // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_LAMBERT_HPP
