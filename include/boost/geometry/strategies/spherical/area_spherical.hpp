// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AREA_SPHERICAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AREA_SPHERICAL_HPP

#include <boost/geometry/formulas/area_formulas.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace area
{

/*!
\brief Spherical area calculation by trapezoidal rule

}

*/
template
<
    typename PointOfSegment,
    bool LongSegment = false,
    typename CalculationType = void
>
class area_spherical
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
        CT m_sum;

        // Keep track if encircles some pole
        size_t m_crosses_prime_meridian;

        inline excess_sum()
            : m_sum(0)
            , m_crosses_prime_meridian(0)
        {}
        inline CT area(CT radius) const
        {
            CT result;

            // Encircles pole
            if(m_crosses_prime_meridian % 2 == 1)
            {
                size_t times_crosses_prime_meridian
                        = 1 + (m_crosses_prime_meridian / 2);

                result = CT(2)
                         * geometry::math::pi<CT>()
                         * times_crosses_prime_meridian
                         - geometry::math::abs(m_sum);

                if(geometry::math::sign<CT>(m_sum) == 1)
                {
                    result = - result;
                }

            } else {
                result =  m_sum;
            }

            result *= radius * radius;

            return result;
        }
    };

public :
    typedef CT return_type;
    typedef PointOfSegment segment_point_type;
    typedef excess_sum state_type;

    inline area_spherical(CT radius = 1.0)
        : m_radius(radius)
    {}

    inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                excess_sum& state) const
    {
        if (! geometry::math::equals(get<0>(p1), get<0>(p2)))
        {

            state.m_sum += geometry::formula::area_formulas
                             <CT>::template spherical_excess<LongSegment>(p1, p2);

            // Keep track whenever a segment crosses the prime meridian
            geometry::formula::area_formulas
                         <CT>::crosses_prime_meridian(p1, p2, state);

        }
    }

    inline return_type result(excess_sum const& state) const
    {
        //std::cout << "(tpole=" << state.crosses_prime_meridian << ")";
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
    typedef strategy::area::area_spherical<Point> type;
};

// Note: spherical polar coordinate system requires "get_as_radian_equatorial"
template <typename Point>
struct default_strategy<spherical_polar_tag, Point>
{
    typedef strategy::area::area_spherical<Point> type;
};

} // namespace services

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::area




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AREA_SPHERICAL_HPP
