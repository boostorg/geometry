// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_SERIES_EXPANSION_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_SERIES_EXPANSION_DIRECT_HPP

/*
 * #include <boost/math/constants/constants.hpp>
#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/formulas/differential_quantities.hpp>
#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/result_direct.hpp>
*/

namespace boost { namespace geometry { namespace formula
{

/*!
\brief The solution of the direct problem of geodesics on latlong coordinates.
       Series approximation by [Bessel 1825] and [Helmert 1880]; here we follow
       presentation of results from [Karney - Algorithm for geodesics, 2011]
*/
template <
    typename CT,
    bool EnableCoordinates = true,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false,
    unsigned int const Order = 1
>
class series_expansion_direct
{
    static const bool CalcQuantities = EnableReducedLength || EnableGeodesicScale;
    static const bool CalcCoordinates = EnableCoordinates || CalcQuantities;
    static const bool CalcRevAzimuth = EnableReverseAzimuth || CalcQuantities;

public:
    typedef result_direct<CT> result_type;

    template <typename T, typename Dist, typename Azi, typename Spheroid>
    static inline result_type apply(T const& lo1,
                                    T const& la1,
                                    Dist const& distance,
                                    Azi const& azimuth12,
                                    Spheroid const& spheroid)
    {
        result_type result;

        CT const lon1 = lo1;
        CT const lat1 = la1;

        if ( math::equals(distance, Dist(0)) || distance < Dist(0) )
        {
            result.lon2 = lon1;
            result.lat2 = lat1;
            return result;
        }

        CT const f = formula::flattening<CT>(spheroid);

        CT const bet1 = atan((1-f) * tan(lat1));
        CT const sin_alp1 = sin(azimuth12);
        CT const sin_bet1 = sin(bet1);
        CT const cos_alp1 = cos(azimuth12);
        CT const cos_bet1 = cos(bet1);

        CT const norm = math::sqrt(cos_alp1 * cos_alp1 + sin_alp1 * sin_alp1 *
                                                   sin_bet1 * sin_bet1);
        CT const alp0 = atan2(sin_alp1 * cos_bet1, norm);
        CT const sig1 = atan2(sin_bet1, cos_alp1 * cos_bet1);
        CT const sin_alp0 = sin(alp0);
        CT const cos_alp0 = cos(alp0);

        // Compute lat
        CT const e2 = f * (CT(2) - f);
        CT const ep2 = e2 / (CT(1) - e2);
        CT const ep = math::sqrt(ep2);
        CT const k = cos_alp0 * ep;
        CT const k2 = k * k;
        CT const sqrt_k2_plus_one = math::sqrt(CT(1) + k2);
        CT const eps = (sqrt_k2_plus_one - 1) / (sqrt_k2_plus_one + 1);

        CT const sin_sig1 = sin(sig1);
        CT const cos_sig1 = cos(sig1);

        CT const A1 = evaluate_coeffs_A1_I1(eps);
        CT c[get_order()];
        evaluate_coeffs_C_I1(eps,c);
        CT const I1 = A1 * (sig1 + clenshaw_sum(sin_sig1, cos_sig1, c, c+get_order()));

        //Alternative evaluation
        //return_result<CT> res = compute_A1_I1(eps, sig1);
        //CT const A1 = res.A1;
        //CT const I1 = res.I1;

        CT const b = get_radius<2>(spheroid);
        CT const s1 = I1 * b;
        CT const s2 = s1 + distance;
        CT const tau2 = s2 / (b * A1);

        CT cp[get_order()];
        evaluate_coeffs_Cp(eps, cp);
        CT const sig2 = tau2 + clenshaw_sum(sin(tau2), cos(tau2), cp, cp+get_order());

        //
        CT const cos_sig2 = cos(sig2);

        if (BOOST_GEOMETRY_CONDITION(CalcRevAzimuth))
        {
            CT const alp2 = atan2(sin_alp0, cos_alp0 * cos_sig2);
            result.reverse_azimuth = alp2;
        }

        if (BOOST_GEOMETRY_CONDITION(CalcCoordinates))
        {
            CT const sin_sig2 = sin(sig2);
            CT const norm2 = math::sqrt(cos_alp0 * cos_alp0 *
                                        cos_sig2 * cos_sig2
                                        + sin_alp0 * sin_alp0);

            CT const bet2 = atan2(cos_alp0 * sin_sig2, norm2);
            CT const lat2 = atan(tan(bet2)/(1-f));

            result.lat2 = lat2;

            // Compute lon
            CT const n = f / (CT(2) - f);
            CT const omg1 = atan2(sin_alp0 * sin_sig1, cos_sig1);
            CT const omg2 = atan2(sin_alp0 * sin_sig2, cos_sig2);

            unsigned int const order_1 = get_order()+1;
            CT A3_coeff[order_1];
            evaluate_coeffs_A3_I3(n, A3_coeff);
            CT A3 = horner_evaluate(eps, A3_coeff, A3_coeff+order_1);

            unsigned int const size = order_1*order_1*order_1/(order_1+1)/2; //Triangular numbers
            CT C3_coeff[size];
            evaluate_coeffs_C3_I3(n, C3_coeff);
            CT C3[get_order()];
            evaluate_coeffs_var2(eps, C3_coeff, C3);

            CT const I3_sig1 = A3 * (sig1 + clenshaw_sum(sin_sig1, cos_sig1, C3, C3+get_order()));
            CT const I3_sig2 = A3 * (sig2 + clenshaw_sum(sin_sig2, cos_sig2, C3, C3+get_order()));

            CT const lam1 = omg1 - f * sin_alp0 * I3_sig1;
            CT const lam2 = omg2 - f * sin_alp0 * I3_sig2;
            CT const lam12 = lam2 - lam1;

            result.lon2 = lon1 + lam12;
        }

        if (BOOST_GEOMETRY_CONDITION(CalcQuantities))
        {
            typedef differential_quantities<CT, EnableReducedLength, EnableGeodesicScale, Order> quantities;
            quantities::apply(lon1, lat1, result.lon2, result.lat2,
                              azimuth12, result.reverse_azimuth,
                              b, f,
                              result.reduced_length, result.geodesic_scale);
        }

        return result;
    }

private:

