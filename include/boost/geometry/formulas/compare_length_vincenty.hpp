// Boost.Geometry

// Copyright (c) 2017, Ruoyun Jing, Northwest University, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_VINCENTY_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_VINCENTY_HPP

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/algorithms/comparable_geographic_distance.hpp>

#define BOOST_GEOMETRY_EPS 1e-9

namespace bg = boost::geometry;
namespace boost { namespace geometry { namespace formula
{

template 
<
    typename CT
>
class compare_length_vincenty 
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
                                    Spheriod const& spher)
    {
        result_type result;
        CT const distance_result1 = bg::distance
                           (p1, p2, 
                            bg::strategy::distance::vincenty<Spheriod >(spher));
        CT const distance_result2 = bg::distance
                           (p3, p4, 
                            bg::strategy::distance::vincenty<Spheriod >(spher));

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

#endif // BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_VINCENTY_HPP
