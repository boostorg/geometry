// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_LAZY_DECIMAL_GEOMETRY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_LAZY_DECIMAL_GEOMETRY_HPP

#include <type_traits>
#include <utility>
#include <vector>
#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/geometries/adapted/boost_variant.hpp>
#include <boost/geometry/geometries/detail/rebind_geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/helper_geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/strategies/cartesian/exact/lazy_decimal_scalar.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>
#include <boost/geometry/util/detail/binary_rounding.hpp>
#include <boost/geometry/util/exact_coordinate.hpp>
#include <boost/geometry/util/numeric_cast.hpp>
#include <boost/geometry/util/tuples.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace lazy_decimal_detail
{

using scalar = lazy_decimal_scalar;
using point = model::point<scalar, 2, cs::cartesian>;

struct decimal_working_models : geometry::detail::working_models<point>
{
    struct segment { using type = model::segment<point>; };
    template <typename G> struct other
        : std::conditional_t<std::is_same<tag_t<G>, segment_tag>::value,
            segment, helper_geometry<G, scalar>> {};
};

template <typename G>
using working = geometry::detail::rebind_working<decimal_working_models, G>;

template <typename G>
using working_output = geometry::detail::rebind_output_tuple<decimal_working_models, G>;

// These models expose their stored points, not accessor temporaries. Indexed
// geometries and collection visitors can synthesize points or whole members.
template <typename G> struct borrow_cached_input : std::false_type {};
template <typename C, std::size_t D, typename Cs>
struct borrow_cached_input<model::point<C, D, Cs>> : std::is_same<C, exact_coordinate<rational>> {};
template <typename C, typename Cs>
struct borrow_cached_input<model::d2::point_xy<C, Cs>> : std::is_same<C, exact_coordinate<rational>> {};
template <typename P, bool Clockwise, bool Closed>
struct borrow_cached_input<model::ring<P, Clockwise, Closed>> : borrow_cached_input<P> {};
template <typename P, bool Clockwise, bool Closed>
struct borrow_cached_input<model::polygon<P, Clockwise, Closed>> : borrow_cached_input<P> {};
template <typename P>
struct borrow_cached_input<model::linestring<P>> : borrow_cached_input<P> {};
template <typename P>
struct borrow_cached_input<model::multi_point<P>> : borrow_cached_input<P> {};
template <typename L>
struct borrow_cached_input<model::multi_linestring<L>> : borrow_cached_input<L> {};
template <typename P>
struct borrow_cached_input<model::multi_polygon<P>> : borrow_cached_input<P> {};

template <bool Borrow = false>
struct input_point
{
    template <typename T, std::enable_if_t<fixed_native_supported<T>::value, int> = 0>
    static scalar leaf(T x) { return scalar(x); }

    static scalar leaf(rational const& x)
    {
        auto const converted = geometry::detail::binary_rounding::round<double>(x);
        return converted.exact ? scalar(converted.value)
            : scalar::from_source(arena::current().own(x), converted.value);
    }

    template <typename S, typename Conversion>
    static scalar leaf(exact_coordinate<S, Conversion> const& x)
    {
        // No materialization for an exactly representable ordinate. Otherwise
        // own the authoritative number: BG accessors may return temporaries.
        if (x.binary64_is_exact()) return x.binary64();
        return scalar::from_source(arena::current().own(x.rational()), x.binary64());
    }

    template <std::size_t D, typename P>
    static scalar ordinate(P const& p)
    {
        if constexpr (Borrow)
        {
            auto const& x = p.template get<D>();
            return x.binary64_is_exact() ? scalar(x.binary64())
                : scalar::from_source(x.rational(), x.binary64());
        }
        else return leaf(get<D>(p));
    }

    template <typename P>
    static bool apply(P const& p, point& out)
    {
        set<0>(out, ordinate<0>(p));
        set<1>(out, ordinate<1>(p));
        return true;
    }
};

template <bool Borrow = false, typename G>
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
            geometry::detail::transform::transform_box_or_segment::apply(g, result, input_point<>());
        else
            geometry::transform(g, result, input_point<Borrow>());
        return result;
    }
}

template <bool Approximate>
struct output_point
{
    template <typename T>
    static T ordinate(scalar x)
    {
        if constexpr (std::is_integral<T>::value || (Approximate && std::is_floating_point<T>::value))
            return geometry::util::numeric_cast<T>(x.approximation());
        else if constexpr (std::is_same<T, float>::value || std::is_same<T, double>::value)
            return geometry::detail::binary_rounding::round<T>(x.exact_value()).value;
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

template <typename Source>
struct matching_working
{
    template <typename G> struct apply : std::is_same<typename working<G>::type, Source> {};
};

template <bool Approximate>
struct convert_output
{
    template <typename Src, typename Result, typename Dst>
    void operator()(Src const& src, Result& result, Dst const&) const
    {
        if constexpr (util::is_box<Dst>::value)
            geometry::detail::transform::transform_box_or_segment::apply(src, result, output_point<Approximate>());
        else if constexpr (util::is_geometry_collection<Dst>::value)
            traits::visit<Src>::apply([&](auto const& part)
            {
                using match = matching_working<std::decay_t<decltype(part)>>;
                typename util::sequence_find_if<typename traits::geometry_types<Dst>::type,
                    match::template apply>::type converted;
                geometry::transform(part, converted, output_point<Approximate>());
                result = std::move(converted);
            }, src);
        else
            geometry::transform(src, result, output_point<Approximate>());
    }
};

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::lazy_decimal_detail

#endif
