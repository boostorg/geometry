// Boost.Geometry

// Copyright (c) 2022 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_JOIN_MITER_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_JOIN_MITER_HPP

#include <boost/range/value_type.hpp>

#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/srs/spheroid.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_join_miter
{
    static bool const enable_azimuth = true;
    static bool const enable_coordinates = true;

    template <typename T>
    using inverse = typename FormulaPolicy::template inverse
        <
            T, false, enable_azimuth, false, false, false
        >;
    template <typename T>
    using direct = typename FormulaPolicy::template direct
        <
            T, enable_coordinates, false, false, false
        >;

public :

    //! \brief Constructs the strategy
    //! \param miter_limit The miter limit, to avoid excessively long miters around sharp corners
    explicit inline geographic_join_miter(double miter_limit = 5.0)
        : m_miter_limit(valid_limit(miter_limit))
    {}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    //! Fills output_range with a sharp shape around a vertex
    template <typename Point, typename DistanceType, typename RangeOut>
    inline bool apply(Point const& , Point const& vertex,
                      Point const& perp1, Point const& perp2,
                      DistanceType const& buffer_distance,
                      RangeOut& range_out) const
    {
        using calc_t = typename select_calculation_type
            <
                Point,
                typename boost::range_value<RangeOut>::type,
                CalculationType
            >::type;

        calc_t const lon_rad = get_as_radian<0>(vertex);
        calc_t const lat_rad = get_as_radian<1>(vertex);
        calc_t const lon1_rad = get_as_radian<0>(perp1);
        calc_t const lat1_rad = get_as_radian<1>(perp1);
        calc_t const lon2_rad = get_as_radian<0>(perp2);
        calc_t const lat2_rad = get_as_radian<1>(perp2);

        // Calculate angles from vertex to perp1/perp2
        auto const inv1 = inverse<calc_t>::apply(lon_rad, lat_rad, lon1_rad, lat1_rad, m_spheroid);
        auto const inv2 = inverse<calc_t>::apply(lon_rad, lat_rad, lon2_rad, lat2_rad, m_spheroid);

        // For a sharp corner, perpendicular points are nearly opposite and the
        // angle between the two azimuths can be nearly 180, but not more.
        calc_t const two_pi = geometry::math::two_pi<calc_t>();
        bool const wrapped = inv2.azimuth < inv1.azimuth;
        calc_t const angle_diff = wrapped
            ? ((two_pi + inv2.azimuth) - inv1.azimuth)
            : inv2.azimuth - inv1.azimuth;

        if (angle_diff < 0 || angle_diff > geometry::math::pi<calc_t>())
        {
            // Defensive check with asserts
            BOOST_GEOMETRY_ASSERT(angle_diff >= 0);
            BOOST_GEOMETRY_ASSERT(angle_diff <= geometry::math::pi<calc_t>());
            return false;
        }

        calc_t const half = 0.5;
        calc_t const half_angle_diff = half * angle_diff;
        calc_t const cos_angle = std::cos(half_angle_diff);

        calc_t const max_distance = m_miter_limit * geometry::math::abs(buffer_distance);

        if (cos_angle == 0)
        {
            // It is opposite, perp1==perp2, do not generate a miter cap
            return false;
        }

        // If it is sharp (angle close to 0), the distance will become too high and will be capped.
        calc_t const distance = (std::min)(max_distance, buffer_distance / cos_angle);
        calc_t const azi = math::wrap_azimuth_in_radian(inv1.azimuth + half_angle_diff);

        Point point;
        auto const d = direct<calc_t>::apply(lon_rad, lat_rad, distance, azi, m_spheroid);
        set_from_radian<0>(point, d.lon2);
        set_from_radian<1>(point, d.lat2);

        range_out.push_back(perp1);
        range_out.push_back(point);
        range_out.push_back(perp2);
        return true;
    }

    template <typename NumericType>
    inline NumericType max_distance(NumericType const& distance) const
    {
        return distance * m_miter_limit;
    }

#endif // DOXYGEN_SHOULD_SKIP_THIS

private :
    double valid_limit(double miter_limit) const
    {
        if (miter_limit < 1.0)
        {
            // It should always exceed the buffer distance
            miter_limit = 1.0;
        }
        return miter_limit;
    }

    double m_miter_limit;
    Spheroid m_spheroid;
};

}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_JOIN_MITER_HPP
