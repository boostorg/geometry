// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RELATE_PRIMITIVES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RELATE_PRIMITIVES_HPP

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/geometry/policies/relate/intersection_points.hpp>
#include <boost/geometry/policies/relate/intersection_policy.hpp>
#include <boost/geometry/strategies/cartesian/exact/iterator_point_view.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_box.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_predicates.hpp>
#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/detail/section.hpp>
#include <boost/geometry/util/sequence.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

using bounding_box = search_box;

template <typename Coordinate>
struct side
{
    using cs_tag = cartesian_tag;

    template <typename A, typename B, typename P>
    static int apply(A const& a, B const& b, P const& p)
    {
        check_coordinate_type<Coordinate, A>();
        check_coordinate_type<Coordinate, B>();
        check_coordinate_type<Coordinate, P>();
        return static_orientation<Coordinate>(a, b, p);
    }

    template <typename A, typename B, typename P>
    static int direction_code(A const& first, B const& second, P const& point)
    {
        return static_direction<Coordinate>(first, second, point);
    }
};

template <typename Coordinate>
struct point_equal
{
    using cs_tag = cartesian_tag;

    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    {
        check_coordinate_type<Coordinate, A>();
        check_coordinate_type<Coordinate, B>();
        return static_equal_points<Coordinate>(a, b);
    }
};

template <typename Coordinate>
struct winding
{
    using cs_tag = cartesian_tag;
    struct state_type { int count = 0; bool boundary = false; };

    template <typename P, typename FirstPoint, typename SecondPoint = FirstPoint>
    static bool apply(P const& point, FirstPoint const& first, SecondPoint const& second,
                      state_type& state)
    {
        check_coordinate_type<Coordinate, P>();
        check_coordinate_type<Coordinate, FirstPoint>();
        check_coordinate_type<Coordinate, SecondPoint>();
        int const ay = static_compare_coordinate<1, Coordinate>(first, point);
        int const by = static_compare_coordinate<1, Coordinate>(second, point);
        // A segment strictly above or below the query cannot cross its ray or
        // contain it. Equal ordinates need only the exact abscissa comparisons.
        if (ay * by > 0) { return true; }
        if (ay == 0 && by == 0)
        {
            if (static_compare_coordinate<0, Coordinate>(first, point)
                * static_compare_coordinate<0, Coordinate>(second, point) <= 0)
            {
                state.boundary = true;
                return false;
            }
            return true;
        }
        int const side = static_orientation<Coordinate>(first, second, point);
        if (side == 0 && static_compare_coordinate<0, Coordinate>(first, point)
                * static_compare_coordinate<0, Coordinate>(second, point) <= 0
            && ay * by <= 0)
        {
            state.boundary = true;
            return false;
        }
        if (ay <= 0 && by > 0 && side > 0)
        {
            ++state.count;
        }
        else if (by <= 0 && ay > 0 && side < 0)
        {
            --state.count;
        }
        return true;
    }

    static int result(state_type const& state)
    {
        return state.boundary ? 0 : state.count == 0 ? -1 : 1;
    }
};

template <typename Coordinate, typename ComparePolicy>
struct point_compare
{
    template <typename A, typename B>
    static bool apply(A const& first, B const& second)
    {
        check_coordinate_type<Coordinate, A>();
        check_coordinate_type<Coordinate, B>();
        int const x = static_compare_coordinate<0, Coordinate>(first, second);
        int const order = x == 0 ? static_compare_coordinate<1, Coordinate>(first, second) : x;
        return std::is_same<ComparePolicy, strategy::compare::equal_to>::value
            ? order == 0 : ComparePolicy::apply(order, 0);
    }
};

template <typename Coordinate>
struct topology
{
    template <typename A, typename B>
    bool cluster_equal(A const& a, B const& b) const
    {
        return point_equal<Coordinate>::apply(a, b);
    }

    template <typename A, typename B>
    int compare_y(A const& a, B const& b) const
    {
        return static_compare_coordinate<1, Coordinate>(a, b);
    }

    template <typename A, typename B>
    bool cluster_window_exceeded(A const& upper, B const& lower) const
    {
        return compare_y(upper, lower) > 0;
    }

    template <typename Indices, typename Turns>
    void colocate_cluster(Indices const& indices, Turns& turns) const
    {
        auto const point = turns[*indices.begin()].point;
        for (auto const index : indices)
        {
            turns[index].point = point;
        }
    }

    template <typename A, typename B>
    bool same_direction_point(A const& a, B const& b) const
    {
        return point_equal<Coordinate>::apply(a, b);
    }

