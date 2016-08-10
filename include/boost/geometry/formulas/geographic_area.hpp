// Boost.Geometry

// Copyright (c) 2015-2016 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_GEOGRAPHIC_AREA_HPP
#define BOOST_GEOMETRY_FORMULAS_GEOGRAPHIC_AREA_HPP

#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/formulas/vincenty_inverse.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Formulas for computing spherical and ellipsoidal polygon area.
 The current class computes the area of the trapezoid defined by a segment
 the two meridians passing by the endpoints and the equator.
\author See
- Charles F.F Karney, Algorithms for geodesics, 2011
https://arxiv.org/pdf/1109.4448.pdf
*/

template <
    typename CT,
    const std::size_t SeriesOrder = 2,
    bool LongSegment = false
>
class geographic_area
{

public:

    /*
        Evaluate the polynomial in x, of coefficients
        CoefficientsOfPolynomial, using Horner's method.
    */
    template <class IteratorType>
    static inline CT HornerEvaluate(CT x,
                                    IteratorType begin,
                                    IteratorType end)
    {
        CT result(0);
        IteratorType it = end;
        do{
            result = result * x + *--it;
        }while(it != begin);
        return result;
    }


    /*
        Evaluate the series given the set of coefficients A[]
    */
    static inline void EvaluateSeries(CT eps,
                                      CT A[],
                                      CT c_4[]){
        std::size_t begin(0), end(0);
        for(std::size_t i = 0; i <= SeriesOrder; i++){
            end = begin + SeriesOrder + 1 - i;
            //replace pow()
            c_4[i] = pow(eps,i) *
                     HornerEvaluate(eps,
                                    A + begin,
                                    A + end);
            begin = end;
        }
        //E.g. for SeriesOrder = 2
        //c_4[0] = HornerEvaluate(eps, A, A + 3);
        //c_4[1] = eps * HornerEvaluate(eps, A + 3, A + 5);
        //c_4[2] = eps * eps * HornerEvaluate(eps, A + 5, A + 6);
        //
        //CT c_40 = _C4x[0] + eps * (_C4x[1] + eps * _C4x[2]);
        //CT c_41 = eps * (_C4x[3] + eps * (_C4x[4]));
        //CT c_42 = eps * (eps * _C4x[5]);
    }

    /*
        Clenshaw algorithm for summing trigonometric series
    */
    static inline CT ClenshawSum(CT cosx,
                                 const CT c[])
    {
        int n = 2 * SeriesOrder + 1;
        CT b_k, b_k1(0), b_k2(0);
        do{
            CT c_k = n % 2 ? c[n / 2] : CT(0);
            b_k = c_k + CT(2) * cosx * b_k1 - b_k2;
            b_k2 = b_k1;
            b_k1 = b_k;
        }while(--n);

        return c[0] + b_k1 * cosx - b_k2;
    }

    /*
        Compute the spherical excess of a geodesic (or shperical) segment
    */
    template <
                typename PointOfSegment
             >
    static inline CT spherical_excess(PointOfSegment const& p1,
                                      PointOfSegment const& p2)
    {
        //Spherical excess computation

        CT excess;

        if(LongSegment)
        {
            CT cbet1 = cos(geometry::get_as_radian<1>(p1));
            CT sbet1 = sin(geometry::get_as_radian<1>(p1));
            CT cbet2 = cos(geometry::get_as_radian<1>(p2));
            CT sbet2 = sin(geometry::get_as_radian<1>(p2));
            CT omg12 = geometry::get_as_radian<0>(p1)
                     - geometry::get_as_radian<0>(p2);
            CT comg12 = cos(omg12);
            CT somg12 = sin(omg12);

            CT alp1 = atan2(cbet1 * sbet2
                            - sbet1 * cbet2 * comg12,
                            cbet2 * somg12);

            CT alp2 = atan2(cbet1 * sbet2 * comg12
                            - sbet1 * cbet2,
                            cbet1 * somg12);

            excess = alp2 - alp1;

        } else { // Trapezoidal formula
            CT tan_lat1 =
                    tan(geometry::get_as_radian<1>(p1) / 2.0);
            CT tan_lat2 =
                    tan(geometry::get_as_radian<1>(p2) / 2.0);

            excess = CT(2.0)
                    * atan(((tan_lat1 + tan_lat2) / (CT(1) + tan_lat1 * tan_lat2))
                           * tan((geometry::get_as_radian<0>(p2)
                                - geometry::get_as_radian<0>(p1)) / 2));
        }

        return excess;
    }

