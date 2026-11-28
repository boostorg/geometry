// Boost.Geometry (aka GGL, Generic Geometry Library)
// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2014-2015 Adam Wulkiewicz, Lodz, Poland.
// Modifications copyright (c) 2013-2024 Oracle and/or its affiliates.
// Copyright (c) 2026 Tinko Bartels.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_SECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_SECTION_HPP

#include <cstddef>
#include <iterator>
#include <type_traits>

#include <boost/geometry/algorithms/detail/expand_by_epsilon.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/cartesian/point_in_box.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>
#include <boost/geometry/util/sequence.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace section_detail
{


// Latitude is not monotone along a general spherical/geographic segment.
template <typename Point, typename DimensionVector, std::size_t Index,
          std::size_t Count,
          typename CastedCSTag = tag_cast_t<cs_tag_t<Point>, spherical_tag>>
struct get_direction_loop
{
    using dimension = typename util::sequence_element<Index, DimensionVector>::type;

    template <typename Segment>
    static inline void apply(Segment const& segment, int directions[Count])
    {
        auto const& c0 = geometry::get<0, dimension::value>(segment);
        auto const& c1 = geometry::get<1, dimension::value>(segment);
        directions[Index] = c1 > c0 ? 1 : c1 < c0 ? -1 : 0;
        get_direction_loop<Point, DimensionVector, Index + 1, Count, CastedCSTag>
            ::apply(segment, directions);
    }
};

template <typename Point, typename DimensionVector, std::size_t Count>
struct get_direction_loop<Point, DimensionVector, 0, Count, spherical_tag>
{
    template <typename Segment>
    static inline void apply(Segment const& segment, int directions[Count])
    {
        using coordinate = coordinate_type_t<Segment>;
        using units = geometry::detail::coordinate_system_units_t<Point>;
        coordinate const difference = math::longitude_distance_signed<units, coordinate>
            (geometry::get<0, 0>(segment), geometry::get<1, 0>(segment));
        coordinate const zero = coordinate();
        directions[0] = difference > zero ? 1 : difference < zero ? -1 : 0;
        get_direction_loop<Point, DimensionVector, 1, Count, spherical_tag>
            ::apply(segment, directions);
    }
};

template <typename Point, typename DimensionVector, std::size_t Count, typename CastedCSTag>
struct get_direction_loop<Point, DimensionVector, Count, Count, CastedCSTag>
{
    template <typename Segment>
    static inline void apply(Segment const&, int[Count]) {}
};

template <bool Strict>
struct coordinate_equal
{
    template <typename A, typename B>
    static inline bool apply(A const& a, B const& b)
    {
        return geometry::math::equals(a, b);
    }
};

template <>
struct coordinate_equal<true>
{
    template <typename A, typename B>
    static inline bool apply(A const& a, B const& b) { return a == b; }
};

template <std::size_t Dimension, std::size_t Count, bool Strict = false>
struct check_duplicate_loop
{
    template <typename Segment>
    static inline bool apply(Segment const& segment)
    {
        return coordinate_equal<Strict>::apply(geometry::get<0, Dimension>(segment),
                                                geometry::get<1, Dimension>(segment))
            && check_duplicate_loop<Dimension + 1, Count, Strict>::apply(segment);
    }
};

template <std::size_t Count, bool Strict>
struct check_duplicate_loop<Count, Count, Strict>
{
    template <typename Segment>
    static inline bool apply(Segment const&) { return true; }
};

template <std::size_t Dimension, typename Geometry,
          typename CastedCSTag = tag_cast_t<cs_tag_t<Geometry>, spherical_tag>>
struct preceding_check
{
    template <typename Point, typename Box>
    static inline bool apply(int dir, Point const& point, Box const&, Box const& other_box)
    {
        return (dir == 1 && get<Dimension>(point) < get<min_corner, Dimension>(other_box))
            || (dir == -1 && get<Dimension>(point) > get<max_corner, Dimension>(other_box));
    }
};

template <typename Geometry>
struct preceding_check<0, Geometry, spherical_tag>
{
    template <typename Point, typename Box>
    static inline bool apply(int dir, Point const& point, Box const& point_box, Box const& other_box)
    {
        using coordinate = typename select_coordinate_type<Point, Box>::type;
        using units = geometry::detail::coordinate_system_units_t<Point>;
        coordinate const zero = 0;
        coordinate const value = get<0>(point);
        coordinate const other_min = get<min_corner, 0>(other_box);
        coordinate const other_max = get<max_corner, 0>(other_box);
        bool const covered = strategy::within::detail::covered_by_range<Point, 0, spherical_tag>
            ::apply(value, other_min, other_max);
        if (covered)
        {
            return false;
        }
        if (dir == 1)
        {
            coordinate const difference = math::longitude_distance_signed<units, coordinate>(other_min, value);
            coordinate const box_difference = math::longitude_distance_signed<units, coordinate>
                (other_min, get<min_corner, 0>(point_box));
            return difference < zero && box_difference <= zero && box_difference <= difference;
        }
        if (dir == -1)
        {
            coordinate const difference = math::longitude_distance_signed<units, coordinate>(other_max, value);
            coordinate const box_difference = math::longitude_distance_signed<units, coordinate>
                (other_max, get<max_corner, 0>(point_box));
            return difference > zero && box_difference >= zero && difference <= box_difference;
        }
        return false;
    }
};

} // namespace section_detail

