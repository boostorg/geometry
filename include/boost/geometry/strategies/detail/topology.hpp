// Boost.Geometry

// Copyright (c) 2021-2023 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2024 Oracle and/or its affiliates.
// Copyright (c) 2026 Tinko Bartels.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_TOPOLOGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_TOPOLOGY_HPP

#include <type_traits>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/strategies/detail/distance_measure.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace topology_detail
{

template <typename T>
struct common_approximately_equals_epsilon_multiplier
{
    static T value()
    {
        return T(100);
    }
};

template <typename Point1, typename Point2, typename E>
inline bool approximately_equals(Point1 const& a, Point2 const& b,
                                 E const& epsilon_multiplier)
{
    using coor_t = typename select_coordinate_type<Point1, Point2>::type;
    using calc_t = typename geometry::select_most_precise<coor_t, E>::type;

    calc_t const& a0 = geometry::get<0>(a);
    calc_t const& b0 = geometry::get<0>(b);
    calc_t const& a1 = geometry::get<1>(a);
    calc_t const& b1 = geometry::get<1>(b);

    math::detail::equals_factor_policy<calc_t> policy(a0, b0, a1, b1);
    policy.multiply_epsilon(epsilon_multiplier);

    return math::detail::equals_by_policy(a0, b0, policy)
        && math::detail::equals_by_policy(a1, b1, policy);
}

template <bool Integral>
struct sweep_equal_policy
{
    template <typename Point1, typename Point2>
    static bool equals(Point1 const& p1, Point2 const& p2)
    {
        using coor_t = typename select_coordinate_type<Point1, Point2>::type;
        static auto const tolerance
            = common_approximately_equals_epsilon_multiplier<coor_t>::value();
        return topology_detail::approximately_equals(p1, p2, tolerance);
    }

    template <typename T>
    static bool exceeds(T value)
    {
        // Preserve the absolute sweep window, independently of scaled equality.
        static auto const tolerance
            = common_approximately_equals_epsilon_multiplier<T>::value();
        T const limit = T(1) / tolerance;
        return value > limit;
    }
};

template <>
struct sweep_equal_policy<true>
{
    template <typename Point1, typename Point2>
    static bool equals(Point1 const& p1, Point2 const& p2)
    {
        return geometry::get<0>(p1) == geometry::get<0>(p2)
            && geometry::get<1>(p1) == geometry::get<1>(p2);
    }

    template <typename T>
    static bool exceeds(T value)
    {
        return value > 0;
    }
};

template
<
    typename Point,
    typename CoordinateType = geometry::coordinate_type_t<Point>,
    typename CsTag = geometry::cs_tag_t<Point>,
    bool IsIntegral = std::is_integral<CoordinateType>::value
>
struct cluster_colocator
{
    template <typename TurnIndices, typename Turns>
    static void apply(TurnIndices const& indices, Turns& turns)
    {
        auto it = indices.begin();
        auto const& first_point = turns[*it].point;
        for (++it; it != indices.end(); ++it)
        {
            turns[*it].point = first_point;
        }
    }
};

template <typename Point, typename CoordinateType>
struct cluster_colocator<Point, CoordinateType, geometry::cartesian_tag, false>
{
    template <typename TurnIndices, typename Turns>
    static void apply(TurnIndices const& indices, Turns& turns)
    {
        CoordinateType centroid_0 = 0;
        CoordinateType centroid_1 = 0;
        for (auto const& index : indices)
        {
            centroid_0 += geometry::get<0>(turns[index].point);
            centroid_1 += geometry::get<1>(turns[index].point);
        }
        centroid_0 /= indices.size();
        centroid_1 /= indices.size();
        for (auto const& index : indices)
        {
            geometry::set<0>(turns[index].point, centroid_0);
            geometry::set<1>(turns[index].point, centroid_1);
        }
    }
};

} // namespace topology_detail

// Preserve the clustering and direction-point tolerances independently
// of the umbrella strategy's geometric predicates.
template <typename Strategy>
class tolerance_based_topology
{
public:
    explicit tolerance_based_topology(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Point1, typename Point2>
    bool cluster_equal(Point1 const& p1, Point2 const& p2) const
    {
        using coor_t = typename select_coordinate_type<Point1, Point2>::type;
        return topology_detail::sweep_equal_policy
            <std::is_integral<coor_t>::value>::equals(p1, p2);
    }

    template <typename Point1, typename Point2>
    int compare_y(Point1 const& p1, Point2 const& p2) const
    {
        auto const& y1 = geometry::get<1>(p1);
        auto const& y2 = geometry::get<1>(p2);
        return y1 < y2 ? -1 : y1 > y2 ? 1 : 0;
    }

    template <typename Point1, typename Point2>
    bool cluster_window_exceeded(Point1 const& upper, Point2 const& lower) const
    {
        using coor_t = typename select_coordinate_type<Point1, Point2>::type;
        return topology_detail::sweep_equal_policy
            <std::is_integral<coor_t>::value>::exceeds(
                geometry::get<1>(upper) - geometry::get<1>(lower));
    }

    template <typename TurnIndices, typename Turns>
    void colocate_cluster(TurnIndices const& indices, Turns& turns) const
    {
        using point_t = decltype(turns[*indices.begin()].point);
        topology_detail::cluster_colocator<point_t>::apply(indices, turns);
    }

    template <typename Point1, typename Point2>
    bool same_direction_point(Point1 const& p1, Point2 const& p2) const
    {
        // Direction walks historically use at least double precision.
        using coor_t = typename select_coordinate_type<Point1, Point2>::type;
        using calc_t = typename geometry::select_most_precise<coor_t, double>::type;
        static calc_t const tolerance
            = topology_detail::common_approximately_equals_epsilon_multiplier<calc_t>::value();
        return topology_detail::approximately_equals(p1, p2, tolerance);
    }

    template <typename Position1, typename Position2>
    bool same_segment_position(Position1 const& a, Position2 const& b) const
    {
        return geometry::math::equals(a, b);
    }

    template <typename SegmentPoint, typename Point>
    auto distance_measure(SegmentPoint const& p1, SegmentPoint const& p2, Point const& p) const
    {
        return distance_measure_detail::get_distance_measure(p1, p2, p, m_strategy);
    }

    template <typename Point1, typename Point2>
    auto squared_distance(Point1 const& a, Point2 const& b) const
    {
        return distance_measure_detail::squared_distance(a, b);
    }

    template <typename Distance>
    bool distance_is_zero(Distance const& distance) const
    {
        return distance_measure_detail::distance_is_zero(distance);
    }

private:
    Strategy const& m_strategy;
};

namespace topology_detail
{

template <typename Strategy>
inline auto get_topology(Strategy const& strategy, int)
    -> decltype(strategy.topology())
{
    return strategy.topology();
}

template <typename Strategy>
inline tolerance_based_topology<Strategy> get_topology(Strategy const& strategy, long)
{
    return tolerance_based_topology<Strategy>(strategy);
}

} // namespace topology_detail

template <typename Strategy>
inline decltype(auto) get_topology(Strategy const& strategy)
{
    return topology_detail::get_topology(strategy, 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_TOPOLOGY_HPP
