// Boost.Geometry

// Copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_CARTESINE_DISTANCE_HPP
#define BOOST_GEOMETRY_FORMULAS_CARTESINE_DISTANCE_HPP

#include <boost/geometry/util/conditon.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/formulas/result_compare_distance.hpp>

#define BOOST_GEOMETRY_EPS 1e-9
namespace boost { namespace geometry { namespace formula
{

template <typename CT>
class cartesine_distance
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
 
        distance_result1 = bg::distance
            (p1, p2, bg::strategy::distance::pythagoras<void>());
        distance_result2 = bg::distance
            (p3, p4, bg::strategy::distance::pythagoras<void>());
 
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
        

#endif //BOOST_GEOMETRY_FORMULAS_CARTESINE_DISTANCE_HPP
