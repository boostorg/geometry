// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_GEOMETRY_CONVERSION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_GEOMETRY_CONVERSION_HPP

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/visit.hpp>
#include <boost/geometry/geometries/adapted/boost_variant.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/geometry_collection.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_box.hpp>
#include <boost/geometry/util/sequence.hpp>
#include <boost/geometry/util/tuples.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Coordinate, typename Geometry, typename Tag = geometry::tag_t<Geometry>>
struct working_geometry { using type = Geometry; };

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, linestring_tag>
{
    using type = model::linestring<exact_point<Coordinate>>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, point_tag>
{
    using type = exact_point<Coordinate>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, box_tag>
{
    using type = model::box<exact_point<Coordinate>>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, ring_tag>
{
    using type = model::ring<exact_point<Coordinate>,
        geometry::point_order<Geometry>::value == clockwise,
        geometry::closure<Geometry>::value == closed>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, polygon_tag>
{
    using type = model::polygon<exact_point<Coordinate>,
        geometry::point_order<Geometry>::value == clockwise,
        geometry::closure<Geometry>::value == closed>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, multi_point_tag>
{
    using type = model::multi_point<exact_point<Coordinate>>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, multi_linestring_tag>
{
    using type = model::multi_linestring<model::linestring<exact_point<Coordinate>>>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, multi_polygon_tag>
{
    using type = model::multi_polygon<typename working_geometry<Coordinate,
        typename boost::range_value<Geometry>::type>::type>;
};

template <typename Coordinate, typename Sequence>
struct working_dynamic;

template <typename Coordinate, typename... Geometry>
struct working_dynamic<Coordinate, util::type_sequence<Geometry...>>
{
    using type = boost::variant<typename working_geometry<Coordinate, Geometry>::type...>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, dynamic_geometry_tag>
    : working_dynamic<Coordinate, typename traits::geometry_types<Geometry>::type>
{};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, geometry_collection_tag>
{
    using type = model::geometry_collection<typename working_dynamic<Coordinate,
        typename traits::geometry_types<Geometry>::type>::type>;
};

template <typename Coordinate, typename Geometry>
struct working_geometry<Coordinate, Geometry, void>
{
    using type = std::vector<typename working_geometry<Coordinate,
        typename boost::range_value<Geometry>::type>::type>;
};

template <typename Coordinate, typename... Geometry>
struct working_geometry<Coordinate, std::tuple<Geometry...>, void>
{
    using type = std::tuple<typename working_geometry<Coordinate, Geometry>::type...>;
};

template <typename Coordinate, typename First, typename Second>
struct working_geometry<Coordinate, std::pair<First, Second>, void>
{
    using type = std::pair<typename working_geometry<Coordinate, First>::type,
                           typename working_geometry<Coordinate, Second>::type>;
};

template <typename Coordinate>
struct working_geometry<Coordinate, boost::tuples::null_type, void>
{
    using type = boost::tuples::null_type;
};

template <typename Coordinate, typename Head, typename Tail>
struct working_geometry<Coordinate, boost::tuples::cons<Head, Tail>, void>
{
    using type = boost::tuples::cons<typename working_geometry<Coordinate, Head>::type,
                                    typename working_geometry<Coordinate, Tail>::type>;
};

template <typename Coordinate, typename... Geometry>
struct working_geometry<Coordinate, boost::tuples::tuple<Geometry...>, void>
{
    using type = boost::tuples::tuple<typename working_geometry<Coordinate, Geometry>::type...>;
};

template <typename Coordinate, typename Collection, bool IsTuple = tuples::is_tuple<Collection>::value>
struct working_output : working_geometry<Coordinate, Collection> {};

template <typename Coordinate, typename Collection>
struct working_output<Coordinate, Collection, true>
{
    template <std::size_t... I>
    static auto type_of(std::index_sequence<I...>)
        -> std::tuple<typename working_output<Coordinate,
            typename tuples::element<I, Collection>::type>::type...>;
    using type = decltype(type_of(std::make_index_sequence<tuples::size<Collection>::value>()));
};

template <typename Coordinate, typename Tag, typename OutputPolicy = strategies::relate::approximate_output>
struct geometry_converter
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<!util::is_polygon<Destination>::value>* = nullptr)
    {
        geometry::detail::conversion::range_to_range::apply(source, destination,
                                                           point_converter<Coordinate, OutputPolicy>());
    }

    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<util::is_polygon<Destination>::value>* = nullptr)
    {
        geometry::clear(destination);
        geometry::detail::conversion::range_to_range::apply(source, exterior_ring(destination),
                                                           point_converter<Coordinate, OutputPolicy>());
    }
};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, point_tag, OutputPolicy> : point_converter<Coordinate, OutputPolicy> {};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, ring_tag, OutputPolicy>
{
    template <typename FirstIterator, typename Destination>
    struct ring_point_converter
    {
        Destination* ring;
        FirstIterator first;
        std::size_t closure_index;
        std::size_t index = 0;

        template <typename SourcePoint, typename OutputPoint>
        void apply(SourcePoint const& source, OutputPoint& destination)
        {
            // Closing the ring reuses its first emitted vertex, even if the
            // source's closing occurrence has been refined independently.
            if (index == 0)
            {
                point_converter<Coordinate, OutputPolicy>::apply(*first, destination);
            }
            else if (index == closure_index)
            {
                destination = *boost::begin(*ring);
            }
            else
            {
                point_converter<Coordinate, OutputPolicy>::apply(source, destination);
            }
            ++index;
        }
    };

    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<!util::is_polygon<Destination>::value>* = nullptr)
    {
        if constexpr (static_predicates_detail::exact_input<geometry::point_type_t<Destination>>::value)
        {
            geometry::detail::conversion::range_to_range::apply(source, destination,
                point_converter<Coordinate, OutputPolicy>());
            return;
        }
        auto const source_first = boost::begin(source);
        if (source_first == boost::end(source))
        {
            geometry::clear(destination);
            return;
        }
        using policy = ring_point_converter<decltype(source_first), Destination>;
        geometry::detail::conversion::range_to_range::apply(source, destination,
            policy{&destination, source_first,
                boost::size(source) - (geometry::closure<Source>::value == closed ? 1 : 0)});
    }

    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<util::is_polygon<Destination>::value>* = nullptr)
    {
        geometry::clear(destination);
        apply(source, exterior_ring(destination));
    }
};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, polygon_tag, OutputPolicy>
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<util::is_polygon<Destination>::value>* = nullptr)
    {
        geometry_converter<Coordinate, ring_tag, OutputPolicy>::apply(exterior_ring(source), exterior_ring(destination));
        auto const& src = interior_rings(source);
        auto&& dst = interior_rings(destination);
        using rings_type = std::remove_reference_t<decltype(dst)>;
        traits::resize<rings_type>::apply(dst, boost::size(src));
        auto output = boost::begin(dst);
        for (auto const& ring : src)
        {
            geometry_converter<Coordinate, ring_tag, OutputPolicy>::apply(ring, *output++);
        }
    }

    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<util::is_ring<Destination>::value>* = nullptr)
    {
        geometry_converter<Coordinate, ring_tag, OutputPolicy>::apply(exterior_ring(source), destination);
    }
};

