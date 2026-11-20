// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_RECORDS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_RECORDS_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <boost/assert.hpp>
#include <boost/mp11.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range.hpp>
#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/helper_geometry.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/strategies/cartesian/exact/construction_rounding.hpp>
#include <boost/geometry/strategies/cartesian/exact/construction_capacity.hpp>
#include <boost/geometry/strategies/cartesian/exact/iterator_point_view.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_traits.hpp>
#include <boost/geometry/strategies/cartesian/exact/sterbenz.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root> class borrowed_point;
template <typename Root> struct borrowed_record;
template <typename Root> class borrowed_context;
template <typename Root> struct borrowed_stage;
template <typename Root> struct borrowed_geometry_types;

template <typename Root, typename A, typename B> bool root_equal(A const&, B const&);
template <typename Root, typename A, typename B, typename P> int root_orientation(A const&, B const&, P const&);
template <typename Root, typename A, typename B, typename P, typename Certified>
int root_orientation(A const&, B const&, P const&, Certified);
template <std::size_t D, typename Root, typename A, typename B> int root_compare_coordinate(A const&, B const&);
struct borrowed_segment_position_tag
{
    using no_fraction = void;
    template <typename A, typename B, typename P, typename Q>
    static int compare(A const& a, B const& b, P const& p, Q const& q)
    {
        using root = typename P::root_type;
        int const x = root_compare_coordinate<0, root>(b, a);
        return x != 0 ? x * root_compare_coordinate<0, root>(p, q)
            : root_compare_coordinate<1, root>(b, a) * root_compare_coordinate<1, root>(p, q);
    }
    template <typename P, typename Q>
    static bool equal(P const& p, Q const& q) { return root_equal<typename P::root_type>(p, q); }
};

struct original_edge
{
    std::size_t range = (std::numeric_limits<std::size_t>::max)();
    std::size_t segment = 0;
    bool valid() const { return range != (std::numeric_limits<std::size_t>::max)(); }
    bool operator==(original_edge const& other) const
    { return range == other.range && segment == other.segment; }
};

namespace borrowed_detail
{
template <typename G, typename Tag = geometry::tag_t<G>>
struct geometries { using type = boost::mp11::mp_list<G>; };

template <typename Sequence> struct geometry_sequence;
template <typename... G>
struct geometry_sequence<geometry::util::type_sequence<G...>>
{ using type = boost::mp11::mp_append<typename geometries<G>::type...>; };

template <typename G> struct geometries<G, dynamic_geometry_tag>
    : geometry_sequence<typename geometry::traits::geometry_types<G>::type> {};
template <typename G> struct geometries<G, geometry_collection_tag>
    : geometry_sequence<typename geometry::traits::geometry_types<typename boost::range_value<G>::type>::type> {};
template <typename G> struct geometries<G, multi_polygon_tag>
{ using type = boost::mp11::mp_append<boost::mp11::mp_list<G>,
    typename geometries<typename boost::range_value<G>::type>::type>; };
template <typename G> struct geometries<G, multi_linestring_tag>
    : geometries<G, multi_polygon_tag> {};
template <typename G> struct geometries<G, polygon_tag>
{
    using outer = std::remove_cv_t<std::remove_reference_t<decltype(
        geometry::exterior_ring(std::declval<G const&>()))>>;
    using inner = std::remove_cv_t<typename boost::range_value<std::remove_reference_t<decltype(
        geometry::interior_rings(std::declval<G const&>()))>>::type>;
    using type = boost::mp11::mp_list<G, outer, inner>;
};

template <typename G> using point_pointer = geometry::point_type_t<G> const*;
template <typename G> using geometry_pointer = G const*;
template <typename G> using is_range = std::integral_constant<bool,
    std::is_same<geometry::tag_t<G>, ring_tag>::value
    || std::is_same<geometry::tag_t<G>, linestring_tag>::value
    || std::is_same<geometry::tag_t<G>, multi_point_tag>::value>;
template <typename G> using is_boundary = std::integral_constant<bool,
    std::is_same<geometry::tag_t<G>, ring_tag>::value
    || std::is_same<geometry::tag_t<G>, linestring_tag>::value
    || std::is_same<geometry::tag_t<G>, box_tag>::value
    || std::is_same<geometry::tag_t<G>, segment_tag>::value>;
template <typename G> using is_indexed = std::integral_constant<bool,
    std::is_same<geometry::tag_t<G>, box_tag>::value
    || std::is_same<geometry::tag_t<G>, segment_tag>::value>;
template <typename G> using has_segments = std::integral_constant<bool,
    !std::is_same<geometry::tag_t<G>, point_tag>::value
    && !std::is_same<geometry::tag_t<G>, multi_point_tag>::value>;
template <typename G> using working_source = std::integral_constant<bool,
    has_segments<G>::value && !std::is_same<geometry::tag_t<G>, box_tag>::value>;
template <typename G> using iterator_access = iterator_point_view<
    typename boost::range_iterator<G const>::type>;

template <typename G>
struct indexed_vertex
{
    G const* geometry;
    std::size_t index;
};

template <std::size_t D, typename G>
inline auto coordinate(indexed_vertex<G> const& point)
{
    if constexpr (std::is_same<geometry::tag_t<G>, segment_tag>::value)
        return point.index == 0 ? geometry::get<0, D>(*point.geometry)
                                : geometry::get<1, D>(*point.geometry);
    else
    {
        // Raw box boundary: lower-left, upper-left, upper-right, lower-right.
        bool const upper = D == 0 ? point.index >= 2 : point.index == 1 || point.index == 2;
        return upper ? geometry::get<max_corner, D>(*point.geometry)
                     : geometry::get<min_corner, D>(*point.geometry);
    }
}
template <std::size_t D, typename P>
inline auto coordinate(P const* point) { BOOST_ASSERT(point); return geometry::get<D>(*point); }
template <std::size_t D, typename I>
inline auto coordinate(iterator_point_view<I> const& point) { return geometry::get<D>(point); }

template <typename G>
std::size_t range_size(G const& geometry)
{
    using tag = geometry::tag_t<G>;
    if constexpr (std::is_same<tag, box_tag>::value) return 4;
    else if constexpr (std::is_same<tag, segment_tag>::value) return 2;
    else if constexpr (std::is_same<tag, ring_tag>::value)
    {
        auto const size = boost::size(geometry);
        return size == 0 ? 0 : size - (geometry::closure<G>::value == closed ? 1 : 0);
    }
    else return boost::size(geometry);
}

template <typename G, typename Apply>
decltype(auto) selected_range(G const& geometry, segment_identifier const& id, Apply&& apply)
{
    using tag = geometry::tag_t<G>;
    if constexpr (std::is_same<tag, polygon_tag>::value)
    {
        if (id.ring_index < 0) return apply(geometry::exterior_ring(geometry));
        auto it = boost::begin(geometry::interior_rings(geometry));
        std::advance(it, id.ring_index);
        return apply(*it);
    }
    else if constexpr (std::is_same<tag, multi_polygon_tag>::value
                       || std::is_same<tag, multi_linestring_tag>::value)
    {
        auto it = boost::begin(geometry);
        std::advance(it, id.multi_index);
        return selected_range(*it, id, std::forward<Apply>(apply));
    }
    else return apply(geometry);
}

template <typename G>
std::size_t source_vertex_index(G const& geometry, std::size_t index, bool reverse)
{
    auto const size = range_size(geometry);
    BOOST_ASSERT(size != 0);
    if constexpr (std::is_same<geometry::tag_t<G>, ring_tag>::value
                  || std::is_same<geometry::tag_t<G>, box_tag>::value)
        return reverse ? (size - index % size) % size : index % size;
    else
    {
        BOOST_ASSERT(index < size);
        return reverse ? size - index - 1 : index;
    }
}

template <typename G>
std::size_t source_edge_index(G const& geometry, std::size_t index, bool reverse)
{
    if (!reverse) return index % range_size(geometry);
    return source_vertex_index(geometry, index + 1, true);
}
} // namespace borrowed_detail

template <typename C, typename Policy, typename G1, typename G2>
struct borrowed_root
{
    using coordinate_type = C;
    using output_policy = Policy;
    using native_geometries = boost::mp11::mp_unique<boost::mp11::mp_append<
        typename borrowed_detail::geometries<G1>::type,
        typename borrowed_detail::geometries<G2>::type>>;
    template <typename G> using admissible = std::integral_constant<bool,
        geometry::dimension<G>::value == 2
        && std::is_same<geometry::coordinate_system_t<G>, cs::cartesian>::value
        && fixed_native_supported<geometry::coordinate_type_t<G>>::value
        && lossless_coordinate_conversion<geometry::coordinate_type_t<G>, C>::value>;
    static_assert(boost::mp11::mp_all_of<native_geometries, admissible>::value,
        "Exact roots require losslessly promotable native two-dimensional Cartesian inputs");
    using native_ranges = boost::mp11::mp_copy_if<native_geometries, borrowed_detail::is_range>;
    using native_boundaries = boost::mp11::mp_copy_if<native_geometries, borrowed_detail::is_boundary>;
    using indexed_geometries = boost::mp11::mp_copy_if<native_geometries, borrowed_detail::is_indexed>;
    using native_access = boost::mp11::mp_unique<boost::mp11::mp_append<
        boost::mp11::mp_transform<borrowed_detail::point_pointer, native_geometries>,
        boost::mp11::mp_transform<borrowed_detail::iterator_access, native_ranges>,
        boost::mp11::mp_transform<borrowed_detail::indexed_vertex, indexed_geometries>>>;
    using point_type = borrowed_point<borrowed_root>;
    using operation_type = geometry::detail::overlay::turn_operation<point_type, borrowed_segment_position_tag>;
    using numeric_state = construction_rounding::point_state<C, Policy>;
};

template <typename Root>
struct borrowed_record
{
    using numeric_state = typename Root::numeric_state;
    numeric_state numeric;
    borrowed_stage<Root>* stage = nullptr;
    std::array<typename Root::operation_type const*, 2> operations{{nullptr, nullptr}};
    bool is_seed = false;
};

template <typename Root>
class borrowed_point
{
public:
    using root_type = Root;
    using coordinate_type = typename Root::coordinate_type;
    using turn_record_type = borrowed_record<Root>;
    using variant_type = boost::mp11::mp_rename<boost::mp11::mp_append<typename Root::native_access,
        boost::mp11::mp_list<turn_record_type*>>, std::variant>;

    borrowed_point() = default;
    explicit borrowed_point(turn_record_type& record) : m_value(&record) {}

    template <typename Source>
    static borrowed_point borrow(Source const& source)
    {
        if constexpr (std::is_same<std::decay_t<Source>, borrowed_point>::value) return source;
        else
        {
            borrowed_point result;
            if constexpr (boost::mp11::mp_contains<typename Root::native_access, Source const*>::value)
                result.m_value = &source;
            else
            {
                static_assert(boost::mp11::mp_contains<typename Root::native_access, Source>::value,
                    "Borrow an original input accessor, not a temporary coordinate snapshot");
                result.m_value = source;
            }
            return result;
        }
    }

    template <typename Iterator>
    static borrowed_point from_iterator(Iterator const& iterator)
    {
        using value = typename std::iterator_traits<Iterator>::value_type;
        if constexpr (std::is_same<value, borrowed_point>::value) return *iterator;
        else return borrow(iterator_point_view<Iterator>(iterator));
    }

    template <bool Reverse, typename Geometry, typename Identifier>
    static void copy_source_vertex(Geometry const& geometry, Identifier const& id,
                                   signed_size_type offset, borrowed_point& output)
    {
        borrowed_detail::selected_range(geometry, id, [&](auto const& range)
        {
            auto index = id.segment_index + offset;
            using tag = geometry::tag_t<std::decay_t<decltype(range)>>;
            if constexpr (std::is_same<tag, ring_tag>::value || std::is_same<tag, box_tag>::value)
            {
                auto const size = signed_size_type(borrowed_detail::range_size(range));
                index = (index % size + size) % size;
            }
            auto const raw_index = borrowed_detail::source_vertex_index(range,
                std::size_t(index), Reverse);
            output = borrowed_context<Root>::native_vertex(range, raw_index);
        });
    }

    bool is_native() const { return !std::holds_alternative<turn_record_type*>(m_value); }
    turn_record_type& record() const
    {
        auto* value = std::get<turn_record_type*>(m_value);
        BOOST_ASSERT(value);
        return *value;
    }

    template <typename Apply>
    decltype(auto) visit(Apply&& apply) const
    {
        return std::visit([&](auto const& value) -> decltype(auto)
        {
            if constexpr (std::is_same<std::decay_t<decltype(value)>, turn_record_type*>::value)
                return apply(*this);
            else if constexpr (std::is_pointer<std::decay_t<decltype(value)>>::value)
            {
                BOOST_ASSERT(value);
                return apply(*value);
            }
            else return apply(value);
        }, m_value);
    }

    coordinate_type native_coordinate(std::size_t dimension) const
    {
        BOOST_ASSERT(is_native());
        return std::visit([&](auto const& value) -> coordinate_type
        {
            if constexpr (std::is_same<std::decay_t<decltype(value)>, turn_record_type*>::value)
                std::terminate();
            else
            {
                using input = decltype(borrowed_detail::coordinate<0>(value));
                static_assert(lossless_coordinate_conversion<input, coordinate_type>::value,
                              "Exact input promotion must be lossless");
                return dimension == 0 ? borrowed_detail::coordinate<0>(value)
                                      : borrowed_detail::coordinate<1>(value);
            }
        }, m_value);
    }

    bool native_coordinate(std::size_t dimension, coordinate_type& output) const
    {
        BOOST_ASSERT(dimension < 2);
        if (is_native()) { output = native_coordinate(dimension); return true; }
        auto const& source = record();
        for (auto const* operation : source.operations)
        {
            auto const edge = source.stage->context.support_coordinates(source.stage->support(operation->seg_id));
            if (edge[dimension] == edge[dimension + 2])
            { output = edge[dimension]; return true; }
        }
        return false;
    }

    interval approximation(std::size_t dimension) const
    {
        if (!is_native()) return record().numeric.approximation(dimension);
        double approximate, error;
        forward_error::try_native_coordinate(native_coordinate(dimension), approximate, error);
        return {approximate, error};
    }

    std::array<coordinate_type, 8> producers() const;
    double approximate(std::size_t dimension) const
    { return is_native() ? double(native_coordinate(dimension)) : record().numeric.approximate(dimension); }
    interval refine(std::size_t dimension) const
    { return is_native() ? approximation(dimension) : record().numeric.refine(producers(), dimension); }
    typename native_fixed_traits<coordinate_type>::intersection_type homogeneous() const;
    template <std::size_t N, std::size_t D>
    bool copy_compact_homogeneous(fixed_homogeneous<N, D>& output) const
    {
        if (is_native())
            return try_fixed_input_homogeneous(native_coordinate(0), native_coordinate(1), output);
        return point_cache_detail::native_cache<coordinate_type>::copy_compact(
            record().numeric.cache_handle(), true, producers(), output);
    }
    exact_number exact_coordinate(std::size_t dimension) const;

