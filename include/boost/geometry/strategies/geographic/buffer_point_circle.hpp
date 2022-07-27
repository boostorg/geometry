// Boost.Geometry

// Copyright (c) 2018-2022 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2020-2021.
// Modifications copyright (c) 2020-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_POINT_CIRCLE_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_POINT_CIRCLE_HPP

#include <cstddef>

#include <boost/range/value_type.hpp>

#include <boost/geometry/srs/spheroid.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

/*!
\brief Create a circular buffer around a point, on the Earth
\ingroup strategies
\details This strategy can be used as PointStrategy for the buffer algorithm.
    It creates a circular buffer around a point, on the Earth. It can be applied
    for points and multi_points.

\qbk{
[heading Example]
[buffer_geographic_point_circle]
[buffer_geographic_point_circle_output]
[heading See also]
\* [link geometry.reference.algorithms.buffer.buffer_7_with_strategies buffer (with strategies)]
\* [link geometry.reference.strategies.strategy_buffer_point_circle point_circle]
\* [link geometry.reference.strategies.strategy_buffer_point_square point_square]
}
 */
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_point_circle
{
public :
    //! \brief Constructs the strategy
    //! \param count number of points for the created circle (if count
    //! is smaller than 3, count is internally set to 3)
    explicit geographic_point_circle(std::size_t count = 90)
        : m_count((count < 3u) ? 3u : count)
    {}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    //! Fills output_range with a circle around point using distance_strategy
    template
    <
        typename Point,
        typename OutputRange,
        typename DistanceStrategy
    >
    inline void apply(Point const& point,
                      DistanceStrategy const& distance_strategy,
                      OutputRange& output_range) const
    {
        using output_point_type = typename boost::range_value<OutputRange>::type;

        using calculation_type = typename select_calculation_type
            <
                Point, output_point_type,
                CalculationType
            >::type;

        auto const lon_rad = get_as_radian<0>(point);
        auto const lat_rad = get_as_radian<1>(point);

        calculation_type const buffer_distance = distance_strategy.apply(point,
            point, strategy::buffer::buffer_side_left);

        using direct_t = typename FormulaPolicy::template direct
            <
                calculation_type, true, false, false, false
            >;

        calculation_type const two_pi = geometry::math::two_pi<calculation_type>();
        calculation_type const pi = geometry::math::pi<calculation_type>();

        calculation_type const diff = two_pi / calculation_type(m_count);
        calculation_type angle = -pi;

        for (std::size_t i = 0; i < m_count; i++, angle += diff)
        {
            // If angle is zero, shift angle a tiny bit to avoid spikes.
            calculation_type const eps = angle == 0 ? 1.0e-10 : 0.0;
            auto const dir_rad = direct_t::apply(lon_rad, lat_rad,
                                        buffer_distance, angle + eps,
                                        m_spheroid);
            output_point_type p;
            set_from_radian<0>(p, dir_rad.lon2);
            set_from_radian<1>(p, dir_rad.lat2);
            output_range.push_back(p);
        }

        {
            // Close the range
            auto const p = output_range.front();
            output_range.push_back(p);
        }
    }
#endif // DOXYGEN_SHOULD_SKIP_THIS

private :
    std::size_t m_count;
    Spheroid m_spheroid;
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_POINT_CIRCLE_HPP