    static inline unsigned int get_order()
    {
        return (Order > 5) ? 5 : Order;
    }

    static inline CT evaluate_coeffs_A1_I1(CT eps)
    {
        CT eps2 = eps * eps;
        CT t;
        switch (get_order()/2) {
        case 0:
            t = CT(0);
            break;
        case 1:
            t = eps2/CT(4);
            break;
        case 2:
            t = eps2*(eps2+CT(16))/CT(64);
            break;
        default:
            t = eps2*(eps2*(eps2+CT(4))+CT(64))/CT(256);
            break;
        }
        return (t + eps) / (1 - eps) + 1;
    }

    static inline void evaluate_coeffs_C_I1(CT eps, CT c[])
    {
        CT eps_sq = eps * eps;
        CT d = eps;
        switch (get_order()) {
        case 0:
            break;
        case 1:
            c[0] = -d/CT(2);
            break;
        case 2:
            c[0] = -d/CT(2);
            d *= eps;
            c[1] = -d/CT(16);
            break;
        case 3:
            c[0] = d*(CT(3)*eps_sq-CT(8))/CT(16);
            d *= eps;
            c[1] = -d/CT(16);
            d *= eps;
            c[2] = -d/CT(48);
            break;
        case 4:
            c[0] = d*(CT(3)*eps_sq-CT(8))/CT(16);
            d *= eps;
            c[1] = d*(eps_sq-CT(2))/CT(32);
            d *= eps;
            c[2] = -d/CT(48);
            d *= eps;
            c[3] = -CT(5)*d/CT(512);
            break;
        default:
            c[0] = d*((CT(6)-eps_sq)*eps_sq-CT(16))/CT(32);
            d *= eps;
            c[1] = d*(eps_sq-CT(2))/CT(32);
            d *= eps;
            c[2] = d*(CT(9)*eps_sq-CT(16))/CT(768);
            d *= eps;
            c[3] = -CT(5)*d/CT(512);
            d *= eps;
            c[4] = -CT(7)*d/CT(1280);
            break;
        }
    }

