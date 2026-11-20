// Boost.Geometry

// Copyright (c) 2021-2022, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_BUFFER_CARTESIAN_HPP
#define BOOST_GEOMETRY_STRATEGIES_BUFFER_CARTESIAN_HPP


#include <cmath>
#include <type_traits>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/strategies/buffer/services.hpp>
#include <boost/geometry/strategies/distance/cartesian.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace buffer
{

// An independently composed Cartesian buffer umbrella may opt into this
// ordering when its turn points represent the positions traversal will visit.
// It does not supply or replace any predicate or distance strategy.
struct cartesian_touch_placement
{
    template <typename Fraction, typename Point, typename T = coordinate_type_t<Point>>
    static typename std::enable_if<std::is_floating_point<T>::value>::type
    place_touch(Fraction& fraction, Point const& from, Point const& to, Point const& point)
    {
        // Order the touch that traversal will visit, not a determinant ratio
        // that can round to the other endpoint on nearly parallel offsets.
        T const dx = get<0>(to) - get<0>(from), dy = get<1>(to) - get<1>(from);
        bool const x = std::abs(dx) >= std::abs(dy);
        T const length = x ? dx : dy;
        T const distance = x ? get<0>(point) - get<0>(from) : get<1>(point) - get<1>(from);
        if (length == 0 || ! std::isfinite(length) || ! std::isfinite(distance))
        {
            return;
        }
        Fraction const projected(distance, length);
        fraction = projected.left() ? Fraction(0, 1) : projected.right() ? Fraction(1, 1) : projected;
    }
};

template <typename CalculationType = void>
struct cartesian
    : public strategies::distance::cartesian<CalculationType>
    , public cartesian_touch_placement
{};


namespace services
{

template <typename Geometry>
struct default_strategy<Geometry, cartesian_tag>
{
    using type = strategies::buffer::cartesian<>;
};


} // namespace services

}} // namespace strategies::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_CARTESIAN_HPP