    template <typename Output>
    Output emit(std::size_t dimension) const
    {
        if (is_native()) return geometry::util::numeric_cast<Output>(native_coordinate(dimension));
        if constexpr (std::is_same<typename Root::output_policy, relate::approximate_output>::value
                      || (is_bounded_ulp_output<typename Root::output_policy>::value
                          && std::is_same<Output, double>::value)
                      || std::is_integral<Output>::value)
            return geometry::util::numeric_cast<Output>(record().numeric.approximate(dimension));
        else return record().numeric.template emit<Output>(dimension, producers());
    }

private:
    variant_type m_value;
};

}}}}}

namespace boost { namespace geometry { namespace traits
{
template <typename R> struct tag<strategies::detail::exact_cartesian::borrowed_point<R>>
{ using type = point_tag; };
template <typename R> struct coordinate_type<strategies::detail::exact_cartesian::borrowed_point<R>>
{ using type = strategies::detail::exact_cartesian::exact_number; };
template <typename R> struct coordinate_system<strategies::detail::exact_cartesian::borrowed_point<R>>
{ using type = cs::cartesian; };
template <typename R> struct dimension<strategies::detail::exact_cartesian::borrowed_point<R>>
    : std::integral_constant<std::size_t, 2> {};
template <typename R, std::size_t D>
struct access<strategies::detail::exact_cartesian::borrowed_point<R>, D>
{
    using point = strategies::detail::exact_cartesian::borrowed_point<R>;
    static auto get(point const& p) { return p.exact_coordinate(D); }
    static void set(point&, strategies::detail::exact_cartesian::exact_number const&)
    { BOOST_ASSERT_MSG(false, "Implicit geometry points are assigned as a whole"); std::terminate(); }
};

template <typename G> struct tag<strategies::detail::exact_cartesian::borrowed_detail::indexed_vertex<G>>
{ using type = point_tag; };
template <typename G> struct coordinate_type<strategies::detail::exact_cartesian::borrowed_detail::indexed_vertex<G>>
    : geometry::coordinate_type<G> {};
template <typename G> struct coordinate_system<strategies::detail::exact_cartesian::borrowed_detail::indexed_vertex<G>>
    : geometry::coordinate_system<G> {};
template <typename G> struct dimension<strategies::detail::exact_cartesian::borrowed_detail::indexed_vertex<G>>
    : std::integral_constant<std::size_t, 2> {};
template <typename G, std::size_t D>
struct access<strategies::detail::exact_cartesian::borrowed_detail::indexed_vertex<G>, D>
{
    static auto get(strategies::detail::exact_cartesian::borrowed_detail::indexed_vertex<G> const& p)
    { return strategies::detail::exact_cartesian::borrowed_detail::coordinate<D>(p); }
};
}}}

namespace boost { namespace geometry { namespace detail_dispatch
{
template <typename Root, typename Units>
struct helper_geometry<strategies::detail::exact_cartesian::borrowed_point<Root>,
    strategies::detail::exact_cartesian::exact_number, Units, point_tag>
{
    using type = strategies::detail::exact_cartesian::borrowed_point<Root>;
};
}}}

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// Geometry storage, not a numerical producer pool. Labels describe the edge
// arriving at each vertex. The first vertex's label is unused in an open path.
template <typename Point, typename Allocator>
class supported_vector : public std::vector<Point, Allocator>
{
    using base = std::vector<Point, Allocator>;
public:
    using typename base::iterator;
    original_edge pending;
    std::vector<original_edge> incoming;
    void push_back(Point const& point) { base::push_back(point); incoming.push_back(pending); }
    void push_back(Point&& point) { base::push_back(std::move(point)); incoming.push_back(pending); }
    void pop_back() { base::pop_back(); incoming.pop_back(); }
    void clear() { base::clear(); incoming.clear(); pending = {}; }
    void resize(std::size_t size) { base::resize(size); incoming.resize(size); }
    iterator erase(iterator first, iterator last)
    {
        auto const a = first - base::begin(), b = last - base::begin();
        incoming.erase(incoming.begin() + a, incoming.begin() + b);
        return base::erase(first, last);
    }
    iterator erase(iterator position) { return erase(position, position + 1); }
};