    static inline void evaluate_coeffs_Cp(CT eps, CT c[])
    {
        CT const eps_sq =eps * eps;
        CT d = eps;
        switch (get_order()) {
        case 0:
            break;
        case 1:
            c[0] = d/CT(2);
            break;
        case 2:
            c[0] = d/CT(2);
            d *= eps;
            c[1] = CT(5)*d/CT(16);
            break;
        case 3:
            c[0] = d*(CT(16)-CT(9)*eps_sq)/CT(32);
            d *= eps;
            c[1] = CT(5)*d/CT(16);
            d *= eps;
            c[2] = CT(29)*d/CT(96);
            break;
        case 4:
            c[0] = d*(CT(16)-CT(9)*eps_sq)/CT(32);
            d *= eps;
            c[1] = d*(CT(30)-CT(37)*eps_sq)/CT(96);
            d *= eps;
            c[2] = CT(29)*d/CT(96);
            d *= eps;
            c[3] = CT(539)*d/CT(1536);
            break;
        default:
            c[0] = d*(eps_sq*(CT(205)*eps_sq-CT(432))+CT(768))/CT(1536);
            d *= eps;
            c[1] = d*(CT(30)-CT(37)*eps_sq)/CT(96);
            d *= eps;
            c[2] = d*(CT(116)-CT(225)*eps_sq)/CT(384);
            d *= eps;
            c[3] = CT(539)*d/CT(1536);
            d *= eps;
            c[4] = CT(3467)*d/CT(7680);
            break;
        }
    }

    static inline void evaluate_coeffs_A3_I3(CT n, CT c[])
    {
        switch(get_order()){
        case 0:
            c[0] = CT(1);
            break;
        case 1:
            c[0] = CT(1);
            c[1] = -CT(1)/CT(2);
            break;
        case 2:
            c[0] = CT(1);
            c[1] = (n-CT(1))/CT(2);
            c[2] = -CT(1)/CT(4);
            break;
        case 3:
            c[0] = CT(1);
            c[1] = (n-CT(1))/CT(2);
            c[2] = (-n-CT(2))/CT(8);
            c[3] = -CT(1)/CT(16);
            break;
        case 4:
            c[0] = CT(1);
            c[1] = (n-CT(1))/CT(2);
            c[2] = (n*(CT(3)*n-CT(1))-CT(2))/CT(8);
            c[3] = (-CT(3)*n-CT(1))/CT(16);
            c[4] = -CT(3)/CT(64);
            break;
        default:
            c[0] = CT(1);
            c[1] = (n-CT(1))/CT(2);
            c[2] = (n*(CT(3)*n-CT(1))-CT(2))/CT(8);
            c[3] = ((-n-CT(3))*n-CT(1))/CT(16);
            c[4] = (-CT(2)*n-CT(3))/CT(64);
            c[5] = -CT(3)/CT(128);
            break;
        }
    }

    static inline void evaluate_coeffs_C3_I3(CT n, CT c[])
    {
        CT n2 = n * n;
        switch (get_order()) {
        case 0:
            break;
        case 1:
            c[0] = CT(1)/CT(4);
            break;
        case 2:
            c[0] = (CT(1)-n)/CT(4);
            c[1] = CT(1)/CT(8);
            c[2] = CT(1)/CT(16);
            break;
        case 3:
            c[0] = (CT(1)-n)/CT(4);
            c[1] = CT(1)/CT(8);
            c[2] = CT(3)/CT(64);
            c[3] = (CT(2)-CT(3)*n)/CT(32);
            c[4] = CT(3)/CT(64);
            c[5] = CT(5)/CT(192);
            break;
        case 4:
            c[0] = (CT(1)-n)/CT(4);
            c[1] = (CT(1)-n2)/CT(8);
            c[2] = (CT(3)*n+CT(3))/CT(64);
            c[3] = CT(5)/CT(128);
            c[4] = ((n-CT(3))*n+CT(2))/CT(32);
            c[5] = (CT(3)-CT(2)*n)/CT(64);
            c[6] = CT(3)/CT(128);
            c[7] = (CT(5)-CT(9)*n)/CT(192);
            c[8] = CT(3)/CT(128);
            c[9] = CT(7)/CT(512);
            break;
        default:
            c[0] = (CT(1)-n)/CT(4);
            c[1] = (CT(1)-n2)/CT(8);
            c[2] = ((CT(3)-n)*n+CT(3))/CT(64);
            c[3] = (CT(2)*n+CT(5))/CT(128);
            c[4] = CT(3)/CT(128);
            c[5] = ((n-CT(3))*n+CT(2))/CT(32);
            c[6] = ((-CT(3)*n-CT(2))*n+CT(3))/CT(64);
            c[7] = (n+CT(3))/CT(128);
            c[8] = CT(5)/CT(256);
            c[9] = (n*(CT(5)*n-CT(9))+CT(5))/CT(192);
            c[10] = (CT(9)-CT(10)*n)/CT(384);
            c[11] = CT(7)/CT(512);
            c[12] = (CT(7)-CT(14)*n)/CT(512);
            c[13] = CT(7)/CT(512);
            c[14] = CT(21)/CT(2560);
            break;
        }
    }

    template <typename NT, typename IteratorType>
    static inline NT horner_evaluate(NT x,
                                     IteratorType begin,
                                     IteratorType end)
    {
        NT result(0);
        IteratorType it = end;
        do
        {
            result = result * x + *--it;
        }
        while (it != begin);
        return result;
    }

    static inline void evaluate_coeffs_var2(CT var2,
                                            CT coeffs1[],
                                            CT coeffs2[]){
        std::size_t begin(0), end(0);
        for(std::size_t i = 0; i < get_order(); i++){
            end = begin + get_order() - i;
            coeffs2[i] = std::pow(var2,i+1) * horner_evaluate(var2, coeffs1 + begin, coeffs1 + end);
            begin = end;
        }
    }

    /*
        Algorithm for summing trigonometric series
    */
    template <typename NT, typename IteratorType>
    static inline NT clenshaw_sum(NT sinx, NT cosx,
                                  IteratorType begin,
                                  IteratorType end)
    {
        //for small orders it is more efficient to use explicit formulas
        //https://trans4mind.com/personal_development/mathematics/trigonometry/multipleAnglesSine.htm
        unsigned int size = end-begin;
        NT b_k = 0;
        if (size == 0)
        {
            return b_k;
        }

        NT sin_cosx = sinx * cosx;
        NT sin2x = sinx * sinx;
        b_k = *begin * 2 * sin_cosx;
        if (size == 1)
        {
            return b_k;
        }

        NT sin3cosx = sin2x * sin_cosx;
        b_k += *(begin+1) * (4 * sin_cosx - 8 * sin3cosx);
        if (size == 2)
        {
            return b_k;
        }

        NT sin5cosx = sin2x * sin3cosx;
        b_k += *(begin+2) * (6 * sin_cosx - 32 * sin3cosx + 32 * sin5cosx);
        if (size == 3)
        {
            return b_k;
        }

        NT sin7cosx = sin2x * sin5cosx;
        b_k += *(begin+3) * (8 * sin_cosx - 80 * sin3cosx + 192 * sin5cosx - 128 * sin7cosx);
        if (size == 4)
        {
            return b_k;
        }

        NT sin9cosx = sin2x * sin7cosx;
        b_k += *(begin+4) * (10 * sin_cosx - 160 * sin3cosx + 672 * sin5cosx - 1024 * sin7cosx
                             + 512 * sin9cosx);
        if (size == 5)
        {
            return b_k;
        }

        //for higher orders use Clenshaw summation
        //https://en.wikipedia.org/wiki/Clenshaw_algorithm
        IteratorType it = end;
        bool odd = false;
        CT b_k1 = 0;
        CT b_k2 = 0;
        do
        {
            CT c_k = odd ? *--it : NT(0);
            b_k = c_k + NT(2) * cosx * b_k1 - b_k2;
            b_k2 = b_k1;
            b_k1 = b_k;
            odd = !odd;
        }
        while (it != begin);
        b_k = NT(2) * cosx * b_k1 - b_k2 ;
        return b_k * sinx;
    }

