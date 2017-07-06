// Boost.Geometry

// Copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP

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
class compare_length_andoyer 
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
                                    T const& p3.
                                    T const& p4,
                                    Spheriod const& spheriod)
    {
        result_type result;
        CT distance_result1;
        CT lon1, lat1, lon2, lat2, lon3, lat3, lon4, lat4;

        lon1 = bg::get_as_radian<0>(p1);
        lat1 = bg::get_as_radian<1>(p1);
        lon2 = bg::get_as_radian<0>(p2);    
        lat2 = bg::get_as_radian<1>(p2);
        lon3 = bg::get_as_radian<0>(p3);
        lat3 = bg::get_as_radian<1>(p3);    
        lon4 = bg::get_as_radian<0>(p4);
        lat4 = bg::get_as_radian<1>(p4);
    
        distance_result1 = andoyer_distance(lon1, lat1, lon2, lat2);
        distance_result2 = andoyer_distance(lon3, lat3, lon4, lat4);

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
    static inline CT andoyer_distance(CT const& lon1,
                                      CT const& lat1,
                                      CT const& lon2,
                                      CT const& lat2)
    {
        CT const c0 = CT(0);
        
        if ( math::equals(lon1, lon2) && math::equals(lat1, lat2) )
        {
            return c0;
        }

        CT const c1 = CT(1);
        CT const pi = math::pi<CT>();
        CT const f = formula::flattening<CT>(spheroid);

        CT const dlon = lon2 - lon1;
        CT const sin_dlon = sin(dlon);
        CT const cos_dlon = cos(dlon);
        CT const sin_lat1 = sin(lat1);
        CT const cos_lat1 = cos(lat1);
        CT const sin_lat2 = sin(lat2);
        CT const cos_lat2 = cos(lat2);

        // H,G,T = infinity if cos_d = 1 or cos_d = -1
        // lat1 == +-90 && lat2 == +-90
        // lat1 == lat2 && lon1 == lon2
        CT cos_d = sin_lat1*sin_lat2 + cos_lat1*cos_lat2*cos_dlon;
        // on some platforms cos_d may be outside valid range
        if (cos_d < -c1)
            cos_d = -c1;
        else if (cos_d > c1)
            cos_d = c1;

        CT const d = acos(cos_d); // [0, pi]
        CT const sin_d = sin(d);  // [-1, 1]

        CT const K = math::sqr(sin_lat1-sin_lat2);
        CT const L = math::sqr(sin_lat1+sin_lat2);
        CT const three_sin_d = CT(3) * sin_d;

        CT const one_minus_cos_d = c1 - cos_d;
        CT const one_plus_cos_d = c1 + cos_d;
        // cos_d = 1 or cos_d = -1 means that the points are antipodal

        CT const H = math::equals(one_minus_cos_d, c0) ?
                        c0 :
                        (d + three_sin_d) / one_minus_cos_d;
        CT const G = math::equals(one_plus_cos_d, c0) ?
                        c0 :
                        (d - three_sin_d) / one_plus_cos_d;

        CT const dd = -(f/CT(4))*(H*K+G*L);

        CT const a = get_radius<0>(spheroid);

        return a * (d + dd);
    }
};

}}} // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP
