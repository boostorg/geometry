// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_GEOMETRY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_GEOMETRY_HPP

#include <tuple>
#include <boost/geometry/strategies/cartesian/exact/borrowed_records.hpp>
#include <boost/geometry/strategies/cartesian/exact/point.hpp>
#include <boost/geometry/util/tuples.hpp>
#include <boost/geometry/views/detail/boundary_view/interface.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root, typename G, typename Tag = geometry::tag_t<G>>
struct borrowed_working { using type = G; };
template <typename Root, typename G> struct borrowed_working<Root, G, box_tag>
{
    static_assert(!std::is_same<G, G>::value,
        "Root working boxes are unsupported; retain an original native box or use root_bounds metadata");
};
template <typename Root, typename G> struct borrowed_working<Root, G, point_tag>
{ using type = borrowed_point<Root>; };
template <typename Root, typename G> struct borrowed_working<Root, G, linestring_tag>
{ using type = typename borrowed_geometry_types<Root>::line; };
template <typename Root, typename G> struct borrowed_working<Root, G, ring_tag>
{ using type = typename borrowed_geometry_types<Root>::template ring<
    geometry::point_order<G>::value == clockwise, geometry::closure<G>::value == closed>; };
template <typename Root, typename G> struct borrowed_working<Root, G, polygon_tag>
{ using type = typename borrowed_geometry_types<Root>::template polygon<
    geometry::point_order<G>::value == clockwise, geometry::closure<G>::value == closed>; };
template <typename Root, typename G> struct borrowed_working<Root, G, multi_point_tag>
{ using type = model::multi_point<borrowed_point<Root>>; };
template <typename Root, typename G> struct borrowed_working<Root, G, multi_linestring_tag>
{ using type = model::multi_linestring<typename borrowed_geometry_types<Root>::line>; };
template <typename Root, typename G> struct borrowed_working<Root, G, multi_polygon_tag>
{ using type = model::multi_polygon<typename borrowed_working<Root,
    typename boost::range_value<G>::type>::type>; };

template <typename Root, typename Sequence> struct borrowed_dynamic;
template <typename Root, typename... G>
struct borrowed_dynamic<Root, geometry::util::type_sequence<G...>>
{ using type = boost::variant<typename borrowed_working<Root, G>::type...>; };
template <typename Root, typename G> struct borrowed_working<Root, G, dynamic_geometry_tag>
    : borrowed_dynamic<Root, typename traits::geometry_types<G>::type> {};
template <typename Root, typename G> struct borrowed_working<Root, G, geometry_collection_tag>
{ using type = model::geometry_collection<typename borrowed_dynamic<Root,
    typename traits::geometry_types<G>::type>::type>; };
template <typename Root, typename G> struct borrowed_working<Root, G, void>
{ using type = std::vector<typename borrowed_working<Root, typename boost::range_value<G>::type>::type>; };
template <typename Root, typename... G> struct borrowed_working<Root, std::tuple<G...>, void>
{ using type = std::tuple<typename borrowed_working<Root, G>::type...>; };
template <typename Root, typename A, typename B> struct borrowed_working<Root, std::pair<A, B>, void>
{ using type = std::pair<typename borrowed_working<Root, A>::type, typename borrowed_working<Root, B>::type>; };
template <typename Root> struct borrowed_working<Root, boost::tuples::null_type, void>
{ using type = boost::tuples::null_type; };
template <typename Root, typename H, typename T>
struct borrowed_working<Root, boost::tuples::cons<H, T>, void>
{ using type = boost::tuples::cons<typename borrowed_working<Root, H>::type,
                                 typename borrowed_working<Root, T>::type>; };
template <typename Root, typename... G>
struct borrowed_working<Root, boost::tuples::tuple<G...>, void>
{ using type = boost::tuples::tuple<typename borrowed_working<Root, G>::type...>; };

// A standalone box output must not instantiate the rejected working-box type.
template <typename Root, typename G, bool IsBox = std::is_same<geometry::tag_t<G>, box_tag>::value>
struct borrowed_output_working : borrowed_working<Root, G> {};
template <typename Root, typename G>
struct borrowed_output_working<Root, G, true> { using type = G; };

namespace borrowed_detail
{
template <typename Range>
Range const& conversion_range(Range const& range) { return range; }

template <typename Ring>
Ring const& conversion_range(geometry::detail::boundary_view<Ring> const& view)
{ return view.original_geometry(); }

template <typename G> struct matching_tag
{
    template <typename Candidate> struct apply : std::is_same<geometry::tag_t<G>, geometry::tag_t<Candidate>> {};
};
template <typename T, typename = void> struct is_borrowed_point : std::false_type {};
template <typename T> struct is_borrowed_point<T, std::void_t<typename T::turn_record_type>> : std::true_type {};
template <typename T, typename = void> struct is_supported_range : std::false_type {};
template <typename T> struct is_supported_range<T, std::void_t<decltype(std::declval<T>().incoming)>>
    : std::true_type {};

template <typename Range>
original_edge incoming_at(Range const& range, std::size_t index)
{
    if constexpr (std::is_same<geometry::tag_t<Range>, ring_tag>::value
                  && geometry::closure<Range>::value == closed)
        if (index == 0) index = borrowed_detail::range_size(range);
    BOOST_ASSERT(index < range.incoming.size());
    return range.incoming[index];
}

template <typename Root, typename Geometry>
void validate_supports(borrowed_context<Root>& context, Geometry const& geometry)
{
#ifndef NDEBUG
    using tag = geometry::tag_t<Geometry>;
    if constexpr (is_supported_range<Geometry>::value)
    {
        BOOST_ASSERT(geometry.size() == geometry.incoming.size());
        auto edge = [&](std::size_t first, std::size_t second)
        {
            auto const support = incoming_at(geometry, second);
            if constexpr (std::is_same<tag, linestring_tag>::value)
            {
                // Linear output represents an isolated point by two equal
                // vertices. No input segment was followed between them.
                if (geometry.size() == 2 && !support.valid()
                    && root_equal<Root>(geometry[first], geometry[second]))
                    return;
            }
            BOOST_ASSERT(support.valid());
            auto const native = context.support_coordinates(support);
            using P = model::point<typename Root::coordinate_type, 2, cs::cartesian>;
            P const a(native[0], native[1]), b(native[2], native[3]);
            BOOST_ASSERT(root_orientation<Root>(a, b, geometry[first]) == 0);
            BOOST_ASSERT(root_orientation<Root>(a, b, geometry[second]) == 0);
        };
        for (std::size_t i = 1; i < geometry.size(); ++i) edge(i - 1, i);
        if constexpr (std::is_same<tag, ring_tag>::value && geometry::closure<Geometry>::value == open)
            if (geometry.size() > 1) edge(geometry.size() - 1, 0);
    }
    else if constexpr (std::is_same<tag, polygon_tag>::value)
    {
        validate_supports<Root>(context, geometry::exterior_ring(geometry));
        for (auto const& ring : geometry::interior_rings(geometry)) validate_supports<Root>(context, ring);
    }
    else if constexpr (std::is_same<tag, multi_polygon_tag>::value || std::is_same<tag, multi_linestring_tag>::value)
        for (auto const& member : geometry) validate_supports<Root>(context, member);
#else
    (void)context;
    (void)geometry;
#endif
}
}

template <typename Root>
class borrowed_geometry_converter
{
    borrowed_context<Root>& m_context;
    using point = borrowed_point<Root>;

    template <typename Source, typename Destination>
    static void point_output(Source const& source, Destination& destination)
    {
        using target_coordinate = geometry::coordinate_type_t<Destination>;
        if constexpr (std::is_same<std::decay_t<Destination>, point>::value)
            destination = point::borrow(source);
        else if constexpr (borrowed_detail::is_borrowed_point<std::decay_t<Source>>::value)
        {
            if constexpr (std::is_same<std::decay_t<Destination>, exact_point<typename Root::coordinate_type>>::value)
                destination = Destination::from_homogeneous(source.homogeneous());
            else if constexpr (std::is_same<target_coordinate, exact_number>::value)
            {
                geometry::set<0>(destination, source.exact_coordinate(0));
                geometry::set<1>(destination, source.exact_coordinate(1));
            }
            else
            {
                geometry::set<0>(destination, source.template emit<target_coordinate>(0));
                geometry::set<1>(destination, source.template emit<target_coordinate>(1));
            }
        }
        else geometry::detail::conversion::convert_point_to_point(source, destination);
    }

    template <typename Source, typename Destination>
    void range_output(Source const& source, Destination& destination) const
    {
        auto const& original = borrowed_detail::conversion_range(source);
        using original_type = std::decay_t<decltype(original)>;
        constexpr bool is_boundary = !std::is_same<std::decay_t<Source>, original_type>::value;
        constexpr bool is_ring = std::is_same<geometry::tag_t<Source>, ring_tag>::value;
        constexpr bool source_closed = is_ring && geometry::closure<Source>::value == closed;
        constexpr bool target_closed = is_ring && geometry::closure<Destination>::value == closed;
        constexpr bool reverse = is_ring && geometry::point_order<Source>::value
                                                   != geometry::point_order<Destination>::value;
        using source_point = typename boost::range_value<Source>::type;
        constexpr bool supported = borrowed_detail::is_supported_range<Destination>::value;
        destination.clear();
        auto const size = borrowed_detail::range_size(original);
        if (size == 0) return;
        auto at = [&](std::size_t index)
        {
            if constexpr (supported) return m_context.native_vertex(original, index);
            else
            {
                auto iterator = boost::begin(original);
                std::advance(iterator, index);
                return iterator_point_view<decltype(iterator)>(iterator);
            }
        };
        auto edge = [&](std::size_t index) -> original_edge
        {
            if constexpr (borrowed_detail::is_supported_range<original_type>::value)
                return borrowed_detail::incoming_at(original, (index + 1) % size);
            else return {m_context.register_range(original), index};
        };
        auto source_index = [&](std::size_t i)
        {
            if constexpr (!reverse) return i;
            else if constexpr (source_closed) return (size - i) % size;
            else return size - i - 1;
        };
        auto append = [&](std::size_t index, bool first)
        {
            typename boost::range_value<Destination>::type output;
            if constexpr (supported) output = at(index);
            else
            {
                auto iterator = boost::begin(original);
                std::advance(iterator, index);
                point_output(*iterator, output);
            }
            if constexpr (supported)
            {
                if (is_ring || !first)
                    destination.pending = edge(reverse ? index : (index + size - 1) % size);
                else destination.pending = {};
            }
            destination.push_back(std::move(output));
        };
        for (std::size_t i = 0; i < size; ++i) append(source_index(i), i == 0);
        if constexpr (target_closed || is_boundary)
        {
            if constexpr (supported)
            {
                destination.pending = edge(reverse ? source_index(0) : (source_index(0) + size - 1) % size);
                destination.push_back(destination.front());
            }
            else destination.push_back(destination.front());
        }
        if constexpr (supported)
            if constexpr (is_ring) destination.pending = destination.incoming.front();
        (void)sizeof(source_point);
    }

    template <typename Box, typename Destination>
    void box_ring(Box const& box, Destination& destination) const
    {
        static_assert(std::is_same<typename boost::range_value<Destination>::type, point>::value,
                      "Internal box conversion retains original faces");
        destination.clear();
        auto const range = m_context.register_range(box);
        constexpr bool reverse = geometry::point_order<Destination>::value != clockwise;
        auto index = [](std::size_t i) { return reverse ? (4 - i) % 4 : i; };
        for (std::size_t i = 0; i != 4; ++i)
        {
            auto const j = index(i);
            destination.pending = {range, reverse ? j : (j + 3) % 4};
            destination.push_back(m_context.native_vertex(box, j));
        }
        if constexpr (geometry::closure<Destination>::value == closed)
        {
            destination.pending = destination.incoming.front();
            destination.push_back(destination.front());
        }
    }

