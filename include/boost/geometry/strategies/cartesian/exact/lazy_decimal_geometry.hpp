// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_LAZY_DECIMAL_GEOMETRY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_LAZY_DECIMAL_GEOMETRY_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/geometries/adapted/boost_variant.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/helper_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/lazy_decimal_scalar.hpp>
#include <boost/geometry/util/exact_coordinate.hpp>
#include <boost/geometry/util/tuples.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace lazy_decimal_detail
{

using scalar = lazy_decimal_scalar;
using point = model::point<scalar, 2, cs::cartesian>;

// Preserve the source tag, ring orientation and closure for native BG dispatch.
template <typename G, typename Tag = tag_t<G>>
struct working : helper_geometry<G, scalar> {};

template <typename G>
struct working<G, segment_tag> { using type = model::segment<point>; };

template <typename G>
struct working<G, polygon_tag>
{
    using type = model::polygon<point, point_order<G>::value == clockwise, closure<G>::value == closed>;
};

template <typename G>
struct working<G, multi_polygon_tag>
{
    using type = model::multi_polygon<typename working<typename boost::range_value<G>::type>::type>;
};

template <typename G>
struct working<G, multi_point_tag> { using type = model::multi_point<point>; };

template <typename G>
struct working<G, multi_linestring_tag> { using type = model::multi_linestring<model::linestring<point>>; };

template <typename Sequence> struct working_variant;

template <typename... G>
struct working_variant<util::type_sequence<G...>>
{
    using type = boost::variant<typename working<G>::type...>;
};

template <typename G>
struct working<G, geometry_collection_tag>
{
    using type = model::geometry_collection<typename working_variant<typename traits::geometry_types<G>::type>::type>;
};

template <typename G>
struct working<G, void> { using type = std::vector<typename working<typename boost::range_value<G>::type>::type>; };

template <typename Tuple, std::size_t... I>
auto working_tuple(std::index_sequence<I...>)
    -> std::tuple<typename working<typename tuples::element<I, Tuple>::type>::type...>;

template <typename G, bool IsTuple = tuples::is_tuple<G>::value>
struct working_output : working<G> {};

template <typename G>
struct working_output<G, true>
{
    using type = decltype(working_tuple<G>(std::make_index_sequence<tuples::size<G>::value>()));
};

struct input_point
{
    template <typename S, typename Conversion>
    static scalar leaf(exact_coordinate<S, Conversion> const& x)
    {
        // No materialization for an exactly representable ordinate. Otherwise
        // own the authoritative number: BG accessors may return temporaries.
        if (x.binary64_is_exact()) return x.binary64();
        return scalar::from_source(arena::current().own(x.rational()), x.binary64());
    }

    template <typename P>
    static bool apply(P const& p, point& out)
    {
        set<0>(out, leaf(get<0>(p)));
        set<1>(out, leaf(get<1>(p)));
        return true;
    }
};

template <typename G>
auto operand(G const& g)
{
    static_assert(dimension<G>::value == 2 && std::is_same<cs_tag_t<G>, cartesian_tag>::value,
                  "Lazy decimal strategies require two-dimensional Cartesian geometries");
    if constexpr (util::is_geometry_collection<G>::value)
    {
        typename working<G>::type result;
        for (auto it = boost::begin(g); it != boost::end(g); ++it)
            traits::iter_visit<G>::apply([&](auto const& part) { result.emplace_back(operand(part)); }, it);
        return result;
    }
    else
    {
        typename working<G>::type result;
        if constexpr (util::is_box<G>::value)
            geometry::detail::transform::transform_box_or_segment::apply(g, result, input_point());
        else
            geometry::transform(g, result, input_point());
        return result;
    }
}

struct output_point
{
    template <typename T>
    static T ordinate(scalar x)
    {
        if constexpr (std::is_same<T, float>::value || std::is_same<T, double>::value)
            return rational_rounding::round<T>(x.exact_value()).value;
        else
        {
            static_assert(std::is_same<T, rational>::value || std::is_same<T, exact_coordinate<rational>>::value,
                          "Lazy decimal output requires cpp_rational, exact_coordinate<cpp_rational>, float or double");
            return T(x.exact_value());
        }
    }

    template <typename P>
    static bool apply(point const& p, P& out)
    {
        static_assert(dimension<P>::value == 2 && std::is_same<cs_tag_t<P>, cartesian_tag>::value,
                      "Lazy decimal output requires a two-dimensional Cartesian geometry");
        using target = coordinate_type_t<P>;
        set<0>(out, ordinate<target>(get<0>(p)));
        set<1>(out, ordinate<target>(get<1>(p)));
        return true;
    }
};

template <typename Src, typename Dst>
void append(Src const& src, Dst& dst);

template <typename Source>
struct matching_working
{
    template <typename G> struct apply : std::is_same<typename working<G>::type, Source> {};
};

template <typename Src, typename Dst, std::size_t... I>
void append_tuple(Src const& src, Dst& dst, std::index_sequence<I...>)
{
    (append(tuples::get<I>(src), tuples::get<I>(dst)), ...);
}

template <typename Src, typename Dst>
void append(Src const& src, Dst& dst)
{
    if constexpr (tuples::is_tuple<Dst>::value)
        append_tuple(src, dst, std::make_index_sequence<tuples::size<Dst>::value>());
    else if constexpr (std::is_same<tag_t<Dst>, box_tag>::value)
        geometry::detail::transform::transform_box_or_segment::apply(src, dst, output_point());
    else if constexpr (util::is_geometry_collection<Dst>::value)
    {
        for (auto const& member : src)
        {
            traits::visit<typename boost::range_value<Src>::type>::apply([&](auto const& g)
            {
                using match = matching_working<std::decay_t<decltype(g)>>;
                typename util::sequence_find_if<typename traits::geometry_types<Dst>::type,
                    match::template apply>::type result;
                geometry::transform(g, result, output_point());
                traits::push_back<Dst>::apply(dst, std::move(result));
            }, member);
        }
    }
    else
    {
        for (auto const& g : src)
        {
            typename boost::range_value<Dst>::type result;
            geometry::transform(g, result, output_point());
            traits::push_back<Dst>::apply(dst, std::move(result));
        }
    }
}

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::lazy_decimal_detail

#endif
