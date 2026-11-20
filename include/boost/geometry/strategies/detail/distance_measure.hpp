// Boost.Geometry

// Copyright (c) 2019-2021 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2022 Oracle and/or its affiliates.
// Copyright (c) 2026 Tinko Bartels.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_DISTANCE_MEASURE_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_DISTANCE_MEASURE_HPP

#include <boost/geometry/arithmetic/infinite_line_functions.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/static_assert.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

template <typename T>
struct distance_measure
{
    T measure;

    distance_measure()
        : measure(T())
    {}

    bool is_zero() const { return ! is_positive() && ! is_negative(); }
    bool is_positive() const { return measure > T(0); }
    bool is_negative() const { return measure < T(0); }
};

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
    using result_type = detail::distance_measure<CalculationType>;

    template <typename SegmentPoint, typename Point>
    static result_type apply(SegmentPoint const&, SegmentPoint const&, Point const&)
    {
        return result_type();
    }
};

template <typename CalculationType>
struct side_measure<CalculationType, geographic_tag>
    : side_measure<CalculationType, spherical_tag>
{};

template <typename CalculationType>
struct side_measure<CalculationType, cartesian_tag>
{
    using result_type = detail::distance_measure<CalculationType>;

    template <typename SegmentPoint, typename Point>
    static result_type apply(SegmentPoint const& p1, SegmentPoint const& p2, Point const& p)
    {
        // Preserve the order of floating-point evaluation.
        model::infinite_line<CalculationType> line;
        line.a = geometry::get<1>(p1) - geometry::get<1>(p2);
        line.b = geometry::get<0>(p2) - geometry::get<0>(p1);
        line.c = -line.a * geometry::get<0>(p1) - line.b * geometry::get<1>(p1);
        result_type result;
        result.measure = arithmetic::side_value(line, p);
        return result;
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
        return detail::distance_measure<calc_t>();
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

template <typename Distance>
inline bool distance_is_zero(Distance const& distance)
{
    Distance const zero = 0;
    return geometry::math::equals(distance, zero);
}

} // namespace distance_measure_detail

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_DISTANCE_MEASURE_HPP