    template <typename T>
    struct return_result
    {
        T A1;
        T I1;
    };

    template <typename T>
    static inline return_result<T> compute_A1_I1(T eps, T sig1)
    {
        return_result<T> res;
        T const c1 = 1;
        T eps_m1 = c1 - eps;
        T A1 = c1;
        if (Order == 0)
        {
            res.A1 = A1 / eps_m1;
            res.I1 = A1 / eps_m1 * (sig1);
            return res;
        }

        T C11 = -0.5 * eps;
        if (Order == 1)
        {
            res.A1 = A1 / eps_m1;
            res.I1 = A1 / eps_m1 * (sig1 + C11 * sin(2 * sig1));
            return res;
        }

        T eps2 = eps * eps;
        A1 += 0.25 * eps2;
        T C12 = -0.0625 * eps2;
        if (Order == 2)
        {
            res.A1 = A1 / eps_m1;
            res.I1 = A1 / eps_m1 * (sig1 + C11 * sin(2 * sig1)
                                    + C12 * sin(4 * sig1));
            return res;
        }

        T eps3 = eps * eps2;
        C11 += 0.1875 * eps3;
        T C13 = -0.020833333 * eps3;
        if (Order == 3)
        {
            res.A1 = A1 / eps_m1;
            res.I1 = A1 / eps_m1 * (sig1 + C11 * sin(2 * sig1)
                                       + C12 * sin(4 * sig1)
                                       + C13 * sin(8 * sig1));
            return res;
        }

        T eps4 = eps2 * eps2;
        C12 += 0.03125 * eps4;
        T C14 = -0.009765625 * eps4;
        if (Order == 4)
        {
            res.A1 = A1 / eps_m1;
            res.I1 = A1 / eps_m1 * (sig1 + C11 * sin(2 * sig1)
                                       + C12 * sin(4 * sig1)
                                       + C13 * sin(8 * sig1)
                                       + C14 * sin(16 * sig1));
            return res;
        }

        //order 5 or higher
        T eps5 = eps2 * eps3;
        C13 += 0.01171875 * eps5;
        T C15 = -0.00546875 * eps5;
        res.A1 = A1 / eps_m1;
        res.I1 = A1 / eps_m1 * (sig1 + C11 * sin(2 * sig1)
                                   + C12 * sin(4 * sig1)
                                   + C13 * sin(6 * sig1)
                                   + C14 * sin(8 * sig1)
                                   + C15 * sin(10 * sig1));
        return res;
    }

    static inline bool vflip_if_south(CT const& lat1, CT const& azi12, CT & lat1_alt, CT & azi12_alt)
    {
        CT const c2 = 2;
        CT const pi = math::pi<CT>();
        CT const pi_half = pi / c2;

        if (azi12 > pi_half)
        {
            azi12_alt = pi - azi12;
            lat1_alt = -lat1;
            return true;
        }
        else if (azi12 < -pi_half)
        {
            azi12_alt = -pi - azi12;
            lat1_alt = -lat1;
            return true;
        }

        return false;
    }

    static inline void vflip_rev_azi(CT & rev_azi, CT const& azimuth12)
    {
        CT const c0 = 0;
        CT const pi = math::pi<CT>();

        if (rev_azi == c0)
        {
            rev_azi = azimuth12 >= 0 ? pi : -pi;
        }
        else if (rev_azi > c0)
        {
            rev_azi = pi - rev_azi;
        }
        else
        {
            rev_azi = -pi - rev_azi;
        }
    }

    static inline CT normalized1_1(CT const& value)
    {
        CT const c1 = 1;
        return value > c1 ? c1 :
               value < -c1 ? -c1 :
               value;
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_SERIES_EXPANSION_DIRECT_HPP
