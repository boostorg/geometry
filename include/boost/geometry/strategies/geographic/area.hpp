// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP

#include <boost/geometry/algorithms/detail/andoyer_inverse.hpp>
#include <boost/geometry/algorithms/detail/thomas_inverse.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace area
{



/*!
\brief Area calculation by trapezoidal rule plus ellipsoidal correction

}

*/
template
<
    typename PointOfSegment,
    typename CalculationType = void
>
class geographic_area
{
typedef typename boost::mpl::if_c
    <
        boost::is_void<CalculationType>::type::value,
        typename select_most_precise
            <
                typename coordinate_type<PointOfSegment>::type,
                double
            >::type,
        CalculationType
    >::type CT;

protected :
    struct excess_sum
    {
        CT sum;

        // Keep track if encircles some pole
        size_t crosses_prime_meridian;

        inline excess_sum()
            : sum(0)
            , crosses_prime_meridian(0)
        {}
        inline CT area() const
        {
            CT result;

            std::cout << "(sum=" << sum << ")";

            // Encircles pole
            if(crosses_prime_meridian % 2 == 1)
            {
                size_t times_crosses_prime_meridian
                        = 1 + (crosses_prime_meridian / 2);

                result = 2.0
                         * geometry::math::pi<CT>()
                         * times_crosses_prime_meridian
                         - std::abs(sum);

                if(geometry::math::sign<CT>(sum) == 1)
                {
                    result = - result;
                }

            } else {
                result =  sum;
            }

            //result *= radius * radius;

            return result;
        }
    };

public :
    typedef CT return_type;
    typedef PointOfSegment segment_point_type;
    typedef excess_sum state_type;

    inline geographic_area()
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                excess_sum& state) const
    {

        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {
            //
            //TODO: from shperical
            //
            CT const two = 2.0;
            CT const pi
                = geometry::math::pi<CT>();
            CT const two_pi
                = geometry::math::two_pi<CT>();
            CT const half_pi
                = geometry::math::half_pi<CT>();

            // Trapezoidal formula
            //

            CT tan_lat1 =
                    tan(geometry::get_as_radian<1>(p1) / two);
            CT tan_lat2 =
                    tan(geometry::get_as_radian<1>(p2) / two);

            CT excess = two
                * atan(((tan_lat1 + tan_lat2) / (1 + tan_lat1 * tan_lat2))
                       * tan((geometry::get_as_radian<0>(p2)
                              - geometry::get_as_radian<0>(p1)) / 2));


            //GEO

            //TODO put it as member
            geometry::srs::spheroid<CT> spheroid;

            //TODO compute only once for the whole polygon
            CT const f = detail::flattening<CT>(spheroid);
            CT const one_minus_f = 1.0 - f;

            CT const a = get_radius<0>(spheroid); //equitorial radius
            CT const a2 = math::sqr(a);
            CT const b = a * one_minus_f; //polar semi-minor axis
            CT const b2 = math::sqr(b);
            CT const e2 = f * (2.0 - f);
            CT const e = math::sqrt(e2);
            CT const ep2 = e2 / math::sqr(one_minus_f);
            CT const ep = math::sqrt(ep2);

            CT const c2 = (a2 / 2.0) + ((b2 * std::atanh(e)) / (2.0 * e));  // authalic radius
            //----upto here only once


            int order = 2;
            CT alp1, alp2;

            //FIRST ORDER Approximation
            if(order == 1)
            {
                alp1 = geometry::detail::andoyer_inverse
                        <
                        CT, false, true
                        >::apply(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                 get_as_radian<0>(p2), get_as_radian<1>(p2),
                                 spheroid).azimuth;

                alp2 = geometry::detail::andoyer_inverse
                        <
                        CT, false, true
                        >::apply(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                 get_as_radian<0>(p2), get_as_radian<1>(p2),
                                 spheroid).reverse_azimuth;
            }
            //SECOND ORDER Approximation
            if(order == 2)
            {
                alp1 = geometry::detail::thomas_inverse
                        <
                        CT, false, true
                        >::apply(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                 get_as_radian<0>(p2), get_as_radian<1>(p2),
                                 spheroid).azimuth;

                alp2 = geometry::detail::thomas_inverse
                        <
                        CT, false, true
                        >::apply(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                 get_as_radian<0>(p2), get_as_radian<1>(p2),
                                 spheroid).reverse_azimuth;
            }

            //CT cos_a1 = geometry::math:cos(a1);
            //CT sin_a1 = geometry::math:sin(a1);
            //CT sin_ph1 = geometry::math:sin(get_as_radian<1>(p1));

            //CT cos_a0 = std::hypot(cos_a1, sin_a1 * sin_ph1);
            //CT sin_a0 = math::sqrt(1 - cos_a0 * cos_a0);

            CT cos_bet1 = cos(get_as_radian<1>(p1));
            CT cos_bet2 = cos(get_as_radian<1>(p2));

            CT sin_alp1 = sin(alp1);
            CT cos_alp1 = cos(alp1);
            CT sin_alp2 = sin(alp2);
            CT cos_alp2 = cos(alp2);

            CT sin_alp0 = sin_alp1 * cos_bet1;
            CT cos_alp0 = math::sqrt(1 - math::sqr(sin_alp0)); //TODO: do it more accurate!

            //CT sin_sig1 = sin_bet1;
            CT cos_sig1 = cos_alp1 * cos_bet1;
            //CT sin_sig2 = sin_bet2;
            CT cos_sig2 = cos_alp2 * cos_bet2;

            CT cos3_sig1 = 4.0 * std::pow(cos_sig1,3) - 3.0 * cos_sig1;
            CT cos3_sig2 = 4.0 * std::pow(cos_sig2,3) - 3.0 * cos_sig2;

            CT k2 = math::sqr(ep * cos_alp0);

            CT I12;

            if(order == 1)
            {
                CT c_40 = ((2.0 / 3.0) - (1.0 / 15.0) * ep2)
                        - ((1.0 / 20.0) - (1.0 / 35.0) * ep2) * k2;
                CT c_41 = ((1.0 / 180.0) - (1.0 / 315.0) * ep2) * k2;

                I12 = c_40 * (cos_sig2 - cos_sig1)
                    + c_41 * (cos3_sig2 - cos3_sig1);
            }
            if(order == 2)
            {
                CT ep4 = math::sqr(ep2);
                CT k4 = math::sqr(k2);

                CT c_40 = ((2.0 / 3.0) - (1.0 / 15.0) * ep2
                                       + (4.0 / 105.0) * ep4)
                        - ((1.0 / 20.0) - (1.0 / 35.0) * ep2
                                        + (2.0 /105.0) * ep4) * k2
                        + ((1.0 / 42.0) - (1.0 / 63.0) * ep2
                                       + (8.0 / 693.0) * ep4) * k4;

                CT c_41 = ((1.0 / 180.0) - (1.0 / 315.0) * ep2
                                         + (2.0 / 945.0) * ep4) * k2
                        - ((1.0 / 252.0) - (1.0 / 378.0) * ep2
                                        + (4.0 / 2079.0) * ep4) * k4;

                CT c_42 = ((1.0 / 2100.0) - (1.0 / 3150.0) * ep2
                                         + (4.0 / 17325.0) * ep4) * k4;

                CT cos5_sig1 = 16.0 * std::pow(cos_sig1,5)
                             - 20.0 * std::pow(cos_sig1,3)
                             + 5.0 * cos_sig1;
                CT cos5_sig2 = 16.0 * std::pow(cos_sig2,5)
                             - 20.0 * std::pow(cos_sig2,3)
                             + 5.0 * cos_sig2;

                I12 = c_40 * (cos_sig2 - cos_sig1)
                    + c_41 * (cos3_sig2 - cos3_sig1)
                    + c_42 * (cos5_sig2 - cos5_sig1);
            }

            state.sum += excess * c2 + e2 * a2 * cos_alp0 * sin_alp0 * I12;

            //
            //TODO: from shperical
            //
            // Keep track whenever a segment crosses the prime meridian
            // First normalize to [0,360)

            CT p1_lon = get_as_radian<0>(p1)
                                    - ( floor( get_as_radian<0>(p1) / two_pi ) * two_pi );
            CT p2_lon = get_as_radian<0>(p2)
                                    - ( floor( get_as_radian<0>(p2) / two_pi ) * two_pi );

            CT max_lon = std::max(p1_lon, p2_lon);
            CT min_lon = std::min(p1_lon, p2_lon);

            if(max_lon > pi && min_lon < pi && max_lon - min_lon > pi)
            {
                state.crosses_prime_meridian++;
            }


        }
    }

    inline return_type result(excess_sum const& state) const
    {
        return state.area();
    }

//private :
//    /// Radius of the sphere
//    CT m_radius;
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{


template <typename Point>
struct default_strategy<geographic_tag, Point>
{
    typedef strategy::area::geographic_area<Point> type;
};

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}

}} // namespace strategy::area




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP
