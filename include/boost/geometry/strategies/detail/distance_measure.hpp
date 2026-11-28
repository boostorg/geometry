// Boost.Geometry

// Copyright (c) 2019-2021 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2022 Oracle and/or its affiliates.
// Copyright (c) 2026 Tinko Bartels.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_DISTANCE_MEASURE_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_DISTANCE_MEASURE_HPP

#include <type_traits>

#include <boost/geometry/arithmetic/infinite_line_functions.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/static_assert.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

// A strategy can supply a lazy squared distance instead of a scalar.
template <typename Distance, typename Compare>
bool compare_squared_distances(Distance const& a, Distance const& b, Compare const& compare)
{
    auto const compare_lazy = [&](auto const& value) -> decltype(value.compare_distance(b))
    { return value.compare_distance(b); };
    if constexpr (std::is_invocable_v<decltype(compare_lazy), Distance const&>)
    {
        return compare(compare_lazy(a), 0);
    }
    else
    {
        return compare(a, b);
    }
}

namespace distance_measure_detail
{

template <typename CalculationType, typename CsTag>
struct side_measure
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this coordinate system.", CsTag);
};

template <typename CalculationType>
struct side_measure<CalculationType, spherical_tag>
{
    template <typename SegmentPoint, typename Point>
    static CalculationType apply(SegmentPoint const&, SegmentPoint const&, Point const&)
    {
        return CalculationType(0);
    }
};

template <typename CalculationType>
struct side_measure<CalculationType, geographic_tag>
    : side_measure<CalculationType, spherical_tag>
{};

template <typename CalculationType>
struct side_measure<CalculationType, cartesian_tag>
{
    template <typename SegmentPoint, typename Point>
    static CalculationType apply(SegmentPoint const& p1, SegmentPoint const& p2, Point const& p)
    {
        // Preserve the order of floating-point evaluation.
        auto const line = arithmetic::make_infinite_line<CalculationType>(p1, p2);
        return arithmetic::side_value(line, p);
    }
};

template <typename SegmentPoint, typename Point, typename Strategy>
inline auto get_distance_measure(SegmentPoint const& p1, SegmentPoint const& p2,
                                 Point const& p, Strategy const&)
{
    using calc_t = typename select_coordinate_type<SegmentPoint, Point>::type;

    // Endpoint identity is deliberately strict, not tolerance-based.
    auto identical = [](auto const& a, auto const& b)
    {
        return geometry::get<0>(a) == geometry::get<0>(b)
            && geometry::get<1>(a) == geometry::get<1>(b);
    };
    if (identical(p1, p) || identical(p2, p))
    {
        return calc_t(0);
    }

    return side_measure<calc_t, typename Strategy::cs_tag>::apply(p1, p2, p);
}

template <typename Point1, typename Point2>
inline typename select_coordinate_type<Point1, Point2>::type
squared_distance(Point1 const& a, Point2 const& b)
{
    using coor_t = typename select_coordinate_type<Point1, Point2>::type;
    coor_t const dx = geometry::get<0>(a) - geometry::get<0>(b);
    coor_t const dy = geometry::get<1>(a) - geometry::get<1>(b);
    return dx * dx + dy * dy;
}

} // namespace distance_measure_detail

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_DISTANCE_MEASURE_HPP
