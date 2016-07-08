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
    >::type calculation_type;

protected :
    struct excess_sum
    {
        calculation_type sum;

        // Keep track if encircles some pole
        size_t crosses_prime_meridian;

        inline excess_sum()
            : sum(0)
            , crosses_prime_meridian(0)
        {}
        inline calculation_type area(calculation_type radius) const
        {
            calculation_type result;

            std::cout << "(sum=" << sum << ")";

            // Encircles pole
            if(crosses_prime_meridian % 2 == 1)
            {
                size_t times_crosses_prime_meridian
                        = 1 + (crosses_prime_meridian / 2);

                result = 2.0
                         * geometry::math::pi<calculation_type>()
                         * times_crosses_prime_meridian
                         - std::abs(sum);

                if(geometry::math::sign<calculation_type>(sum) == 1)
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
    typedef calculation_type return_type;
    typedef PointOfSegment segment_point_type;
    typedef excess_sum state_type;

    inline trapezoidal(calculation_type radius = 1.0)
        : m_radius(radius)
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                excess_sum& state) const
    {
        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {
            calculation_type const two = 2.0;
            calculation_type const pi
                = geometry::math::pi<calculation_type>();
            calculation_type const two_pi
                = geometry::math::two_pi<calculation_type>();
            calculation_type const half_pi
                = geometry::math::half_pi<calculation_type>();

            // Trapezoidal formula
            //

            calculation_type tan_lat1 =
                    tan(geometry::get_as_radian<1>(p1) / two);
            calculation_type tan_lat2 =
                    tan(geometry::get_as_radian<1>(p2) / two);

            calculation_type excess = two
                * atan(((tan_lat1 + tan_lat2) / (1 + tan_lat1 * tan_lat2))
                       * tan((geometry::get_as_radian<0>(p2)
                              - geometry::get_as_radian<0>(p1)) / 2));

            // Keep track whenever a segment crosses the prime meridian
            // First normalize to [0,360)

            calculation_type p1_lon = get_as_radian<0>(p1)
                                    - ( floor( get_as_radian<0>(p1) / two_pi ) * two_pi );
            calculation_type p2_lon = get_as_radian<0>(p2)
                                    - ( floor( get_as_radian<0>(p2) / two_pi ) * two_pi );

            calculation_type max_lon = std::max(p1_lon, p2_lon);
            calculation_type min_lon = std::min(p1_lon, p2_lon);

            if(max_lon > pi && min_lon < pi && max_lon - min_lon > pi)
            {
                state.crosses_prime_meridian++;
            }

            state.sum += excess;
        }
    }

    inline return_type result(excess_sum const& state) const
    {
        std::cout << "(tpole=" << state.crosses_prime_meridian << ")";
        return state.area(m_radius);
    }

private :
    /// Radius of the sphere
    calculation_type m_radius;
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
