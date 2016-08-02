// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/formulas/geographic_area.hpp>

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
    typename Strategy = void,
    typename Spheroid = void,
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

//If strategy not set select default strategy for azimuth computation
typedef typename boost::mpl::if_c
    <
        boost::is_void<Strategy>::type::value,
        typename geometry::formula::thomas_inverse
            <
                CT,
                false,
                true,
                true
            >,
        Strategy
    >::type AzimuthStrategy;

typedef typename boost::mpl::if_c
    <
        boost::is_void<Spheroid>::type::value,
        geometry::srs::spheroid<CT>,
        Spheroid
    >::type SpheroidType;

protected :
    struct spheroid_constants
    {
        SpheroidType spheroid;
        CT const a2;  // squared equatorial radius
        CT const e2;  // squared eccentricity
        CT const ep2; // squared second eccentricity
        CT const ep;  // second eccentricity
        CT const c2;  // authalic radius

        inline spheroid_constants(SpheroidType spheroid)
            : spheroid(spheroid)
            , a2(math::sqr(get_radius<0>(spheroid)))
            , e2(detail::flattening<CT>(spheroid)
                 * (2.0 - detail::flattening<CT>(spheroid)))
            , ep2(e2 / (1 - e2))
            , ep(math::sqrt(ep2))
            , c2((a2 / 2.0) +
              ((math::sqr(get_radius<2>(spheroid)) * std::atanh(math::sqrt(e2)))
              / (2.0 * math::sqrt(e2))))
        {}
    };

    struct spheroid_param
    {
        CT excess_sum;
        CT correction_sum;

        // Keep track if encircles some pole
        size_t crosses_prime_meridian;

        inline spheroid_param()
            : excess_sum(0)
            , correction_sum(0)
            , crosses_prime_meridian(0)
        {}
        inline CT area(spheroid_constants spheroid_const) const
        {
            CT result;

            CT sum = spheroid_const.c2 * excess_sum
                   + spheroid_const.e2 * spheroid_const.a2 * correction_sum;

            std::cout << "(result=" << result << ")";

            // If encircles some pole
            if(crosses_prime_meridian % 2 == 1)
            {
                size_t times_crosses_prime_meridian
                        = 1 + (crosses_prime_meridian / 2);

                result = 2.0
                         * geometry::math::pi<CT>()
                         * spheroid_const.c2
                         * times_crosses_prime_meridian
                         - std::abs(sum);

                if(geometry::math::sign<CT>(sum) == 1)
                {
                    result = - result;
                }

            } else {
                result = sum;
            }

            return result;
        }
    };

public :
    typedef CT return_type;
    typedef PointOfSegment segment_point_type;
    typedef spheroid_param state_type;

    inline geographic_area(SpheroidType spheroid)
        :   spheroid_const(spheroid)
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                spheroid_param& state) const
    {

        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {

            //Compute the trapezoidal area
            state.excess_sum += geometry::formula::geographic_area
                                <CT>
                                ::spherical_excess(p1, p2);


            state.correction_sum += geometry::formula::geographic_area
               <CT>::template ellipsoidal_correction<AzimuthStrategy, 2>
                    (p1, p2, spheroid_const);

            // Keep track whenever a segment crosses the prime meridian
            geometry::formula::geographic_area
                         <CT>
                         ::crosses_prime_meridian(p1, p2, state);
        }
    }

    inline return_type result(spheroid_param const& state) const
    {
        return state.area(spheroid_const);
    }

private:
    spheroid_constants spheroid_const;

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
