// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_PREDICATES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_PREDICATES_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/predicate_point.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_measures.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/strategies/cartesian/exact/tagged_orientation.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>
#include <boost/geometry/views/detail/iterator_point_view.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root> class midpoint_query;

namespace root_predicate_detail
{
using exact_cartesian::with_homogeneous;
template <typename T> using has_root = std::integral_constant<bool, !std::is_void<typename root_of<T>::type>::value>;

template <typename A, typename B>
bool same_intersection(A const&, B const&) { return false; }

template <typename Root>
bool same_intersection(point_ref<Root> const& a, point_ref<Root> const& b)
{
    if (a.is_native() || b.is_native()) return false;
    auto const& x = a.record();
    auto const& y = b.record();
    if (&x == &y) return true;
    if (x.context != y.context) return false;
    // A unique crossing of the same original supports is the same exact point,
    // even when separate turn records own it. Never compare rounded coordinates.
    return x.supports == y.supports
        || (x.supports[0] == y.supports[1] && x.supports[1] == y.supports[0]);
}

template <typename P, bool Indirect = has_root<P>::value>
struct native_operand : fixed_native_supported<geometry::coordinate_type_t<P>> {};
template <typename P> struct native_operand<P, true> : std::false_type {};

template <typename P, bool Native = native_operand<P>::value>
struct exact_double : std::false_type {};
template <typename P> struct exact_double<P, true>
    : std::integral_constant<bool, std::numeric_limits<geometry::coordinate_type_t<P>>::digits
                                   <= std::numeric_limits<double>::digits> {};

template <typename Root, typename P, typename F>
decltype(auto) visit(P const& point, F&& function)
{
    if constexpr (record_point<P>::value) return point.visit(std::forward<F>(function));
    else return function(point);
}

template <typename Root, typename A, typename F>
decltype(auto) visit(geometry::detail::iterator_point_view<A> const& point, F&& function)
{
    using input = typename geometry::detail::iterator_point_view<A>::point_type;
    if constexpr (has_root<input>::value)
        return visit<Root>(point.read(), std::forward<F>(function));
    else return function(point);
}

template <typename Root, typename A, typename B, typename F>
decltype(auto) visit(A const& a, B const& b, F&& function)
{
    return visit<Root>(a, [&](auto const& x) -> decltype(auto)
    { return visit<Root>(b, [&](auto const& y) -> decltype(auto) { return function(x, y); }); });
}

template <typename Root, typename A, typename B, typename P, typename F>
decltype(auto) visit(A const& a, B const& b, P const& p, F&& function)
{
    return visit<Root>(a, b, [&](auto const& x, auto const& y) -> decltype(auto)
    { return visit<Root>(p, [&](auto const& z) -> decltype(auto) { return function(x, y, z); }); });
}


template <typename P>
auto copy_compact(P const& point, fixed_homogeneous<128>& output, int)
    -> decltype(point.copy_compact_homogeneous(output))
{ return point.copy_compact_homogeneous(output); }

template <typename P>
bool copy_compact(P const& point, fixed_homogeneous<128>& output, long)
{
    if constexpr (native_operand<P>::value)
        return try_fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point), output);
    else return with_homogeneous(point, [&](auto const& value) { return try_fixed_narrow(value, output); });
}

template <typename P>
void refine(P const& point, std::size_t dimension)
{
    if constexpr (!native_operand<P>::value)
        point.refine(dimension);
}

template <typename P>
void refine(P const& point) { refine(point, 0); refine(point, 1); }

template <typename P>
forward_error::point_approximation approximation(P const& point)
{
    if constexpr (native_operand<P>::value)
    {
        forward_error::point_approximation result;
        if (!forward_error::try_native_point(geometry::get<0>(point), geometry::get<1>(point), result))
            return {{{0, 0}}, {{(std::numeric_limits<double>::infinity)(),
                               (std::numeric_limits<double>::infinity)()}}};
        return result;
    }
    else
    {
        auto const x = point.approximation(0), y = point.approximation(1);
        return {{{x.value, y.value}}, {{x.error, y.error}}};
    }
}

template <typename I>
constexpr bool view_preserves_coordinate = std::is_same<geometry::coordinate_type_t<geometry::detail::iterator_point_view<I>>,
    geometry::coordinate_type_t<std::decay_t<typename geometry::detail::iterator_point_view<I>::reference>>>::value;

template <typename P>
constexpr bool direct_bilinear_read = true;
template <typename I>
constexpr bool direct_bilinear_read<geometry::detail::iterator_point_view<I>> = view_preserves_coordinate<I>
    && std::is_lvalue_reference<typename geometry::detail::iterator_point_view<I>::reference>::value;

template <typename Root, typename A, typename B, typename P>
constexpr bool stored_bilinear_operands = std::is_same<typename Root::coordinate_type, double>::value
    && (record_point<A>::value || record_point<B>::value || record_point<P>::value)
    && (record_point<A>::value || (exact_double<A>::value && direct_bilinear_read<A>))
    && (record_point<B>::value || (exact_double<B>::value && direct_bilinear_read<B>))
    && (record_point<P>::value || (exact_double<P>::value && direct_bilinear_read<P>));

// This is only used inside the typed visitor, where a point_ref argument
// denotes a construction. Decode the record or lvalue iterator once for X/Y.
template <typename P>
decltype(auto) bilinear_source(P const& p)
{
    if constexpr (record_point<P>::value) return (p.record().numeric);
    else return (p);
}

template <typename I>
decltype(auto) bilinear_source(geometry::detail::iterator_point_view<I> const& p) { return p.read(); }

template <std::size_t D, typename C, typename Policy>
scalar_approximation bilinear_coordinate(construction_rounding::point_state<C, Policy> const& p)
{ return p.approximation(D); }

template <std::size_t D, typename P>
scalar_approximation bilinear_coordinate(P const& p) { return {double(geometry::get<D>(p)), 0}; }

template <typename P>
auto bilinear_point(P const& point)
{
    auto const& source = bilinear_source(point);
    auto const x = bilinear_coordinate<0>(source), y = bilinear_coordinate<1>(source);
    return forward_error::detail::enclosed_point{{{x.value, x.error}, {y.value, y.error}}};
}

template <bool Dot, typename A, typename B, typename P>
bool stored_bilinear_sign(A const& a, B const& b, P const& p, int& result)
{
    namespace fe = forward_error::detail;
    fe::enclosed_bilinear_ops const ops;
    auto const x = bilinear_point(a), y = bilinear_point(b), z = bilinear_point(p);
    if constexpr (Dot)
        return fe::sign(linear_formula::direction(ops, x, y, z), result);
    else
        return fe::sign(linear_formula::orientation::evaluate(ops, x, y, z), result);
}

template <std::size_t D, typename P>
scalar_approximation coordinate_approximation(P const& point)
{
    if constexpr (native_operand<P>::value)
    {
        double value, error;
        if (!forward_error::try_native_coordinate(geometry::get<D>(point), value, error))
            return {0, (std::numeric_limits<double>::infinity)()};
        return {value, error};
    }
    else return point.approximation(D);
}

template <std::size_t D, typename Root, typename P>
bool native_coordinate(P const& point, typename Root::coordinate_type& output)
{
    if constexpr (record_point<P>::value) return point.native_coordinate(D, output);
    else if constexpr (native_operand<P>::value)
    {
        if constexpr (lossless_coordinate_conversion<geometry::coordinate_type_t<P>, typename Root::coordinate_type>::value)
        { output = geometry::get<D>(point); return true; }
    }
    return false;
}

template <typename Root, typename A, typename B, typename P>
bool on_support(A const& a, B const& b, P const& point)
{
    if constexpr (native_operand<A>::value && native_operand<B>::value && record_point<P>::value)
    {
        auto const& record = point.record();
        if (!record.context) return false;
        using C = typename Root::coordinate_type;
        for (auto const& edge : record.supports)
        {
            auto const support = record.context->support_coordinates(edge);
            if (support[0] == support[2] && support[1] == support[3]) continue;
            model::point<C, 2, cs::cartesian> const first(support[0], support[1]);
            model::point<C, 2, cs::cartesian> const second(support[2], support[3]);
            if (native_orientation<C>(first, second, a) == 0
                && native_orientation<C>(first, second, b) == 0) return true;
        }
    }
    return false;
}

template <typename Root, typename A, typename B, typename P>
int orientation(A const& a, B const& b, P const& p)
{
    if constexpr (native_operand<A>::value && native_operand<B>::value && native_operand<P>::value)
    {
        return native_orientation<typename Root::coordinate_type>(a, b, p);
    }
    else
    {
        int result;
        auto filter = [&]
        {
            return forward_error::try_orientation(approximation(a), approximation(b), approximation(p), result);
        };
        if constexpr (stored_bilinear_operands<Root, A, B, P>)
        {
            if (stored_bilinear_sign<false>(a, b, p, result)) return result;
        }
        else if (filter()) return result;
        if (same_intersection(a, b) || same_intersection(a, p) || same_intersection(b, p)) return 0;
        if (on_support<Root>(a, b, p) || on_support<Root>(b, p, a) || on_support<Root>(p, a, b)) return 0;
        refine(a); refine(b); refine(p);
        if (filter()) return result;
        // The visitor leaves point_ref operands here only for proper
        // crossings. Keep the fixed path if any exact record is already warm.
        if constexpr (std::is_same<typename Root::coordinate_type, double>::value
                   && record_point<A>::value && record_point<B>::value && record_point<P>::value)
        {
            using cache = point_cache_detail::native_cache<double>;
            if (!cache::has_exact(a.record().numeric.cache_handle())
             && !cache::has_exact(b.record().numeric.cache_handle())
             && !cache::has_exact(p.record().numeric.cache_handle()))
            {
                return tagged_orientation::from_producers(a.producers(), b.producers(), p.producers());
            }
        }
        fixed_homogeneous<128> x, y, z;
        if (copy_compact(a, x, 0) && copy_compact(b, y, 0) && copy_compact(p, z, 0))
            return fixed_orientation(x, y, z);
        return with_homogeneous(a, b, p, [](auto const& x, auto const& y, auto const& z)
        { return fixed_orientation(x, y, z); });
    }
}

template <std::size_t D, typename Root, typename A, typename B>
bool coordinate_filter(A const& a, B const& b, int& result)
{
    auto const x = coordinate_approximation<D>(a), y = coordinate_approximation<D>(b);
    return forward_error::try_compare_scalars(x, y, result);
}

template <std::size_t D, typename Root, typename A, typename B>
int compare_coordinate(A const& a, B const& b)
{
    static_assert(D < 2, "Exact Cartesian coordinate dimension");
    if constexpr (record_point<A>::value && record_point<B>::value)
        if (&a.record() == &b.record()) return 0;
    if constexpr (native_operand<A>::value && native_operand<B>::value)
    {
        using C = typename Root::coordinate_type;
        static_assert(lossless_coordinate_conversion<geometry::coordinate_type_t<A>, C>::value
            && lossless_coordinate_conversion<geometry::coordinate_type_t<B>, C>::value,
            "Root native coordinate promotion must be lossless");
        C const x = geometry::get<D>(a), y = geometry::get<D>(b);
        return int(x > y) - int(x < y);
    }
    else
    {
        int result;
        auto filter = [&] { return coordinate_filter<D, Root>(a, b, result); };
        if (filter()) return result;
        if (same_intersection(a, b)) return 0;
        typename Root::coordinate_type x, y;
        if (native_coordinate<D, Root>(a, x) && native_coordinate<D, Root>(b, y))
            return int(x > y) - int(x < y);
        refine(a, D); refine(b, D);
        if (filter()) return result;
        fixed_homogeneous<128> first, second;
        if (copy_compact(a, first, 0) && copy_compact(b, second, 0))
            return fixed_coordinate_compare(first, second, D);
        return with_homogeneous(a, b, [](auto const& x, auto const& y)
        { return fixed_coordinate_compare(x, y, D); });
    }
}

template <typename Root, typename A, typename B>
bool equal(A const& a, B const& b)
{
    if constexpr (std::is_same<A, B>::value) if (&a == &b) return true;
    if constexpr (record_point<A>::value && record_point<B>::value)
        if (&a.record() == &b.record()) return true;
    if constexpr (native_operand<A>::value && native_operand<B>::value)
        return compare_coordinate<0, Root>(a, b) == 0 && compare_coordinate<1, Root>(a, b) == 0;
    else
    {
        // Either axis can disprove equality without exact refinement of the
        // other one, including points on a shared vertical/horizontal support.
        for (bool refined : {false, true})
        {
            if (refined)
            {
                if (same_intersection(a, b)) return true;
                refine(a); refine(b);
            }
            int x, y;
            bool const cx = coordinate_filter<0, Root>(a, b, x);
            if (cx && x != 0) return false;
            bool const cy = coordinate_filter<1, Root>(a, b, y);
            if (cy && y != 0) return false;
            if (cx && cy) return true;
        }
        fixed_homogeneous<128> first, second;
        if (copy_compact(a, first, 0) && copy_compact(b, second, 0))
            return fixed_coordinate_compare(first, second, 0) == 0 && fixed_coordinate_compare(first, second, 1) == 0;
        return with_homogeneous(a, b, [](auto const& x, auto const& y)
        { return fixed_coordinate_compare(x, y, 0) == 0 && fixed_coordinate_compare(x, y, 1) == 0; });
    }
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD, std::size_t PN, std::size_t PD>
int checked_fixed_direction(fixed_homogeneous<AN, AD> const& a, fixed_homogeneous<BN, BD> const& b,
                    fixed_homogeneous<PN, PD> const& p)
{
    if (a.W <= 0 || b.W <= 0 || p.W <= 0) throw std::domain_error("Normalized finite homogeneous points required");
    return exact_cartesian::fixed_direction(a, b, p);
}

template <typename Root, typename A, typename B, typename P>
int direction(A const& a, B const& b, P const& p)
{
    int result;
    auto filter = [&]
    {
        return forward_error::try_direction(approximation(a), approximation(b), approximation(p), result);
    };
    if constexpr (stored_bilinear_operands<Root, A, B, P>)
    {
        if (stored_bilinear_sign<true>(a, b, p, result)) return result;
    }
    else if (filter()) return result;
    if (same_intersection(a, b) || same_intersection(b, p)) return 0;
    if (root_predicate_detail::orientation<Root>(a, b, p) == 0)
    {
        int const dx = compare_coordinate<0, Root>(b, a);
        return dx != 0 ? dx * compare_coordinate<0, Root>(p, b)
            : compare_coordinate<1, Root>(b, a) * compare_coordinate<1, Root>(p, b);
    }
    refine(a); refine(b); refine(p);
    if (filter()) return result;
    fixed_homogeneous<128> x, y, z;
    if (copy_compact(a, x, 0) && copy_compact(b, y, 0) && copy_compact(p, z, 0))
        return checked_fixed_direction(x, y, z);
    return with_homogeneous(a, b, p, [](auto const& x, auto const& y, auto const& z)
    { return checked_fixed_direction(x, y, z); });
}

template <typename Root, typename A, typename B>
scalar_approximation squared_distance_approximation(A const& a, B const& b, bool refined)
{
    return visit<Root>(a, b, [&](auto const& x, auto const& y)
    {
        if (refined) { refine(x); refine(y); }
        scalar_approximation result{0, (std::numeric_limits<double>::infinity)()};
        forward_error::try_squared_distance(approximation(x), approximation(y), result);
        return result;
    });
}

inline void require_zero(int value)
{ if (value != 0) throw std::invalid_argument("A Root measure accepts only the zero scalar"); }

template <typename Root>
using measure_pair = typename fixed_measure_traits<native_fixed_traits<typename Root::coordinate_type>::intersection_numerator_bits,
    native_fixed_traits<typename Root::coordinate_type>::intersection_denominator_bits>::pair_type;

template <typename Root, typename P>
point_ref<Root> capture(P const& source) { return point_ref<Root>::borrow(source); }

template <typename Root, typename I>
point_ref<Root> capture(geometry::detail::iterator_point_view<I> const& source)
{
    using view = geometry::detail::iterator_point_view<I>;
    if constexpr (boost::mp11::mp_contains<typename Root::native_access, view>::value)
        return point_ref<Root>::borrow(source);
    else
    {
        // Boundary/reverse views need not expand Root's variant. Keep
        // the actual original lvalue, never a pointer to this temporary view.
        static_assert(std::is_lvalue_reference<typename view::reference>::value
            || std::is_same<std::decay_t<typename view::reference>, point_ref<Root>>::value,
            "Value-producing proxy iterators must be registered in Root::native_access");
        return capture<Root>(source.read());
    }
}

} // namespace root_predicate_detail

template <typename Root, typename A, typename B, typename P>
int root_orientation(A const& a, B const& b, P const& p)
{
    return root_predicate_detail::visit<Root>(a, b, p, [](auto const& x, auto const& y, auto const& z)
    { return root_predicate_detail::orientation<Root>(x, y, z); });
}

template <std::size_t D, typename Root, typename A, typename B>
int root_compare_coordinate(A const& a, B const& b)
{
    return root_predicate_detail::visit<Root>(a, b, [](auto const& x, auto const& y)
    { return root_predicate_detail::compare_coordinate<D, Root>(x, y); });
}

template <typename Root, typename A, typename B>
int root_compare_coordinate(A const& a, B const& b, std::size_t axis)
{
    if (axis > 1) throw std::out_of_range("Exact Cartesian coordinate dimension");
    return axis == 0 ? root_compare_coordinate<0, Root>(a, b) : root_compare_coordinate<1, Root>(a, b);
}

template <typename Root, typename A, typename B>
bool root_equal(A const& a, B const& b)
{
    return root_predicate_detail::visit<Root>(a, b, [](auto const& x, auto const& y)
    { return root_predicate_detail::equal<Root>(x, y); });
}

template <typename Root, typename A, typename B, typename P>
int root_direction(A const& a, B const& b, P const& p)
{
    return root_predicate_detail::visit<Root>(a, b, p, [](auto const& x, auto const& y, auto const& z)
    { return root_predicate_detail::direction<Root>(x, y, z); });
}

// A descriptor retains only borrowed accessors/record references and its scalar
// forward bound. Native snapshots are not stored.
template <typename Root>
struct RootSquaredDistanceValue
{
    using root_type = Root;
    using pair_type = root_predicate_detail::measure_pair<Root>;
    using compact_pair_type = typename fixed_measure_detail::distance_capacity<128, 128, 128, 128>::type;
    point_ref<Root> a, b;
    mutable double approximate = 0, error = 0;
    mutable bool valid = true;
    bool literal = true, unset = false;

    RootSquaredDistanceValue() = default;
    RootSquaredDistanceValue(int zero)
    {
        if (zero == -1) { unset = true; approximate = -1; }
        else root_predicate_detail::require_zero(zero);
    }
    RootSquaredDistanceValue(point_ref<Root> first, point_ref<Root> second)
        : a(std::move(first)), b(std::move(second)), literal(false)
    { refresh(false); }
    template <typename A, typename B>
    RootSquaredDistanceValue(A const& first, B const& second)
        : RootSquaredDistanceValue(root_predicate_detail::capture<Root>(first),
                                  root_predicate_detail::capture<Root>(second)) {}

    void refresh(bool refined) const
    {
        auto const result = root_predicate_detail::squared_distance_approximation<Root>(a, b, refined);
        valid = result.valid(); approximate = result.value; error = result.error;
    }
    scalar_approximation approximation(bool refined = false) const
    {
        if (literal) return {unset ? -1.0 : 0.0, 0};
        if (refined) refresh(true);
        return valid ? scalar_approximation{approximate, error} : scalar_approximation{0, (std::numeric_limits<double>::infinity)()};
    }
    pair_type fixed_pair() const
    {
        pair_type result;
        if (literal) { result.N = unset ? -1 : 0; return result; }
        root_predicate_detail::visit<Root>(a, b, [&](auto const& x, auto const& y)
        {
            root_predicate_detail::with_homogeneous(x, [&](auto const& first)
            { root_predicate_detail::with_homogeneous(y, [&](auto const& second)
              { auto const pair = fixed_squared_distance(first, second); result.N = pair.N; result.D = pair.D; }); });
        });
        return result;
    }
    bool compact_pair(compact_pair_type& output) const
    {
        if (literal) { output.N = unset ? -1 : 0; output.D = 1; return true; }
        fixed_homogeneous<128> first, second;
        if (!a.copy_compact_homogeneous(first) || !b.copy_compact_homogeneous(second)) return false;
        output = fixed_squared_distance(first, second);
        return true;
    }
    bool is_zero() const { return !unset && (literal || root_equal<Root>(a, b)); }

    int compare_distance(RootSquaredDistanceValue const& other) const
    {
        if (this == &other) return 0;
        int result;
        auto const filter = [&](bool refine)
        {
            auto const first = approximation(refine), second = other.approximation(refine);
            return forward_error::try_compare_scalars(first, second, result);
        };
        if (filter(false)) return result;

        std::uint64_t first, second;
        if (!literal && !other.literal
            && small_integer_squared_distance(a, b, first)
            && small_integer_squared_distance(other.a, other.b, second))
        {
            return static_cast<int>(first > second) - static_cast<int>(first < second);
        }
        if (filter(true)) return result;

        compact_pair_type compact_first, compact_second;
        if (compact_pair(compact_first) && other.compact_pair(compact_second))
        {
            return fixed_pair_compare(compact_first, compact_second);
        }
        return fixed_pair_compare(fixed_pair(), other.fixed_pair());
    }
};

template <typename Root, typename A, typename B>
RootSquaredDistanceValue<Root> root_squared_distance(A const& a, B const& b)
{ return {a, b}; }

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
