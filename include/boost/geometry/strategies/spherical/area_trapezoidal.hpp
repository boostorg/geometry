// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AREA_TRAPEZOIDAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AREA_TRAPEZOIDAL_HPP


#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/formulas/geographic_area.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace area
{



/*!
\brief Area calculation by trapezoidal rule

}

*/
template
<
    typename PointOfSegment,
    typename CalculationType = void
>
class trapezoidal
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
        inline CT area(CT radius) const
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

            result *= radius * radius;

            return result;
        }
    };

public :
    typedef CT return_type;
    typedef PointOfSegment segment_point_type;
    typedef excess_sum state_type;

    inline trapezoidal(CT radius = 1.0)
        : m_radius(radius)
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                excess_sum& state) const
    {
        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {

            state.sum += geometry::formula::geographic_area
                                <CT>
                                ::spherical_excess(p1, p2);

            // Keep track whenever a segment crosses the prime meridian
            geometry::formula::geographic_area
                         <CT>
                         ::crosses_prime_meridian(p1, p2, state);

        }
    }

    inline return_type result(excess_sum const& state) const
    {
        std::cout << "(tpole=" << state.crosses_prime_meridian << ")";
        return state.area(m_radius);
    }

private :
    /// Radius of the sphere
    CT m_radius;
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{


template <typename Point>
struct default_strategy<spherical_equatorial_tag, Point>
{
    typedef strategy::area::trapezoidal<Point> type;
};

// Note: spherical polar coordinate system requires "get_as_radian_equatorial"
/***template <typename Point>
struct default_strategy<spherical_polar_tag, Point>
{
    typedef strategy::area::trapezoidal<Point> type;
};***/

} // namespace services

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::area




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AREA_TRAPEZOIDAL_HPP
