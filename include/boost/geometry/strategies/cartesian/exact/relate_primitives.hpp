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
#include <boost/geometry/views/detail/iterator_point_view.hpp>
#include <boost/geometry/strategies/cartesian/exact/box_primitives.hpp>
#include <boost/geometry/strategies/cartesian/exact/predicate_dispatch.hpp>
#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/detail/section.hpp>
#include <boost/geometry/util/sequence.hpp>
#include <boost/geometry/views/detail/indexed_point_view.hpp>

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

template <typename Coordinate, std::size_t Axis = 1>
struct winding
{
    static_assert(Axis < 2, "Cartesian winding requires a planar scan axis");
    using cs_tag = cartesian_tag;
    static constexpr int buffer_scan_axis = Axis;
    struct state_type { int count = 0; bool boundary = false; };

    template <typename P, typename Box>
    static bool section_may_contribute(P const& point, Box const& box)
    {
        geometry::detail::indexed_point_view<Box const, min_corner> const low(box);
        geometry::detail::indexed_point_view<Box const, max_corner> const high(box);
        return static_compare_coordinate<buffer_scan_axis, Coordinate>(low, point) <= 0
            && static_compare_coordinate<buffer_scan_axis, Coordinate>(high, point) >= 0;
    }

    template <typename P, typename Box>
    static bool section_may_contribute(P const&, Box const& box, search_box const& enclosure)
    {
        return geometry::get<min_corner, Axis>(enclosure) <= geometry::get<max_corner, Axis>(box)
            && geometry::get<max_corner, Axis>(enclosure) >= geometry::get<min_corner, Axis>(box);
    }

    template <typename P, typename FirstPoint, typename SecondPoint = FirstPoint>
    static bool apply(P const& point, FirstPoint const& first, SecondPoint const& second,
                      state_type& state)
    {
        check_coordinate_type<Coordinate, P>();
        check_coordinate_type<Coordinate, FirstPoint>();
        check_coordinate_type<Coordinate, SecondPoint>();
        int const a = static_compare_coordinate<Axis, Coordinate>(first, point);
        int const b = static_compare_coordinate<Axis, Coordinate>(second, point);
        // Only a segment meeting the scan line can contain the point or
        // cross its ray. A segment on that line only needs the other axis.
        if (a * b > 0) { return true; }
        if (a == 0 && b == 0)
        {
            if (static_compare_coordinate<1 - Axis, Coordinate>(first, point)
                * static_compare_coordinate<1 - Axis, Coordinate>(second, point) <= 0)
            {
                state.boundary = true;
                return false;
            }
            return true;
        }
        int const side = (Axis == 1 ? 1 : -1) * static_orientation<Coordinate>(first, second, point);
        if (side == 0 && static_compare_coordinate<1 - Axis, Coordinate>(first, point)
                * static_compare_coordinate<1 - Axis, Coordinate>(second, point) <= 0
            && a * b <= 0)
        {
            state.boundary = true;
            return false;
        }
        if (a <= 0 && b > 0 && side > 0)
        {
            ++state.count;
        }
        else if (b <= 0 && a > 0 && side < 0)
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
    static constexpr bool uses_cluster_tolerance = false;

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

    template <typename Distance>
    bool distance_is_zero(Distance const& distance) const { return distance.is_zero(); }
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
    static void assign_point(Iterator const& iterator, geometry::detail::iterator_point_view<Iterator>& point)
    {
        point = geometry::detail::iterator_point_view<Iterator>(iterator);
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

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
