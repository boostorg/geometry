// Boost.Geometry

// Copyright (c) 2017, Ruoyun Jing, Northwest University, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP

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
    typename CT
>
class compare_length_andoyer 
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
    
        CT const distance_result1 = andoyer_distance(lon1, lat1, lon2, lat2, spheriod);
        CT const distance_result2 = andoyer_distance(lon3, lat3, lon4, lat4, spheriod);

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
    static inline CT andoyer_distance(CT const& lo1,
                                      CT const& la1,
                                      CT const& lo2,
                                      CT const& la2,
                                      Geometry const& spher)
    {
        CT const c0 = CT(0);
        
        if ( math::equals(lo1, lo2) && math::equals(la1, la2) )
        {
            return c0;
        }

        CT const c1 = CT(1);
        CT const pi = math::pi<CT>();
        CT const f = formula::flattening<CT, Geometry>(spher);

        CT const dlon = lo2 - lo1;
        CT const sin_dlon = sin(dlon);
        CT const cos_dlon = cos(dlon);
        CT const sin_lat1 = sin(la1);
        CT const cos_lat1 = cos(la1);
        CT const sin_lat2 = sin(la2);
        CT const cos_lat2 = cos(la2);

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

        CT const a = get_radius<0>(spher);

        return a * (d + dd);
    }
};

}}} // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP
