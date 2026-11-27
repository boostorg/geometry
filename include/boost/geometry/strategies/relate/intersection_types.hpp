// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_INTERSECTION_TYPES_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_INTERSECTION_TYPES_HPP

#include <type_traits>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/policies/robustness/segment_ratio.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/geometries/box.hpp>

namespace boost { namespace geometry { namespace strategies { namespace relate { namespace services {


struct empty_operation_scope {};

template <typename Strategy, typename = void>
struct operation_scope { using type = empty_operation_scope; };

template <typename Strategy>
struct operation_scope<Strategy, std::void_t<typename Strategy::operation_scope>>
{ using type = typename Strategy::operation_scope; };

// Algorithm fastpaths require certified predicates for the represented points.
// In particular, both side() and relate()'s side information must report exact
// collinearity, consistently with topology().side_measure_sign(). A strategy can
// set this flag to false to retain recovery code without changing its kernel.
template <typename Strategy, typename = void>
struct exact_predicates : std::false_type {};

template <typename Strategy>
struct exact_predicates<Strategy, std::void_t<decltype(Strategy::exact_predicates)>>
    : std::integral_constant<bool, Strategy::exact_predicates>
{};

// Opt in only if static apply(a, b) is a strict weak ordering whose equivalence
// classes coincide with topology().cluster_equal(a, b). Tolerant equality does
// not in general satisfy this contract.
template <typename Strategy, typename = void>
struct exact_cluster_compare { using type = void; };

template <typename Strategy>
struct exact_cluster_compare<Strategy,
    std::void_t<typename Strategy::exact_cluster_compare>>
{
    using type = typename Strategy::exact_cluster_compare;
};

template <typename Strategy, typename Point, typename = void>
struct intersection_point_type { using type = Point; };

template <typename Strategy, typename Point>
struct intersection_point_type<Strategy, Point,
    std::void_t<typename Strategy::template intersection_point_type<Point>>>
{
    using type = typename Strategy::template intersection_point_type<Point>;
};

template <typename Strategy, typename Point, typename = void>
struct strategy_segment_ratio { using type = typename geometry::segment_ratio_type<Point>::type; };

template <typename Strategy, typename Point>
struct strategy_segment_ratio<Strategy, Point,
    std::void_t<typename Strategy::template segment_ratio_type<Point>>>
{
    using type = typename Strategy::template segment_ratio_type<Point>;
};

template <typename Strategy, typename Point, typename = void>
struct segment_ratio_type : strategy_segment_ratio<Strategy, Point> {};

template <typename Strategy, typename Point>
struct segment_ratio_type<Strategy, Point, std::void_t<typename Strategy::segment_position_type>>
{
    using type = typename Strategy::segment_position_type;
};

template <typename Strategy, typename Point, typename = void>
struct comparable_distance_type { using type = geometry::coordinate_type_t<Point>; };

template <typename Strategy, typename Point>
struct comparable_distance_type<Strategy, Point,
    std::void_t<typename Strategy::template comparable_distance_type<Point>>>
{
    using type = typename Strategy::template comparable_distance_type<Point>;
};

template <typename Strategy, typename Point, typename = void>
struct intersection_result_type
{
    using type = geometry::segment_intersection_points<Point,
        typename segment_ratio_type<Strategy, Point>::type>;
};

template <typename Strategy, typename Point>
struct intersection_result_type<Strategy, Point,
    std::void_t<typename Strategy::template intersection_result_type<Point>>>
{
    using type = typename Strategy::template intersection_result_type<Point>;
};

template <typename Strategy, typename Geometry, typename = void>
struct overlay_geometry_type
{
    using type = Geometry;
};

template <typename Strategy, typename Geometry>
struct overlay_geometry_type<Strategy, Geometry,
    std::void_t<typename Strategy::template overlay_geometry_type<Geometry>>>
{
    using type = typename Strategy::template overlay_geometry_type<Geometry>;
};

template <typename Strategy, typename Collection, typename = void>
struct overlay_output_type { using type = Collection; };

template <typename Strategy, typename Collection>
struct overlay_output_type<Strategy, Collection,
    std::void_t<typename Strategy::template overlay_output_type<Collection>>>
{
    using type = typename Strategy::template overlay_output_type<Collection>;
};

template <typename Strategy, typename Point, typename = void>
struct box_type { using type = model::box<Point>; };

template <typename Strategy, typename Point>
struct box_type<Strategy, Point,
    std::void_t<typename Strategy::template box_type<Point>>>
{
    using type = typename Strategy::template box_type<Point>;
};

// A search enclosure may be rounded outwards. Containment of a whole geometry
// needs its actual extrema instead of a conservative candidate-search box.
template <typename Strategy, typename Point, typename = void>
struct geometry_box_type : box_type<Strategy, Point> {};

template <typename Strategy, typename Point>
struct geometry_box_type<Strategy, Point,
    std::void_t<typename Strategy::template geometry_box_type<Point>>>
{
    using type = typename Strategy::template geometry_box_type<Point>;
};

}}}}} // namespace boost::geometry::strategies::relate::services

#endif