template <typename Coordinate, typename OutputPolicy>
struct multi_geometry_converter
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination)
    {
        traits::resize<Destination>::apply(destination, boost::size(source));
        auto output = boost::begin(destination);
        for (auto const& item : source)
        {
            geometry_converter<Coordinate, geometry::tag_t<decltype(item)>, OutputPolicy>::apply(item, *output++);
        }
    }
};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, multi_point_tag, OutputPolicy> : multi_geometry_converter<Coordinate, OutputPolicy> {};
template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, multi_linestring_tag, OutputPolicy> : multi_geometry_converter<Coordinate, OutputPolicy> {};
template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, multi_polygon_tag, OutputPolicy> : multi_geometry_converter<Coordinate, OutputPolicy> {};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, geometry_collection_tag, OutputPolicy> : multi_geometry_converter<Coordinate, OutputPolicy> {};

template <typename Geometry>
struct matching_geometry_tag
{
    template <typename Candidate>
    struct apply : std::is_same<geometry::tag_t<Geometry>, geometry::tag_t<Candidate>> {};
};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, dynamic_geometry_tag, OutputPolicy>
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination)
    {
        traits::visit<Source>::apply([&](auto const& geometry)
        {
            using source_type = util::remove_cref_t<decltype(geometry)>;
            using destination_type = typename util::sequence_find_if
                <typename traits::geometry_types<Destination>::type,
                 matching_geometry_tag<source_type>::template apply>::type;
            static_assert(!std::is_void<destination_type>::value,
                          "Exact output collection must support the result geometry tag");
            destination_type result;
            geometry_converter<Coordinate, geometry::tag_t<source_type>, OutputPolicy>::apply(geometry, result);
            destination = std::move(result);
        }, source);
    }
};