template <typename Strategy, typename InputPoint, typename Box, typename = void>
struct section_point_type { using type = geometry::point_type_t<Box>; };

template <typename Strategy, typename InputPoint, typename Box>
struct section_point_type<Strategy, InputPoint, Box,
    std::void_t<typename Strategy::template section_point_type<InputPoint>>>
{
    using type = typename Strategy::template section_point_type<InputPoint>;
};

template <typename Strategy, typename Iterator, typename Box, typename = void>
struct section_iterator_point_type
    : section_point_type<Strategy, typename std::iterator_traits<Iterator>::value_type, Box>
{};

template <typename Strategy, typename Iterator, typename Box>
struct section_iterator_point_type<Strategy, Iterator, Box,
    std::void_t<typename Strategy::template section_iterator_point_type<Iterator>>>
{
    using type = typename Strategy::template section_iterator_point_type<Iterator>;
};

struct section_with_tolerance
{
    template <typename DimensionVector, typename Point>
    static inline void directions(Point const& previous, Point const& current, int* result)
    {
        model::referring_segment<Point const> const segment(previous, current);
        section_detail::get_direction_loop<Point, DimensionVector, 0,
            util::sequence_size<DimensionVector>::value>::apply(segment, result);
    }

    template <typename Point>
    static inline bool duplicate(Point const& previous, Point const& current)
    {
        model::referring_segment<Point const> const segment(previous, current);
        return section_detail::check_duplicate_loop<0, geometry::dimension<Point>::value>::apply(segment);
    }

    template <typename Box>
    static inline void expand_box(Box& box)
    {
        // Keep the section box conservative when coordinate comparisons use a tolerance.
        using coordinate = coordinate_type_t<Box>;
        static auto const epsilon = math::scaled_epsilon<coordinate>(1000);
        geometry::detail::expand_by_epsilon(box, epsilon);
    }

    template <std::size_t Dimension, typename Point, typename Box>
    static inline bool preceding(int dir, Point const& point, Box const& point_box, Box const& other_box)
    {
        return section_detail::preceding_check<Dimension, Box>::apply(dir, point, point_box, other_box);
    }
};

// Exact-point traits expose true rational coordinates, so these Cartesian
// comparisons remain exact for constructed points as well as fundamental input.
// No epsilon may turn a nonzero segment into a duplicate or change its bounds.
struct strict_cartesian_section
{
    template <typename DimensionVector, typename Point>
    static inline void directions(Point const& previous, Point const& current, int* result)
    {
        model::referring_segment<Point const> const segment(previous, current);
        section_detail::get_direction_loop<Point, DimensionVector, 0,
            util::sequence_size<DimensionVector>::value, cartesian_tag>::apply(segment, result);
    }

    template <typename Point>
    static inline bool duplicate(Point const& previous, Point const& current)
    {
        model::referring_segment<Point const> const segment(previous, current);
        return section_detail::check_duplicate_loop<0, geometry::dimension<Point>::value, true>::apply(segment);
    }

    template <typename Box>
    static inline void expand_box(Box&) {}

    template <std::size_t Dimension, typename Point, typename Box>
    static inline bool preceding(int dir, Point const& point, Box const& point_box, Box const& other_box)
    {
        return section_detail::preceding_check<Dimension, Box, cartesian_tag>
            ::apply(dir, point, point_box, other_box);
    }
};

template <typename Strategy>
inline decltype(auto) get_section(Strategy const& strategy)
{
    auto const hook = [](auto const& s) -> decltype(s.section()) { return s.section(); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
        return hook(strategy);
    else
        return section_with_tolerance{};
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_SECTION_HPP
