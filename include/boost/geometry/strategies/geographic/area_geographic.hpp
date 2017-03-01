// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP

#include <boost/geometry/formulas/area_formulas.hpp>
#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/math/special_functions/atanh.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace area
{

/*!
\brief Geographic area calculation by trapezoidal rule plus integral
approximation that gives the ellipsoidal correction


*/

template
<
    typename PointOfSegment,
    template <typename, bool, bool, bool, bool, bool> class Inverse =
              geometry::formula::thomas_inverse,
    std::size_t SeriesOrder = 2,
    bool ExpandEpsN = true,
    bool LongSegment = false,
    typename Spheroid = void,
    typename CalculationType = void
>
class geographic
{

    //Select default types in case they are not set

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
/*
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
*/
    typedef typename boost::mpl::if_c
    <
        boost::is_void<Spheroid>::type::value,
        geometry::srs::spheroid<CT>,
        Spheroid
    >::type SpheroidType;

protected :
    struct spheroid_constants
    {
        SpheroidType m_spheroid;
        CT const m_a2;  // squared equatorial radius
        CT const m_e2;  // squared eccentricity
        CT const m_ep2; // squared second eccentricity
        CT const m_ep;  // second eccentricity
        CT const m_c2;  // authalic radius

        inline spheroid_constants(SpheroidType spheroid)
            : m_spheroid(spheroid)
            , m_a2(math::sqr(get_radius<0>(spheroid)))
            , m_e2(formula::flattening<CT>(spheroid)
                 * (CT(2.0) - CT(formula::flattening<CT>(spheroid))))
            , m_ep2(m_e2 / (CT(1.0) - m_e2))
            , m_ep(math::sqrt(m_ep2))
            , m_c2((m_a2 / CT(2.0)) +
              ((math::sqr(get_radius<2>(spheroid)) * boost::math::atanh(math::sqrt(m_e2)))
               / (CT(2.0) * math::sqrt(m_e2))))
        {}
    };

    struct area_sums
    {
        CT m_excess_sum;
        CT m_correction_sum;

        // Keep track if encircles some pole
        std::size_t m_crosses_prime_meridian;

        inline area_sums()
            : m_excess_sum(0)
            , m_correction_sum(0)
            , m_crosses_prime_meridian(0)
        {}
        inline CT area(spheroid_constants spheroid_const) const
        {
            CT result;

            CT sum = spheroid_const.m_c2 * m_excess_sum
                   + spheroid_const.m_e2 * spheroid_const.m_a2 * m_correction_sum;

            // If encircles some pole
            if (m_crosses_prime_meridian % 2 == 1)
            {
                std::size_t times_crosses_prime_meridian
                        = 1 + (m_crosses_prime_meridian / 2);

                result = CT(2.0)
                         * geometry::math::pi<CT>()
                         * spheroid_const.m_c2
                         * CT(times_crosses_prime_meridian)
                         - geometry::math::abs(sum);

                if (geometry::math::sign<CT>(sum) == 1)
                {
                    result = - result;
                }

            }
            else
            {
                result = sum;
            }

            return result;
        }
    };

public :
    typedef CT return_type;
    typedef PointOfSegment segment_point_type;
    typedef area_sums state_type;

    inline geographic(SpheroidType spheroid = SpheroidType())
        :   spheroid_const(spheroid)
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                area_sums& state) const
    {

        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {

            typedef geometry::formula::area_formulas<CT, SeriesOrder,
                    ExpandEpsN> area_formulas;

            typename area_formulas::return_type_ellipsoidal result =
                     area_formulas::template ellipsoidal<Inverse>
                                             (p1, p2, spheroid_const);

            state.m_excess_sum += result.spherical_term;
            state.m_correction_sum += result.ellipsoidal_term;

            // Keep track whenever a segment crosses the prime meridian
            geometry::formula::area_formulas<CT>
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
    typedef strategy::area::geographic<Point> type;
};

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}

}} // namespace strategy::area




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AREA_HPP