    /*
        Compute the ellipsoidal correction of a geodesic (or shperical) segment
    */
    template <
                typename AzimuthStrategy,
                typename PointOfSegment,
                typename SpheroidConst
             >
    static inline CT ellipsoidal_correction(PointOfSegment const& p1,
                                            PointOfSegment const& p2,
                                            SpheroidConst spheroid_const)
    {


        // 1. Azimuth Approximation

        CT alp1, alp2;


        alp1 = AzimuthStrategy::apply(get_as_radian<0>(p1),
                                      get_as_radian<1>(p1),
                                      get_as_radian<0>(p2),
                                      get_as_radian<1>(p2),
                                      spheroid_const.spheroid).azimuth;

        alp2 = AzimuthStrategy::apply(get_as_radian<0>(p1),
                                      get_as_radian<1>(p1),
                                      get_as_radian<0>(p2),
                                      get_as_radian<1>(p2),
                                      spheroid_const.spheroid).reverse_azimuth;
        // 2. Integral approximation

        // Epsoidal constants
        //CT const ep2 = spheroid_const.ep2;
        CT const ep = spheroid_const.ep;
        CT const f = detail::flattening<CT>(spheroid_const.spheroid);

        //CT cos_a1 = geometry::math:cos(a1);
        //CT sin_a1 = geometry::math:sin(a1);
        //CT sin_ph1 = geometry::math:sin(get_as_radian<1>(p1));

        //CT cos_a0 = std::hypot(cos_a1, sin_a1 * sin_ph1);
        //CT sin_a0 = math::sqrt(1 - cos_a0 * cos_a0);

        CT cos_bet1 = cos(get_as_radian<1>(p1));
        CT cos_bet2 = cos(get_as_radian<1>(p2));
        //CT sin_bet1 = sin(get_as_radian<1>(p1));
        //CT sin_bet2 = sin(get_as_radian<1>(p2));

        CT sin_alp1 = sin(alp1);
        CT cos_alp1 = cos(alp1);
        //CT sin_alp2 = sin(alp2);
        CT cos_alp2 = cos(alp2);

        CT sin_alp0 = sin_alp1 * cos_bet1;
        //TODO: do it more accurate!
        CT cos_alp0 = math::sqrt(CT(1) - math::sqr(sin_alp0));

        //CT sin_sig1 = sin_bet1;
        CT cos_sig1 = cos_alp1 * cos_bet1;
        //CT sin_sig2 = sin_bet2;
        CT cos_sig2 = cos_alp2 * cos_bet2;

        CT k2 = math::sqr(ep * cos_alp0);

        CT sqrt_k2_plus_one = math::sqrt(CT(1) + k2);
        CT eps = (sqrt_k2_plus_one - 1) / (sqrt_k2_plus_one + 1);
        CT n = f / (CT(2) - f);

        const std::size_t C4_size = ((SeriesOrder + 2) * (SeriesOrder + 1)) / 2;
        CT _C4x[C4_size];

        switch(SeriesOrder + 1){
        case 0:
            break;
        case 1:
            _C4x[0] = 2.0/3.0;
            break;
        case 2:
            _C4x[0] = (10.0-4.0*n)/15.0;
            _C4x[1] = -1.0/5.0;
            _C4x[2] = 1.0/45.0;
            //Alternatively...
            //{CT A[][C4_size] = { {10, -4, 15},
            //                    {-1, 5},
            //                    {1, 45} };
            //_C4x[0] = HornerEvaluate(n, A[0], A[0] + 2) / A[0][2];
            //_C4x[1] = HornerEvaluate(n, A[1], A[1] + 1) / A[1][1];
            //_C4x[2] = HornerEvaluate(n, A[2], A[2] + 1) / A[2][1];}
            break;
        case 3:
            _C4x[0] = (n*(8.0*n-28.0)+70.0)/105.0;
            _C4x[1] = (16.0*n-7.0)/35.0;
            _C4x[2] = -2.0/105.0;
            _C4x[3] = (7.0-16.0*n)/315.0;
            _C4x[4] = -2.0/105.0;
            _C4x[5] = 4.0/525.0;
            break;
        case 4:
            _C4x[0] = (n*(n*(4.*n+24.)-84.)+210.)/315.;
            _C4x[1] = ((48.-32.*n)*n-21.)/105.;
            _C4x[2] = (-32.*n-6.)/315;
            _C4x[3] = 11./315;
            _C4x[4] = (n*(32.*n-48.)+21.)/945;
            _C4x[5] = (64.*n-18.)/945;
            _C4x[6] = -1./105;
            _C4x[7] = (12.-32*n)/1575;
            _C4x[8] = -8./1575;
            _C4x[9] = 8./2205;
            break;
        case 5:
            _C4x[0] = (n*(n*(n*(16.*n+44)+264)-924)+2310)/3465;
            _C4x[1] = (n*(n*(48.*n-352)+528)-231)/1155;
            _C4x[2] = (n*(1088.*n-352)-66)/3465;
            _C4x[3] = (121.-368*n)/3465;
            _C4x[4] = 4./1155;
            _C4x[5] = (n*((352.-48*n)*n-528)+231)/10395;
            _C4x[6] = ((704.-896*n)*n-198)/10395;
            _C4x[7] = (80*n-99)/10395;
            _C4x[8] = 4./1155;
            _C4x[9] = (n*(320.*n-352)+132)/17325;
            _C4x[10] = (384.*n-88)/17325;
            _C4x[11] = -8./1925;
            _C4x[12] = (88.-256*n)/24255;
            _C4x[13] = -16./8085;
            _C4x[14] = 64./31185;
            break;
        case 6:
            _C4x[0] = (n*(n*(n*(n*(100.*n+208)+572)+3432)-12012)+30030)/45045;
            _C4x[1] = (n*(n*(n*(64.*n+624)-4576)+6864)-3003)/15015;
            _C4x[2] = (n*((14144.-10656*n)*n-4576)-858)/45045;
            _C4x[3] = ((-224.*n-4784)*n+1573)/45045;
            _C4x[4] = (1088.*n+156)/45045;
            _C4x[5] = 97./15015;
            _C4x[6] = (n*(n*((-64.*n-624)*n+4576)-6864)+3003)/135135;
            _C4x[7] = (n*(n*(5952*n-11648)+9152)-2574)/135135;
            _C4x[8] = (n*(5792.*n+1040)-1287)/135135;
            _C4x[9] = (468.-2944*n)/135135;
            _C4x[10] = 1./9009;
            _C4x[11] = (n*((4160.-1440*n)*n-4576)+1716)/225225;
            _C4x[12] = ((4992.-8448*n)*n-1144)/225225;
            _C4x[13] = (1856.*n-936)/225225;
            _C4x[14] = 8./10725;
            _C4x[15] = (n*(3584.*n-3328)+1144)/315315;
            _C4x[16] = (1024.*n-208)/105105;
            _C4x[17] = -136./63063;
            _C4x[18] = (832.-2560*n)/405405;
            _C4x[19] = -128./135135;
            _C4x[20] = 128./99099;
            break;
        }

        CT c_4[SeriesOrder + 1];
        EvaluateSeries(eps,_C4x,c_4);

        CT I12 = ClenshawSum(cos_sig2, c_4)
               - ClenshawSum(cos_sig1, c_4);

        //CT cos3_sig1 = CT(4.0) * cos_sig1 * cos_sig1 * cos_sig1 - CT(3.0) * cos_sig1;
        //CT cos3_sig2 = CT(4.0) * cos_sig2 * cos_sig2 * cos_sig2 - CT(3.0) * cos_sig2;

        /*
            if(SeriesOrder == 1)
            {
                //CT c_40 = (CT(2) / CT(3) - CT(4) / CT(15) * n)
                //        - CT(1) / CT(5) * eps;
                //CT c_41 = CT(1) / CT(45) * eps;

                //CT c_40 = _C4x[0] + _C4x[1] * eps;
                //CT c_41 = _C4x[2] * eps;

                //I12 = c_4[0] * (cos_sig2 - cos_sig1)
                //        + c_4[1] * (cos3_sig2 - cos3_sig1);
            }
            if(SeriesOrder == 2)
            {
                //                CT n2 = math::sqr(n);
                //                CT eps2 = math::sqr(eps);

                //CT c_40 = (CT(2) / CT(3) - CT(4) / CT(15) * n + CT(8) / CT(105)  * n2)
                //        - (CT(1) / CT(5) - CT(16) / CT(35) * n) * eps
                //        - (CT(2) / CT(105)) * eps2;

                //CT c_41 = (CT(1) / CT(45) - CT(16) / CT(315) * n) * eps
                //        - (CT(2) / CT(105)) * eps2;

                //CT c_42 = (CT(4) / CT(525)) * eps2;

                //c_4[0] = HornerEvaluate(eps, _C4x, _C4x + 3);
                //c_4[1] = eps * HornerEvaluate(eps, _C4x + 3, _C4x + 5);
                //c_4[2] = eps * eps * HornerEvaluate(eps, _C4x + 5, _C4x + 6);
                //CT c_40 = _C4x[0] + eps * (_C4x[1] + eps * _C4x[2]);
                //CT c_41 = eps * (_C4x[3] + eps * (_C4x[4]));
                //CT c_42 = eps * (eps * _C4x[5]);



                CT cos5_sig1 = CT(16)
                        * cos_sig1 * cos_sig1 * cos_sig1 * cos_sig1 * cos_sig1
                        - CT(20)
                        * cos_sig1 * cos_sig1 * cos_sig1
                        + CT(5) * cos_sig1;
                CT cos5_sig2 = CT(16)
                        * cos_sig2 * cos_sig2 * cos_sig2 * cos_sig2 * cos_sig2
                        - CT(20)
                        * cos_sig2 * cos_sig2 * cos_sig2
                        + CT(5) * cos_sig2;

                I12 = c_4[0] * (cos_sig2 - cos_sig1)
                        + c_4[1] * (cos3_sig2 - cos3_sig1)
                        + c_4[2] * (cos5_sig2 - cos5_sig1);

*/

        return cos_alp0 * sin_alp0 * I12;

    }

    // Keep track whenever a segment crosses the prime meridian
    // First normalize to [0,360)
    template <typename PointOfSegment, typename StateType>
    static inline int crosses_prime_meridian(PointOfSegment const& p1,
                                             PointOfSegment const& p2,
                                             StateType& state)
    {
        CT const pi
            = geometry::math::pi<CT>();
        CT const two_pi
            = geometry::math::two_pi<CT>();

        CT p1_lon = get_as_radian<0>(p1)
                                - ( floor( get_as_radian<0>(p1) / two_pi )
                                  * two_pi );
        CT p2_lon = get_as_radian<0>(p2)
                                - ( floor( get_as_radian<0>(p2) / two_pi )
                                  * two_pi );

        CT max_lon = std::max(p1_lon, p2_lon);
        CT min_lon = std::min(p1_lon, p2_lon);

        if(max_lon > pi && min_lon < pi && max_lon - min_lon > pi)
        {
            state.crosses_prime_meridian++;
        }

        return state.crosses_prime_meridian;
    }

};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_GEOGRAPHIC_AREA_HPP
