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
#include <boost/math/special_functions/atanh.hpp>

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
    const std::size_t SeriesOrder = 2,
    typename Spheroid = void,
    typename CalculationType = void
>
class geographic_area
{

    //Select default types for Strategy, Spheroid and CalculationType
    //in case they are not set

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
                 * (CT(2.0) - CT(detail::flattening<CT>(spheroid))))
            , ep2(e2 / (CT(1.0) - e2))
            , ep(math::sqrt(ep2))
            , c2((a2 / CT(2.0)) +
              ((math::sqr(get_radius<2>(spheroid))
//                  * 1)
                * atanh(math::sqrt(e2)))
//                  * std::atanh(math::sqrt(e2)))
              / (CT(2.0) * math::sqrt(e2))))
        {}
    };

    struct area_sums
    {
        CT excess_sum;
        CT correction_sum;

        // Keep track if encircles some pole
        std::size_t crosses_prime_meridian;

        inline area_sums()
            : excess_sum(0)
            , correction_sum(0)
            , crosses_prime_meridian(0)
        {}
        inline CT area(spheroid_constants spheroid_const) const
        {
            CT result;

            CT sum = spheroid_const.c2 * excess_sum
                   + spheroid_const.e2 * spheroid_const.a2 * correction_sum;

            // If encircles some pole
            if(crosses_prime_meridian % 2 == 1)
            {
                std::size_t times_crosses_prime_meridian
                        = 1 + (crosses_prime_meridian / 2);

                result = CT(2.0)
                         * geometry::math::pi<CT>()
                         * spheroid_const.c2
                         * CT(times_crosses_prime_meridian)
                         - geometry::math::abs(sum);

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
    typedef area_sums state_type;

    inline geographic_area(SpheroidType spheroid = SpheroidType())
        :   spheroid_const(spheroid)
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                area_sums& state) const
    {

        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {

            //Compute the trapezoidal area
            state.excess_sum += geometry::formula::geographic_area
                        <CT>
                        ::template spherical_excess(p1, p2);

            state.correction_sum += geometry::formula::geographic_area
                        <CT, SeriesOrder>
                        ::template ellipsoidal_correction
                        <AzimuthStrategy>
                        (p1, p2, spheroid_const);

            //std::cout << "current excess=" << state.excess_sum
            //             << " correction=" << state.correction_sum
            //                << std::endl;

            // Keep track whenever a segment crosses the prime meridian
            geometry::formula::geographic_area
                        <CT>
                        ::crosses_prime_meridian(p1, p2, state);
        }
    }

    inline return_type result(area_sums const& state) const
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