template <typename Root>
struct borrowed_geometry_types
{
    using point = borrowed_point<Root>;
    using line = model::linestring<point, supported_vector>;
    template <bool CW, bool Closed> using ring = model::ring<point, CW, Closed, supported_vector>;
    template <bool CW, bool Closed> using polygon = model::polygon<point, CW, Closed, supported_vector>;
    template <bool CW, bool Closed> using multi_polygon = model::multi_polygon<polygon<CW, Closed>>;
    using types = boost::mp11::mp_list<point, line, model::multi_point<point>, model::multi_linestring<line>,
        model::box<point>, ring<true, true>, ring<true, false>, ring<false, true>, ring<false, false>,
        polygon<true, true>, polygon<true, false>, polygon<false, true>, polygon<false, false>,
        multi_polygon<true, true>, multi_polygon<true, false>, multi_polygon<false, true>, multi_polygon<false, false>>;
    using sources = boost::mp11::mp_rename<boost::mp11::mp_unique<boost::mp11::mp_append<
        boost::mp11::mp_list<std::monostate>,
        boost::mp11::mp_transform<borrowed_detail::geometry_pointer,
            boost::mp11::mp_copy_if<typename Root::native_geometries, borrowed_detail::has_segments>>,
        boost::mp11::mp_transform<borrowed_detail::geometry_pointer,
            boost::mp11::mp_copy_if<types, borrowed_detail::working_source>>>>, std::variant>;
};

template <typename Root>
struct borrowed_stage
{
    using source_type = typename borrowed_geometry_types<Root>::sources;
    borrowed_context<Root>& context;
    std::array<source_type, 2> sources;
    std::array<bool, 2> reverse{{false, false}};
    std::array<bool, 2> boundary{{false, false}};
    std::array<typename Root::operation_type const*, 2> model{{nullptr, nullptr}};
    bool finalized = false;
    bool native_differences_exact;

    template <typename A, typename B>
    borrowed_stage(borrowed_context<Root>& owner, A const& a, B const& b)
        : context(owner), sources{{source(a, 0), source(b, 0)}}, boundary{{is_boundary(a, 0), is_boundary(b, 0)}}
        , native_differences_exact(owner.certifies_native_differences(sources))
    {}
    borrowed_point<Root> vertex(segment_identifier const& id, std::size_t offset) const;
    original_edge support(segment_identifier const& id) const;

    template <typename G>
    segment_identifier source_id(G const& geometry, segment_identifier id) const
    {
        if (!boundary[id.source_index]) return id;
        using tag = geometry::tag_t<G>;
        if constexpr (std::is_same<tag, polygon_tag>::value)
        { id.ring_index = id.multi_index - 1; id.multi_index = -1; }
        else if constexpr (std::is_same<tag, multi_polygon_tag>::value)
        {
            auto index = id.multi_index;
            id.multi_index = 0;
            for (auto const& polygon : geometry)
            {
                auto const count = boost::size(geometry::interior_rings(polygon)) + 1;
                if (index < signed_size_type(count)) { id.ring_index = index - 1; break; }
                index -= count;
                ++id.multi_index;
            }
        }
        return id;
    }
private:
    template <typename G>
    static auto source(G const& geometry, int) -> decltype(source_type(&geometry.original_geometry()))
    { return source_type(&geometry.original_geometry()); }
    template <typename G>
    static source_type source(G const& geometry, long) { return source_type(&geometry); }
    template <typename G>
    static auto is_boundary(G const& geometry, int) -> decltype(geometry.original_geometry(), bool()) { return true; }
    template <typename G>
    static bool is_boundary(G const&, long) { return false; }
};