    template <typename A, typename B>
    auto squared_distance(A const& first, B const& second) const
    {
        return static_squared_distance<Coordinate>(first, second);
    }

    template <typename Distance>
    bool distance_is_zero(Distance const& distance) const { return distance.is_zero(); }

    template <typename A, typename B, typename P>
    auto distance_measure(A const& first, B const& second, P const& point) const
    {
        return static_distance_measure<Coordinate>(first, second, point);
    }
};

template <typename Coordinate>
struct comparable_squared_distance
{
    template <typename A, typename B>
    static auto apply(A const& a, B const& b)
    { return static_squared_distance<Coordinate>(a, b); }
};

template <typename Coordinate>
struct static_section : strategies::detail::strict_cartesian_section
{
private:
    template <typename DimensionVector, std::size_t Index, typename Point>
    static int direction(Point const& previous, Point const& current)
    {
        using dimension = typename util::sequence_element<Index, DimensionVector>::type;
        return static_box_detail::compare_coordinate<dimension::value, Coordinate>(current, previous);
    }
    template <typename DimensionVector, typename Point, std::size_t... Index>
    static void directions_impl(Point const& previous, Point const& current, int* result,
                                std::index_sequence<Index...>)
    {
        int const unused[] = {0, (result[Index] = direction<DimensionVector, Index>(previous, current), 0)...};
        (void)unused;
    }
public:
    template <typename Iterator>
    static void assign_point(Iterator const& iterator, iterator_point_view<Iterator>& point)
    {
        point = iterator_point_view<Iterator>(iterator);
    }

    template <typename Iterator, typename Point>
    static auto assign_point(Iterator iterator, Point& point)
        -> std::enable_if_t<std::is_same<typename std::iterator_traits<Iterator>::value_type, Point>::value>
    {
        point = *iterator;
    }

    template <typename DimensionVector, typename Point>
    static void directions(Point const& previous, Point const& current, int* result)
    {
        directions_impl<DimensionVector>(previous, current, result,
            std::make_index_sequence<util::sequence_size<DimensionVector>::value>());
    }

    template <typename Point>
    static bool duplicate(Point const& previous, Point const& current)
    {
        return point_equal<Coordinate>::apply(previous, current);
    }

    template <std::size_t Dimension, typename Point, typename Box>
    static bool preceding(int direction, Point const& point, Box const&, Box const& other)
    {
        return (direction == 1 && static_box_detail::compare_coordinate<Dimension, Coordinate>
                    (point, static_box_detail::corner<min_corner>(other)) < 0)
            || (direction == -1 && static_box_detail::compare_coordinate<Dimension, Coordinate>
                    (point, static_box_detail::corner<max_corner>(other)) > 0);
    }
};

// Most overlay calls request implicit points. Direct primitive callers may
// request native output instead, so keep that conversion at the same boundary.
template <typename Coordinate, typename OutputPolicy>
struct output_segment_intersection : segment_intersection<Coordinate>
{
    using base_type = segment_intersection<Coordinate>;
    using base_type::apply;
    using exact_result = segment_intersection_result<exact_point<Coordinate>>;

    template <typename Result>
    static Result convert(exact_result const& exact)
    {
        Result result;
        result.count = exact.count;
        for (std::size_t i = 0; i != exact.count; ++i)
            point_converter<Coordinate, OutputPolicy>::apply(exact.intersections[i], result.intersections[i]);
        return result;
    }

    template <typename First, typename Second, typename Result>
    static Result apply(First const& first, Second const& second,
                        policies::relate::segments_intersection_points<Result> const& policy)
    {
        if constexpr (std::is_same<typename Result::point_type, exact_point<Coordinate>>::value)
        {
            return base_type::apply(first, second, policy);
        }
        else
        {
            auto const exact = base_type::apply(first, second,
                policies::relate::segments_intersection_points<exact_result>());
            return convert<Result>(exact);
        }
    }

    template <typename First, typename Second, typename Result>
    static auto apply(First const& first, Second const& second,
                      policies::relate::segments_intersection_policy<Result> const& policy)
        -> typename policies::relate::segments_intersection_policy<Result>::return_type
    {
        if constexpr (std::is_same<typename Result::point_type, exact_point<Coordinate>>::value)
        {
            return base_type::apply(first, second, policy);
        }
        else
        {
            auto const exact = base_type::apply(first, second,
                policies::relate::segments_intersection_policy<exact_result>());
            return {convert<Result>(exact.intersection_points), exact.direction};
        }
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
