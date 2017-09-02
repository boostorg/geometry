// Boost.Geometry

// Copyright (c) 2017, Ruoyun Jing, Northwest University, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_DISTANCE_HAVERSINE_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_DISTANCE_HAVERSINE_HPP

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
class compare_length_haversine
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
                                    T const& p3,
                                    T const& p4,
                                    Spheriod const& spheriod)
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

        CT const distance_result1 = bg::math::hav(lat2 - lat1)
                 + cos(lat1) * cos(lat2) * bg::math::hav(lon2 - lon1);
        CT const distance_result2 = bg::math::hav(lat4 - lat3)
                 + cos(lat3) * cos(lat4) * bg::math::hav(lon4 - lon3);
    
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

#endif //BOOST_GEOMETRY_FORMULAS_COMPARE_DISTANCE_HAVERSINE_HPP