template <typename Root>
class borrowed_context
{
    using source_type = typename borrowed_geometry_types<Root>::sources;
    point_cache_scope<typename Root::coordinate_type> m_precision_scope;
    using range_variant = boost::mp11::mp_rename<boost::mp11::mp_push_front<boost::mp11::mp_transform<
        borrowed_detail::geometry_pointer, typename Root::native_boundaries>, std::monostate>, std::variant>;
    struct work_owner { virtual ~work_owner() = default; };
    template <typename T> struct owned_work : work_owner
    {
        template <typename... A> explicit owned_work(A&&... args) : value(std::forward<A>(args)...) {}
        T value;
    };
    std::vector<std::unique_ptr<work_owner>> m_work;
    std::vector<range_variant> m_ranges;
    std::array<source_type, 2> m_sterbenz_sources;
public:
    borrowed_stage<Root>* active_stage = nullptr;

    borrowed_context() = default;

    template <typename A, typename B>
    borrowed_context(A const& a, B const& b)
    {
        if constexpr (std::is_same<typename Root::coordinate_type, double>::value
                      && sterbenz::source<A> && sterbenz::source<B>)
        {
            if (sterbenz::certify_sources(a, b))
                m_sterbenz_sources = {{std::addressof(a), std::addressof(b)}};
        }
    }

    bool certifies_native_differences(std::array<source_type, 2> const& sources) const
    {
        if constexpr (std::is_same<typename Root::coordinate_type, double>::value)
        {
            // Bound strategies can run later stages against other geometries.
            // Accept only the original, immutable sources, in either order.
            auto original = [&](source_type const& source)
            { return source == m_sterbenz_sources[0] || source == m_sterbenz_sources[1]; };
            return m_sterbenz_sources[0].index() != 0 && original(sources[0]) && original(sources[1]);
        }
        else return false;
    }

    template <typename T, typename... A>
    T& retain(A&&... args)
    {
        auto owner = std::make_unique<owned_work<T>>(std::forward<A>(args)...);
        auto& result = owner->value;
        m_work.push_back(std::move(owner));
        return result;
    }

    template <typename Range>
    std::size_t register_range(Range const& range)
    {
        for (std::size_t i = 0; i != m_ranges.size(); ++i)
            if (auto ptr = std::get_if<Range const*>(&m_ranges[i]))
                if (*ptr == &range) return i;
        m_ranges.emplace_back(&range);
        return m_ranges.size() - 1;
    }

    template <typename G>
    static borrowed_point<Root> native_vertex(G const& range, std::size_t index)
    {
        if constexpr (borrowed_detail::is_indexed<G>::value)
            return borrowed_point<Root>::borrow(borrowed_detail::indexed_vertex<G>{&range, index});
        else
        {
            auto iterator = boost::begin(range);
            std::advance(iterator, index);
            return borrowed_point<Root>::from_iterator(iterator);
        }
    }

    std::array<typename Root::coordinate_type, 4> support_coordinates(original_edge const& edge) const
    {
        BOOST_ASSERT(edge.valid() && edge.range < m_ranges.size());
        return std::visit([&](auto const& source) -> std::array<typename Root::coordinate_type, 4>
        {
            if constexpr (std::is_same<std::decay_t<decltype(source)>, std::monostate>::value)
                std::terminate();
            else
            {
            auto const* range = source;
            auto const size = borrowed_detail::range_size(*range);
            auto const first = native_vertex(*range, edge.segment);
            std::size_t const next = borrowed_detail::is_indexed<std::decay_t<decltype(*range)>>::value
                && std::is_same<geometry::tag_t<std::decay_t<decltype(*range)>>, segment_tag>::value
                ? edge.segment + 1 : (edge.segment + 1) % size;
            auto const second = native_vertex(*range, next);
            return std::array<typename Root::coordinate_type, 4>{{first.native_coordinate(0), first.native_coordinate(1),
                second.native_coordinate(0), second.native_coordinate(1)}};
            }
        }, m_ranges[edge.range]);
    }
};

template <typename Root>
borrowed_point<Root> borrowed_stage<Root>::vertex(segment_identifier const& id, std::size_t offset) const
{
    BOOST_ASSERT(!finalized && id.source_index >= 0 && id.source_index < 2);
    return std::visit([&](auto const& source) -> borrowed_point<Root>
    {
        using source_type = std::decay_t<decltype(source)>;
        if constexpr (std::is_same<source_type, std::monostate>::value) std::terminate();
        else return borrowed_detail::selected_range(*source, source_id(*source, id), [&](auto const& range)
        {
            auto const index = borrowed_detail::source_vertex_index(range,
                std::size_t(id.segment_index) + offset, reverse[id.source_index]);
            using point = geometry::point_type_t<std::decay_t<decltype(range)>>;
            if constexpr (std::is_same<point, borrowed_point<Root>>::value)
            {
                auto iterator = boost::begin(range);
                std::advance(iterator, index);
                return *iterator;
            }
            else return context.native_vertex(range, index);
        });
    }, sources[id.source_index]);
}

template <typename Root>
original_edge borrowed_stage<Root>::support(segment_identifier const& id) const
{
    if (finalized) return {std::size_t(id.source_index), std::size_t(id.segment_index)};
    BOOST_ASSERT(id.source_index >= 0 && id.source_index < 2);
    return std::visit([&](auto const& source) -> original_edge
    {
        using source_type = std::decay_t<decltype(source)>;
        if constexpr (std::is_same<source_type, std::monostate>::value) std::terminate();
        else return borrowed_detail::selected_range(*source, source_id(*source, id), [&](auto const& range) -> original_edge
        {
            auto const index = borrowed_detail::source_edge_index(range, id.segment_index, reverse[id.source_index]);
            using point = geometry::point_type_t<std::decay_t<decltype(range)>>;
            if constexpr (std::is_same<point, borrowed_point<Root>>::value)
            {
                auto next = (index + 1) % borrowed_detail::range_size(range);
                if constexpr (std::is_same<geometry::tag_t<std::decay_t<decltype(range)>>, ring_tag>::value
                              && geometry::closure<std::decay_t<decltype(range)>>::value == closed)
                    if (next == 0) next = borrowed_detail::range_size(range);
                BOOST_ASSERT(next < range.incoming.size() && range.incoming[next].valid());
                return range.incoming[next];
            }
            else return {context.register_range(range), index};
        });
    }, sources[id.source_index]);
}

template <typename Root>
std::array<typename borrowed_point<Root>::coordinate_type, 8> borrowed_point<Root>::producers() const
{
    auto const& value = record();
    BOOST_ASSERT(value.stage && value.operations[0] && value.operations[1]);
    auto const a = value.stage->context.support_coordinates(value.stage->support(value.operations[0]->seg_id));
    auto const b = value.stage->context.support_coordinates(value.stage->support(value.operations[1]->seg_id));
    return {{a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3]}};
}

template <typename Root>
typename native_fixed_traits<typename borrowed_point<Root>::coordinate_type>::intersection_type
borrowed_point<Root>::homogeneous() const
{
    using result_type = typename native_fixed_traits<coordinate_type>::intersection_type;
    if (is_native())
    {
        result_type result;
        auto const native = fixed_input_homogeneous(native_coordinate(0), native_coordinate(1));
        bool const fits = try_fixed_narrow(native, result);
        BOOST_ASSERT(fits);
        return result;
    }
    return point_cache_detail::native_cache<coordinate_type>::get(record().numeric.cache_handle(), true, producers());
}

template <typename Root>
exact_number borrowed_point<Root>::exact_coordinate(std::size_t dimension) const
{
    auto const value = homogeneous();
    exact_number result(dimension == 0 ? value.X : value.Y);
    result /= value.W;
    return result;
}

}}}}}

#endif