    template <typename Source, typename Destination, std::size_t... I>
    void tuple_output(Source const& source, Destination& destination, std::index_sequence<I...>) const
    { (apply(tuples::get<I>(source), tuples::get<I>(destination)), ...); }

public:
    explicit borrowed_geometry_converter(borrowed_context<Root>& context) : m_context(context) {}

    template <typename Source, typename Destination>
    void apply(Source const& source, Destination& destination) const
    {
        using from = geometry::tag_t<Source>;
        using to = geometry::tag_t<Destination>;
        if constexpr (std::is_same<from, point_tag>::value) point_output(source, destination);
        else if constexpr ((std::is_same<from, ring_tag>::value || std::is_same<from, box_tag>::value)
                           && std::is_same<to, polygon_tag>::value)
        {
            destination.clear();
            apply(source, geometry::exterior_ring(destination));
        }
        else if constexpr (std::is_same<from, ring_tag>::value || std::is_same<from, linestring_tag>::value)
            range_output(source, destination);
        else if constexpr (std::is_same<from, box_tag>::value && std::is_same<to, ring_tag>::value)
            box_ring(source, destination);
        else if constexpr (std::is_same<from, polygon_tag>::value && std::is_same<to, polygon_tag>::value)
        {
            destination.clear();
            apply(geometry::exterior_ring(source), geometry::exterior_ring(destination));
            for (auto const& inner : geometry::interior_rings(source))
            {
                geometry::interior_rings(destination).emplace_back();
                apply(inner, geometry::interior_rings(destination).back());
            }
        }
        else if constexpr (std::is_same<from, multi_point_tag>::value)
        {
            destination.clear();
            for (auto iterator = boost::begin(source); iterator != boost::end(source); ++iterator)
            {
                if constexpr (std::is_same<typename boost::range_value<Destination>::type, point>::value)
                    destination.push_back(point::from_iterator(iterator));
                else
                {
                    destination.emplace_back();
                    apply(*iterator, destination.back());
                }
            }
        }
        else if constexpr (std::is_same<from, multi_linestring_tag>::value
                           || std::is_same<from, multi_polygon_tag>::value
                           || (std::is_void<from>::value && !tuples::is_tuple<Source>::value))
        {
            destination.clear();
            for (auto const& geometry : source)
            {
                destination.emplace_back();
                apply(geometry, destination.back());
            }
        }
        else if constexpr (tuples::is_tuple<Source>::value && tuples::is_tuple<Destination>::value)
            tuple_output(source, destination, std::make_index_sequence<tuples::size<Source>::value>());
        else if constexpr (std::is_same<from, dynamic_geometry_tag>::value)
            traits::visit<Source>::apply([&](auto const& geometry)
            {
                using value_type = typename util::sequence_find_if<typename traits::geometry_types<Destination>::type,
                    borrowed_detail::matching_tag<std::decay_t<decltype(geometry)>>::template apply>::type;
                static_assert(!std::is_void<value_type>::value, "Output variant must contain the result geometry tag");
                value_type output;
                apply(geometry, output);
                destination = std::move(output);
            }, source);
        else if constexpr (std::is_same<from, geometry_collection_tag>::value)
        {
            destination.clear();
            for (auto const& geometry : source)
            {
                typename boost::range_value<Destination>::type output;
                apply(geometry, output);
                destination.push_back(std::move(output));
            }
        }
        else geometry::convert(source, destination);
    }
};

}}}}}

namespace boost { namespace geometry { namespace detail { namespace conversion
{
template <typename Source, typename Root>
struct point_to_point<Source, strategies::detail::exact_cartesian::borrowed_point<Root>, 0, 2>
{
    static void apply(Source const& source, strategies::detail::exact_cartesian::borrowed_point<Root>& target)
    { target = strategies::detail::exact_cartesian::borrowed_point<Root>::borrow(source); }
};
}}}}

#endif
