// Boost.Geometry

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_INVERSE_DIFFERENTIAL_QUANTITIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_INVERSE_DIFFERENTIAL_QUANTITIES_HPP


#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry { namespace detail
{

/*!
\brief The solution of a part of the inverse problem - differential quantities.
\author See
- Charles F.F Karney, Algorithms for geodesics, 2011
https://arxiv.org/pdf/1109.4448.pdf
*/
template <
    typename CT,
    bool EnableReducedLength,
    bool EnableGeodesicScale
>
class inverse_differential_quantities
{
public:
    template <typename J12Calc>
    static inline void apply(CT const& dlon,
                             CT const& sin_lat1, CT const& cos_lat1,
                             CT const& sin_lat2, CT const& cos_lat2,
                             CT const& azimuth, CT const& reverse_azimuth,
                             CT const& b, CT const& f,
                             CT & reduced_length, CT & geodesic_scale,
                             J12Calc J12_calc)
    {
        CT one_minus_f = 1 - f;
        CT sin_bet1 = one_minus_f * sin_lat1;
        CT sin_bet2 = one_minus_f * sin_lat2;
            
        // equator
        if (math::equals(sin_bet1, CT(0)) && math::equals(sin_bet2, CT(0)))
        {
            CT sig12 = math::abs(dlon) / one_minus_f;
            if (BOOST_GEOMETRY_CONDITION(EnableReducedLength))
            {
                CT m12 = sin(sig12) * b;
                reduced_length = m12;
            }
                
            if (BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
            {
                CT M12 = cos(sig12);
                geodesic_scale = M12;
            }
        }
        else
        {
            CT cos_bet1 = cos_lat1;
            CT cos_bet2 = cos_lat2;

            CT sin_alp1 = sin(azimuth);
            CT cos_alp1 = cos(azimuth);
            CT sin_alp2 = sin(reverse_azimuth);
            CT cos_alp2 = cos(reverse_azimuth);

            CT sin_sig1 = sin_bet1;
            CT cos_sig1 = cos_alp1 * cos_bet1;
            CT sin_sig2 = sin_bet2;
            CT cos_sig2 = cos_alp2 * cos_bet2;

            normalize(sin_sig1, cos_sig1);
            normalize(sin_sig2, cos_sig2);

            CT sin_alp0 = sin_alp1 * cos_bet1;
            CT cos_alp0_sqr = 1 - math::sqr(sin_alp0);

            CT const J12 = J12_calc(sin_sig1, cos_sig1, sin_sig2, cos_sig2, cos_alp0_sqr, f);

            CT const e2 = f * (2 - f);
            CT const dn1 = math::sqrt(1 + e2 * math::sqr(sin_lat1));
            CT const dn2 = math::sqrt(1 + e2 * math::sqr(sin_lat2));

            if (BOOST_GEOMETRY_CONDITION(EnableReducedLength))
            {
                CT const m12_b = dn2 * (cos_sig1 * sin_sig2)
                               - dn1 * (sin_sig1 * cos_sig2)
                               - cos_sig1 * cos_sig2 * J12;
                CT const m12 = m12_b * b;

                reduced_length = m12;
            }

            if (BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
            {
                CT const ep2 = e2 / math::sqr(one_minus_f);
                CT const cos_sig12 = cos_sig1 * cos_sig2 + sin_sig1 * sin_sig2;
                CT const t = ep2 * (cos_bet1 - cos_bet2) * (cos_bet1 + cos_bet2) / (dn1 + dn2);
                CT const M12 = cos_sig12 + (t * sin_sig2 - cos_sig2 * J12) * sin_sig1 / dn1;

                geodesic_scale = M12;
            }
        }
    }

    static inline CT J12_calc_f1(CT const& sin_sig1, CT const& cos_sig1,
                                 CT const& sin_sig2, CT const& cos_sig2,
                                 CT const& cos_alp0_sqr, CT const& f)
    {
        CT const sig12 = atan2(cos_sig1 * sin_sig2 - sin_sig1 * cos_sig2,
                               cos_sig1 * cos_sig2 + sin_sig1 * sin_sig2);
        CT const sin_2sig1 = 2 * cos_sig1 * sin_sig1; // sin(2sig1)
        CT const sin_2sig2 = 2 * cos_sig2 * sin_sig2; // sin(2sig2)
        CT const sin_2sig12 = sin_2sig2 - sin_2sig1;
        
        CT const J12 = (sig12 - sin_2sig12 / 2) * cos_alp0_sqr * f;

        return J12;
    }

    static inline CT J12_calc_f2(CT const& sin_sig1, CT const& cos_sig1,
                                 CT const& sin_sig2, CT const& cos_sig2,
                                 CT const& cos_alp0_sqr, CT const& f)
    {
        CT const sig12 = atan2(cos_sig1 * sin_sig2 - sin_sig1 * cos_sig2,
                               cos_sig1 * cos_sig2 + sin_sig1 * sin_sig2);
        CT const sin_2sig1 = 2 * cos_sig1 * sin_sig1; // sin(2sig1)
        CT const sin_2sig2 = 2 * cos_sig2 * sin_sig2; // sin(2sig2)
        CT const sin_2sig12 = sin_2sig2 - sin_2sig1;
        CT const sin_4sig1 = 2 * sin_2sig1 * (math::sqr(cos_sig1) - math::sqr(sin_sig1)); // sin(4sig1)
        CT const sin_4sig2 = 2 * sin_2sig2 * (math::sqr(cos_sig2) - math::sqr(sin_sig2)); // sin(4sig2)
        CT const sin_4sig12 = sin_4sig2 - sin_4sig1;
        
        CT const t1 = 2 * sig12 - sin_2sig12;
        CT const t2 = cos_alp0_sqr * sin_4sig12
                   + (-8 * cos_alp0_sqr + 12) * sin_2sig12
                   + (12 * cos_alp0_sqr - 24) * sig12;
        CT const J12 = cos_alp0_sqr * f * (t1 / 2 - f * t2 / 16);

        return J12;
    }

private:
    static inline void normalize(CT & x, CT & y)
    {
        CT const l = math::sqrt(math::sqr(x) + math::sqr(y));
        x /= l;
        y /= l;
    }
};

}}} // namespace boost::geometry::detail


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_INVERSE_DIFFERENTIAL_QUANTITIES_HPP