template <typename Coordinate, typename OutputPolicy>
struct geometry_converter<Coordinate, box_tag, OutputPolicy>
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<util::is_box<Destination>::value>* = nullptr)
    {
        using target = typename geometry::point_type<Destination>::type;
        if constexpr (std::is_same<target, exact_point<Coordinate>>::value)
        {
            static_box_detail::convert_box<Coordinate>(source, destination);
        }
        else
        {
            target first, second;
            point_converter<Coordinate, OutputPolicy>::apply(static_box_detail::borrowed_corner<min_corner>(source), first);
            point_converter<Coordinate, OutputPolicy>::apply(static_box_detail::borrowed_corner<max_corner>(source), second);
            geometry::detail::assign_point_to_index<min_corner>(first, destination);
            geometry::detail::assign_point_to_index<max_corner>(second, destination);
        }
    }

    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination,
        std::enable_if_t<!util::is_box<Destination>::value>* = nullptr)
    {
        convert_boundary(source, destination, geometry::tag_t<Destination>());
    }

private:
    template <typename Source, typename Ring>
    static void convert_boundary(Source const& source, Ring& ring, ring_tag)
    {
        auto const lower = make_point<Coordinate>(static_box_detail::corner<min_corner>(source));
        auto const upper = make_point<Coordinate>(static_box_detail::corner<max_corner>(source));
        using point = exact_point<Coordinate>;
        auto left = point::splice_coordinates(lower, 0, upper, 1);
        auto right = point::splice_coordinates(upper, 0, lower, 1);
        if (geometry::point_order<Ring>::value != clockwise) std::swap(left, right);
        std::array<point, 5> const corners{{lower, left, upper, right, lower}};
        geometry::clear(ring);
        auto const count = geometry::closure<Ring>::value == closed ? 5u : 4u;
        for (unsigned i = 0; i != count; ++i)
        {
            typename boost::range_value<Ring>::type output;
            point_converter<Coordinate, OutputPolicy>::apply(corners[i], output);
            traits::push_back<Ring>::apply(ring, std::move(output));
        }
    }

    template <typename Source, typename Polygon>
    static void convert_boundary(Source const& source, Polygon& polygon, polygon_tag)
    {
        geometry::clear(polygon);
        convert_boundary(source, exterior_ring(polygon), ring_tag());
    }
};

template <typename Coordinate, typename OutputPolicy = strategies::relate::approximate_output,
          typename Source, typename Destination>
inline void append_output(Source const& source, Destination& destination,
    std::enable_if_t<!tuples::is_tuple<Destination>::value && !util::is_box<Destination>::value>* = nullptr)
{
    for (auto const& item : source)
    {
        typename boost::range_value<Destination>::type result;
        geometry_converter<Coordinate, geometry::tag_t<decltype(item)>, OutputPolicy>::apply(item, result);
        traits::push_back<Destination>::apply(destination, std::move(result));
    }
}

template <typename Coordinate, typename OutputPolicy = strategies::relate::approximate_output,
          typename Source, typename Destination>
inline void append_output(Source const& source, Destination& destination,
    std::enable_if_t<util::is_box<Destination>::value>* = nullptr)
{
    geometry_converter<Coordinate, box_tag, OutputPolicy>::apply(source, destination);
}

template <typename Coordinate, typename OutputPolicy, typename Source, typename Destination, std::size_t... I>
inline void append_tuple_output(Source const& source, Destination& destination, std::index_sequence<I...>)
{
    using expand = int[];
    (void)expand{0, (append_output<Coordinate, OutputPolicy>(tuples::get<I>(source), tuples::get<I>(destination)), 0)...};
}

template <typename Coordinate, typename OutputPolicy = strategies::relate::approximate_output,
          typename Source, typename Destination>
inline void append_output(Source const& source, Destination& destination,
    std::enable_if_t<tuples::is_tuple<Destination>::value>* = nullptr)
{
    append_tuple_output<Coordinate, OutputPolicy>(source, destination,
        std::make_index_sequence<tuples::size<Destination>::value>());
}

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
