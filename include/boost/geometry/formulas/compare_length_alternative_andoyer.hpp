// Boost.Geometry

// Copyright (c) 2017, Ruoyun Jing, Northwest University, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_SECOND_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_SECOND_HPP

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
class compare_length_andoyer_second
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
    
        CT const distance_result1 = andoyer_distance_second(lon1, lat1, lon2, lat2, spheriod);
        CT const distance_result2 = andoyer_distance_second(lon3, lat3, lon4, lat4, spheriod);

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
    static inline CT andoyer_distance_second(CT const& lo1,
                                             CT const& la1,
                                             CT const& lo2,
                                             CT const& la2,
                                             Geometry const& spher)
    {
        CT const c0 = CT(0);
        CT const c1 = CT(1);
        CT const c2 = CT(2);
        CT const c3 = CT(3);
        if ( math::equals(lo1, lo2) && math::equals(la1, la2) )
        {
            return c0;
        }

        CT const a = get_radius<0>(spher);
        CT const pi = math::pi<CT>();
        CT const f = formula::flattening<CT, Geometry>(spher);

        CT const F = (la1 + la2) / c2;
        CT const G = (la1 - la2) / c2;
        CT const L = (lo1 - lo2) / c2;

        CT const sin_g = sin(G);
        CT const sin_l = sin(L);
        CT const sin_f = sin(F);
        CT const cos_g = cos(G);
        CT const cos_l = cos(L);
        CT const cos_f = cos(F);

        CT const S = math::sqr(sin_g) * math::sqr(cos_l) 
                   + math::sqr(cos_f) * math::sqr(sin_l);
        CT const C = math::sqr(cos_g) * math::sqr(cos_l) 
                   + math::sqr(sin_f) * math::sqr(sin_l);
        CT const W = atan(math::sqrt(S / C));
        CT const R = sqrt(S * C) / W;
        CT const D = c2 * W * a;
        CT const H_1 = (c3 * R - c1) / (c2 * C);
        CT const H_2 = (c3 * R + c1) / (c2 * S);

        return D * (c1 + f * H_1 * math::sqr(sin_f) * math::sqr(cos_g) 
                    - f * H_2 * math::sqr(cos_f) * math::sqr(sin_g));
    }
};

}}} // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP
