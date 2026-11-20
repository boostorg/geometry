// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_PREDICATES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_PREDICATES_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_records.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_measures.hpp>
#include <boost/geometry/strategies/cartesian/exact/tagged_orientation.hpp>
#include <boost/geometry/strategies/cartesian/exact/ulp_filter.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root> class query_point;

namespace borrowed_predicates_detail
{
template <typename T, typename = void> struct has_root : std::false_type {};
template <typename T> struct has_root<T, std::void_t<typename T::root_type>> : std::true_type {};
template <typename T> struct owning_point : std::false_type {};
template <typename C> struct owning_point<exact_point<C>> : std::true_type {};
template <typename T> struct record_point : std::false_type {};
template <typename R> struct record_point<borrowed_point<R>> : std::true_type {};

template <typename A, typename B>
bool same_intersection(A const&, B const&) { return false; }

template <typename Root>
bool same_intersection(borrowed_point<Root> const& a, borrowed_point<Root> const& b)
{
    if (a.is_native() || b.is_native()) return false;
    auto const& x = a.record();
    auto const& y = b.record();
    if (&x == &y) return true;
    if (&x.stage->context != &y.stage->context) return false;
    // A unique crossing of the same original supports is the same exact point,
    // even when separate turn records own it. Never compare rounded coordinates.
    if (x.stage == y.stage || (x.stage->finalized && y.stage->finalized))
    {
        auto const& a0 = x.operations[0]->seg_id;
        auto const& a1 = x.operations[1]->seg_id;
        auto const& b0 = y.operations[0]->seg_id;
        auto const& b1 = y.operations[1]->seg_id;
        if ((a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0)) return true;
    }
    auto const a0 = x.stage->support(x.operations[0]->seg_id);
    auto const a1 = x.stage->support(x.operations[1]->seg_id);
    auto const b0 = y.stage->support(y.operations[0]->seg_id);
    auto const b1 = y.stage->support(y.operations[1]->seg_id);
    return (a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0);
}

template <typename P, bool Indirect = has_root<P>::value || owning_point<P>::value>
struct native_operand : fixed_native_supported<geometry::coordinate_type_t<P>> {};
template <typename P> struct native_operand<P, true> : std::false_type {};

template <typename P, bool Native = native_operand<P>::value>
struct exact_double : std::false_type {};
template <typename P> struct exact_double<P, true>
    : std::integral_constant<bool, std::numeric_limits<geometry::coordinate_type_t<P>>::digits <= 53> {};

template <typename Root, typename P, typename F>
decltype(auto) visit(P const& point, F&& function)
{
    if constexpr (record_point<P>::value) return point.visit(std::forward<F>(function));
    else return function(point);
}

template <typename Root, typename A, typename F>
decltype(auto) visit(iterator_point_view<A> const& point, F&& function)
{
    using input = typename iterator_point_view<A>::point_type;
    if constexpr (has_root<input>::value || owning_point<input>::value)
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

// Called AFTER visiting: native actual points/accessors keep their native
// format, queries keep their declared fixed expression budget, and owning
// auxiliary points keep their precision format. Never round through BG get()
// on an indirect point or narrow an auxiliary query to the Root point budget.
template <typename P, typename F>
decltype(auto) with_homogeneous(P const& point, F&& function)
{
    if constexpr (native_operand<P>::value)
        return function(fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point)));
    else if constexpr (owning_point<P>::value)
    {
        if (point.has_fixed_representation()) return function(point.fixed_homogeneous());
        return function(point.precision_homogeneous());
    }
    else return function(point.homogeneous());
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
    if constexpr (owning_point<P>::value)
        point.refined_approximation(dimension);
    else if constexpr (!native_operand<P>::value)
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
        return {{{x.approximate, y.approximate}}, {{x.error, y.error}}};
    }
}

template <typename Root>
using root_filter = construction_rounding::policy_traits<typename Root::output_policy>;

template <typename Root>
using root_certificate = typename root_filter<Root>::arithmetic_certificate;

// After visiting, a matching record carries its producer's certificate.
// query_point factories either prove exactness or round the fixed quotient.
// An arbitrary owning point or lossy conversion has neither proof.
// For bounded roots this whitelist also proves the producer's absolute
// error bound, not just the distance from the nearest double in steps.
template <typename Root, typename P>
constexpr bool step_operand = exact_double<P>::value || std::is_same<P, borrowed_point<Root>>::value
    || std::is_same<P, query_point<Root>>::value;

template <typename Root, typename... P>
constexpr bool step_filter = !std::is_void<root_certificate<Root>>::value && (step_operand<Root, P> && ...);

template <typename P>
double bounded_ordinate(P const& point, std::size_t dim)
{
    if constexpr (record_point<P>::value) return point.record().numeric.approximate(dim);
    else if constexpr (native_operand<P>::value)
        return dim == 0 ? double(geometry::get<0>(point)) : double(geometry::get<1>(point));
    else return point.approximate(dim);
}

// Called after visit(): a borrowed point here is the selected crossing,
// not a native alternative. Read X and Y without a second visit.
template <typename P>
ulp_filter::coordinates bounded_coordinates(P const& point)
{
    if constexpr (record_point<P>::value)
    {
        auto const& state = point.record().numeric;
        return {{state.approximate(0), state.approximate(1)}};
    }
    else return {{bounded_ordinate(point, 0), bounded_ordinate(point, 1)}};
}

template <typename I>
constexpr bool view_preserves_coordinate = std::is_same<geometry::coordinate_type_t<iterator_point_view<I>>,
    geometry::coordinate_type_t<std::decay_t<typename iterator_point_view<I>::reference>>>::value;

template <typename I>
ulp_filter::coordinates bounded_coordinates(iterator_point_view<I> const& view)
{
    if constexpr (view_preserves_coordinate<I>)
        return bounded_coordinates(view.read());
    else
    {
        // Match the view's Point accessor, even if its proxy uses wider ordinates.
        auto&& p = view.read();
        using C = geometry::coordinate_type_t<iterator_point_view<I>>;
        C const x = geometry::get<0>(p), y = geometry::get<1>(p);
        return {{double(x), double(y)}};
    }
}

template <typename P>
constexpr bool direct_bilinear_read = true;
template <typename I>
constexpr bool direct_bilinear_read<iterator_point_view<I>> = view_preserves_coordinate<I>
    && std::is_lvalue_reference<typename iterator_point_view<I>::reference>::value;

template <typename Root, typename A, typename B, typename P>
constexpr bool stored_bilinear_operands = root_filter<Root>::store_errors
    && std::is_same<typename Root::coordinate_type, double>::value
    && (record_point<A>::value || record_point<B>::value || record_point<P>::value)
    && (record_point<A>::value || (exact_double<A>::value && direct_bilinear_read<A>))
    && (record_point<B>::value || (exact_double<B>::value && direct_bilinear_read<B>))
    && (record_point<P>::value || (exact_double<P>::value && direct_bilinear_read<P>));

// This is only used inside the typed visitor, where a borrowed_point argument
// denotes a construction. Decode the record or lvalue iterator once for X/Y.
template <typename P>
decltype(auto) bilinear_source(P const& p)
{
    if constexpr (record_point<P>::value) return (p.record().numeric);
    else return (p);
}

template <typename I>
decltype(auto) bilinear_source(iterator_point_view<I> const& p) { return p.read(); }

template <std::size_t D, typename C, typename Policy>
interval bilinear_coordinate(construction_rounding::point_state<C, Policy> const& p)
{ return p.approximation(D); }

template <std::size_t D, typename P>
interval bilinear_coordinate(P const& p) { return {double(geometry::get<D>(p)), 0}; }

struct bilinear_axis
{
    double u, v, ea, eb, ep;
    bool exact() const { return ea == 0 && eb == 0 && ep == 0; }
    double ue() const { return forward_error::detail::difference_error(u, ea, eb); }
    template <bool Dot>
    double ve() const { return forward_error::detail::difference_error(v, ep, Dot ? eb : ea); }
};

template <bool Dot, std::size_t D, typename A, typename B, typename P>
bilinear_axis read_bilinear_axis(A const& a, B const& b, P const& p)
{
    auto const x = bilinear_coordinate<D>(a), y = bilinear_coordinate<D>(b), z = bilinear_coordinate<D>(p);
    return {arithmetic_detail::rounded_sub(y.approximate, x.approximate),
            arithmetic_detail::rounded_sub(z.approximate, Dot ? y.approximate : x.approximate),
            x.error, y.error, z.error};
}

template <bool Dot, typename A, typename B, typename P>
bool stored_bilinear_sign(A const& a, B const& b, P const& p, int& result)
{
    namespace fe = forward_error::detail;
    namespace ar = arithmetic_detail;
    auto const& first = bilinear_source(a);
    auto const& second = bilinear_source(b);
    auto const& point = bilinear_source(p);
    auto const x = read_bilinear_axis<Dot, 0>(first, second, point);
    auto const y = read_bilinear_axis<Dot, 1>(first, second, point);
    // These errors come from point_state or exact native conversions. Any
    // nonfinite coordinate propagates through a difference and product, so
    // sign's result check rejects it. The zero-error branch changes only the
    // bound, never the operand's support, identity, or fallback representation.
    auto const& v1 = Dot ? x : y;
    auto const& v2 = Dot ? y : x;
    double const t1 = ar::rounded_mul(x.u, v1.v), t2 = ar::rounded_mul(y.u, v2.v);
    double const value = Dot ? ar::rounded_add(t1, t2) : ar::rounded_sub(t1, t2);
    double const error = x.exact() && y.exact()
        ? fe::native_bilinear_error(t1, t2)
        : fe::bilinear_error(x.u, y.u, v1.v, v2.v, x.ue(), y.ue(),
                             v1.template ve<Dot>(), v2.template ve<Dot>(), t1, t2, value);
    return fe::sign({value, error}, result);
}

template <std::size_t D, typename P>
interval coordinate_approximation(P const& point)
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
        if (!record.stage || !record.operations[0] || !record.operations[1]) return false;
        using C = typename Root::coordinate_type;
        for (auto const* operation : record.operations)
        {
            auto const support = record.stage->context.support_coordinates(record.stage->support(operation->seg_id));
            if (support[0] == support[2] && support[1] == support[3]) continue;
            model::point<C, 2, cs::cartesian> const first(support[0], support[1]);
            model::point<C, 2, cs::cartesian> const second(support[2], support[3]);
            if (native_orientation<C>(first, second, a) == 0
                && native_orientation<C>(first, second, b) == 0) return true;
        }
    }
    return false;
}

template <typename Root, typename A, typename B, typename P, typename Certified = std::false_type>
int orientation(A const& a, B const& b, P const& p, Certified native_differences_exact = {})
{
    if constexpr (native_operand<A>::value && native_operand<B>::value && native_operand<P>::value)
    {
        if constexpr (std::is_same<typename Root::coordinate_type, double>::value
                      && exact_double<A>::value && exact_double<B>::value && exact_double<P>::value)
        {
            if (native_differences_exact) return sterbenz::orientation(a, b, p);
        }
        return native_orientation<typename Root::coordinate_type>(a, b, p);
    }
    else
    {
        int result;
        auto filter = [&]
        {
            if constexpr (step_filter<Root, A, B, P>)
                return ulp_filter::try_orientation<root_certificate<Root>,
                    !exact_double<A>::value, !exact_double<B>::value, !exact_double<P>::value>
                    (bounded_coordinates(a), bounded_coordinates(b), bounded_coordinates(p), result);
            else return forward_error::try_orientation(approximation(a), approximation(b), approximation(p), result);
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
        // The visitor leaves borrowed_point operands here only for proper
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
        return with_homogeneous(a, [&](auto const& x)
        { return with_homogeneous(b, [&](auto const& y)
          { return with_homogeneous(p, [&](auto const& z) { return fixed_orientation(x, y, z); }); }); });
    }
}

template <std::size_t D, typename Root, typename A, typename B>
bool coordinate_filter(A const& a, B const& b, int& result)
{
    if constexpr (step_filter<Root, A, B>)
    {
        return ulp_filter::try_compare_n<root_filter<Root>::order_steps,
            !exact_double<A>::value, !exact_double<B>::value>
            (bounded_ordinate(a, D), bounded_ordinate(b, D), result);
    }
    else
    {
        auto const x = coordinate_approximation<D>(a), y = coordinate_approximation<D>(b);
        return forward_error::try_compare_scalars(x.approximate, x.error, y.approximate, y.error, result);
    }
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
        return with_homogeneous(a, [&](auto const& x)
        { return with_homogeneous(b, [&](auto const& y) { return fixed_coordinate_compare(x, y, D); }); });
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
        return with_homogeneous(a, [&](auto const& x)
        { return with_homogeneous(b, [&](auto const& y)
          { return fixed_coordinate_compare(x, y, 0) == 0 && fixed_coordinate_compare(x, y, 1) == 0; }); });
    }
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD, std::size_t PN, std::size_t PD>
int fixed_direction(fixed_homogeneous<AN, AD> const& a, fixed_homogeneous<BN, BD> const& b,
                    fixed_homogeneous<PN, PD> const& p)
{
    if (a.W <= 0 || b.W <= 0 || p.W <= 0) throw std::domain_error("Normalized finite homogeneous points required");
    constexpr std::size_t first = fixed_detail::maximum(BN + AD, AN + BD) + 1;
    constexpr std::size_t second = fixed_detail::maximum(PN + BD, BN + PD) + 1;
    constexpr std::size_t bits = first + second + 1;
    auto const x = fixed_multiply<first>(b.X, a.W) - fixed_multiply<first>(a.X, b.W);
    auto const y = fixed_multiply<first>(b.Y, a.W) - fixed_multiply<first>(a.Y, b.W);
    auto const px = fixed_multiply<second>(p.X, b.W) - fixed_multiply<second>(b.X, p.W);
    auto const py = fixed_multiply<second>(p.Y, b.W) - fixed_multiply<second>(b.Y, p.W);
    fixed_integer<bits> const result = fixed_multiply<bits>(x, px) + fixed_multiply<bits>(y, py);
    return result.sign();
}

template <typename Root, typename A, typename B, typename P>
int direction(A const& a, B const& b, P const& p)
{
    int result;
    auto filter = [&]
    {
        if constexpr (step_filter<Root, A, B, P>)
            return ulp_filter::try_dot<root_certificate<Root>,
                !exact_double<A>::value, !exact_double<B>::value, !exact_double<P>::value>
                (bounded_coordinates(a), bounded_coordinates(b), bounded_coordinates(p), result);
        else return forward_error::try_direction(approximation(a), approximation(b), approximation(p), result);
    };
    if constexpr (stored_bilinear_operands<Root, A, B, P>)
    {
        if (stored_bilinear_sign<true>(a, b, p, result)) return result;
    }
    else if (filter()) return result;
    if (same_intersection(a, b) || same_intersection(b, p)) return 0;
    if (borrowed_predicates_detail::orientation<Root>(a, b, p) == 0)
    {
        int const dx = compare_coordinate<0, Root>(b, a);
        return dx != 0 ? dx * compare_coordinate<0, Root>(p, b)
            : compare_coordinate<1, Root>(b, a) * compare_coordinate<1, Root>(p, b);
    }
    refine(a); refine(b); refine(p);
    if (filter()) return result;
    fixed_homogeneous<128> x, y, z;
    if (copy_compact(a, x, 0) && copy_compact(b, y, 0) && copy_compact(p, z, 0))
        return fixed_direction(x, y, z);
    return with_homogeneous(a, [&](auto const& x)
    { return with_homogeneous(b, [&](auto const& y)
      { return with_homogeneous(p, [&](auto const& z) { return fixed_direction(x, y, z); }); }); });
}

template <typename Root, bool Signed, typename A, typename B, typename P>
forward_error::scalar_approximation measure(A const& a, B const& b, P const& p, bool refined)
{
    return visit<Root>(a, b, p, [&](auto const& x, auto const& y, auto const& z)
    {
        if (refined) { refine(x); refine(y); if constexpr (Signed) refine(z); }
        if constexpr (step_filter<Root, std::decay_t<decltype(x)>, std::decay_t<decltype(y)>,
                                        std::decay_t<decltype(z)>>)
        {
            constexpr bool xi = !exact_double<std::decay_t<decltype(x)>>::value;
            constexpr bool yi = !exact_double<std::decay_t<decltype(y)>>::value;
            forward_error::scalar_approximation result{0, (std::numeric_limits<double>::infinity)()};
            if constexpr (Signed)
                ulp_filter::try_signed_determinant<root_certificate<Root>, xi, yi,
                    !exact_double<std::decay_t<decltype(z)>>::value>
                    (bounded_coordinates(x), bounded_coordinates(y), bounded_coordinates(z), result);
            else ulp_filter::try_squared_distance<root_certificate<Root>, xi, yi>
                    (bounded_coordinates(x), bounded_coordinates(y), result);
            return result;
        }
        else
        {
            forward_error::scalar_approximation result{0, (std::numeric_limits<double>::infinity)()};
            if constexpr (Signed) forward_error::try_signed_determinant(approximation(x), approximation(y), approximation(z), result);
            else forward_error::try_squared_distance(approximation(x), approximation(y), result);
            return result;
        }
    });
}

inline void require_zero(int value)
{ if (value != 0) throw std::invalid_argument("A Root measure accepts only the zero scalar"); }

template <typename Root>
using measure_pair = typename fixed_measure_traits<native_fixed_traits<typename Root::coordinate_type>::intersection_numerator_bits,
    native_fixed_traits<typename Root::coordinate_type>::intersection_denominator_bits>::pair_type;

template <typename Root, typename P>
borrowed_point<Root> capture(P const& source) { return borrowed_point<Root>::borrow(source); }

template <typename Root, typename I>
borrowed_point<Root> capture(iterator_point_view<I> const& source)
{
    using view = iterator_point_view<I>;
    if constexpr (boost::mp11::mp_contains<typename Root::native_access, view>::value)
        return borrowed_point<Root>::borrow(source);
    else
    {
        // Boundary/reverse views need not expand Root's variant. Keep
        // the actual original lvalue, never a pointer to this temporary view.
        static_assert(std::is_lvalue_reference<typename view::reference>::value
            || std::is_same<std::decay_t<typename view::reference>, borrowed_point<Root>>::value,
            "Value-producing proxy iterators must be registered in Root::native_access");
        return capture<Root>(source.read());
    }
}

} // namespace borrowed_predicates_detail

template <typename Root, typename A, typename B, typename P>
int root_orientation(A const& a, B const& b, P const& p)
{
    return borrowed_predicates_detail::visit<Root>(a, b, p, [](auto const& x, auto const& y, auto const& z)
    { return borrowed_predicates_detail::orientation<Root>(x, y, z); });
}

template <typename Root, typename A, typename B, typename P, typename Certified>
int root_orientation(A const& a, B const& b, P const& p, Certified native_differences_exact)
{
    return borrowed_predicates_detail::visit<Root>(a, b, p, [&](auto const& x, auto const& y, auto const& z)
    { return borrowed_predicates_detail::orientation<Root>(x, y, z, native_differences_exact); });
}

template <std::size_t D, typename Root, typename A, typename B>
int root_compare_coordinate(A const& a, B const& b)
{
    return borrowed_predicates_detail::visit<Root>(a, b, [](auto const& x, auto const& y)
    { return borrowed_predicates_detail::compare_coordinate<D, Root>(x, y); });
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
    return borrowed_predicates_detail::visit<Root>(a, b, [](auto const& x, auto const& y)
    { return borrowed_predicates_detail::equal<Root>(x, y); });
}

template <typename Root, typename A, typename B, typename P>
int root_direction(A const& a, B const& b, P const& p)
{
    return borrowed_predicates_detail::visit<Root>(a, b, p, [](auto const& x, auto const& y, auto const& z)
    { return borrowed_predicates_detail::direction<Root>(x, y, z); });
}

// A descriptor retains only borrowed accessors/record references and its scalar
// forward bound. Native snapshots and owning exact_point values are not stored.
template <typename Root>
struct RootSquaredDistanceValue
{
    using root_type = Root;
    using pair_type = borrowed_predicates_detail::measure_pair<Root>;
    using compact_pair_type = typename fixed_measure_detail::distance_capacity<128, 128, 128, 128>::type;
    borrowed_point<Root> a, b;
    mutable double approximate = 0, error = 0;
    mutable bool valid = true;
    bool literal = true, unset = false;

    RootSquaredDistanceValue() = default;
    RootSquaredDistanceValue(int zero)
    {
        if (zero == -1) { unset = true; approximate = -1; }
        else borrowed_predicates_detail::require_zero(zero);
    }
    RootSquaredDistanceValue(borrowed_point<Root> first, borrowed_point<Root> second)
        : a(std::move(first)), b(std::move(second)), literal(false)
    { refresh(false); }
    template <typename A, typename B>
    RootSquaredDistanceValue(A const& first, B const& second)
        : RootSquaredDistanceValue(borrowed_predicates_detail::capture<Root>(first),
                                  borrowed_predicates_detail::capture<Root>(second)) {}

    void refresh(bool refined) const
    {
        auto const result = borrowed_predicates_detail::measure<Root, false>(a, b, b, refined);
        valid = result.valid(); approximate = result.value; error = result.error;
    }
    interval bounds(bool refined = false) const
    {
        if (literal) return {unset ? -1.0 : 0.0, 0};
        if (refined) refresh(true);
        return valid ? interval{approximate, error} : interval{0, (std::numeric_limits<double>::infinity)()};
    }
    pair_type fixed_pair() const
    {
        pair_type result;
        if (literal) { result.N = unset ? -1 : 0; return result; }
        borrowed_predicates_detail::visit<Root>(a, b, [&](auto const& x, auto const& y)
        {
            borrowed_predicates_detail::with_homogeneous(x, [&](auto const& first)
            { borrowed_predicates_detail::with_homogeneous(y, [&](auto const& second)
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
    int sign() const { return unset ? -1 : is_zero() ? 0 : 1; }
    bool is_positive() const { return sign() > 0; }
    bool is_negative() const { return sign() < 0; }
};

template <typename Root>
struct RootSignedMeasure
{
    using root_type = Root;
    using pair_type = borrowed_predicates_detail::measure_pair<Root>;
    using compact_pair_type = typename fixed_measure_detail::determinant_capacity<128, 128, 128, 128, 128, 128>::type;
    borrowed_point<Root> a, b, p;
    mutable double approximate = 0, error = 0;
    mutable bool valid = true;
    bool literal = true, negated = false;

    RootSignedMeasure() = default;
    RootSignedMeasure(int zero) { borrowed_predicates_detail::require_zero(zero); }
    RootSignedMeasure(borrowed_point<Root> first, borrowed_point<Root> second, borrowed_point<Root> point)
        : a(std::move(first)), b(std::move(second)), p(std::move(point)), literal(false)
    { refresh(false); }
    template <typename A, typename B, typename P>
    RootSignedMeasure(A const& first, B const& second, P const& point)
        : RootSignedMeasure(borrowed_predicates_detail::capture<Root>(first),
            borrowed_predicates_detail::capture<Root>(second), borrowed_predicates_detail::capture<Root>(point)) {}

    void refresh(bool refined) const
    {
        auto const result = borrowed_predicates_detail::measure<Root, true>(a, b, p, refined);
        valid = result.valid(); approximate = negated ? -result.value : result.value; error = result.error;
    }
    interval bounds(bool refined = false) const
    {
        if (literal) return {0, 0};
        if (refined) refresh(true);
        return valid ? interval{approximate, error} : interval{0, (std::numeric_limits<double>::infinity)()};
    }
    pair_type fixed_pair() const
    {
        pair_type result;
        if (literal) return result;
        borrowed_predicates_detail::visit<Root>(a, b, p, [&](auto const& x, auto const& y, auto const& z)
        {
            borrowed_predicates_detail::with_homogeneous(x, [&](auto const& first)
            { borrowed_predicates_detail::with_homogeneous(y, [&](auto const& second)
              { borrowed_predicates_detail::with_homogeneous(z, [&](auto const& third)
                { auto const pair = fixed_signed_determinant(first, second, third); result.N = pair.N; result.D = pair.D; }); }); });
        });
        if (negated) result.N = -result.N;
        return result;
    }
    bool compact_pair(compact_pair_type& output) const
    {
        if (literal) { output.N = 0; output.D = 1; return true; }
        fixed_homogeneous<128> first, second, third;
        if (!a.copy_compact_homogeneous(first) || !b.copy_compact_homogeneous(second)
            || !p.copy_compact_homogeneous(third)) return false;
        output = fixed_signed_determinant(first, second, third);
        if (negated) output.N = -output.N;
        return true;
    }
    int sign() const
    { if (literal) return 0; int const result = root_orientation<Root>(a, b, p); return negated ? -result : result; }
    bool is_zero() const { return sign() == 0; }
    bool is_positive() const { return sign() > 0; }
    bool is_negative() const { return sign() < 0; }
    RootSignedMeasure operator-() const
    { auto result = *this; result.negated = !result.negated; result.approximate = -result.approximate; return result; }
};

template <typename Root, typename A, typename B>
RootSquaredDistanceValue<Root> root_squared_distance(A const& a, B const& b)
{ return {a, b}; }

template <typename Root, typename A, typename B, typename P>
RootSignedMeasure<Root> root_signed_determinant(A const& a, B const& b, P const& p)
{ return {a, b, p}; }

template <typename Root, typename A, typename B, typename P>
RootSignedMeasure<Root> root_distance_measure(A const& a, B const& b, P const& p)
{ return root_signed_determinant<Root>(a, b, p); }

namespace borrowed_predicates_detail
{
template <typename T> struct is_measure : std::false_type {};
template <typename R> struct is_measure<RootSquaredDistanceValue<R>> : std::true_type {};
template <typename R> struct is_measure<RootSignedMeasure<R>> : std::true_type {};

template <typename Root>
bool small_integer_coordinate(borrowed_point<Root> const& point, std::size_t axis, std::int64_t& output)
{
    typename Root::coordinate_type native;
    if (!point.native_coordinate(axis, native)) return false;
    double const value = double(native);
    constexpr double limit = 1073741824.0;
    if (!(value >= -limit && value <= limit)) return false;
    auto const integer = static_cast<std::int64_t>(value);
    if (static_cast<typename Root::coordinate_type>(integer) != native) return false;
    output = integer;
    return true;
}

template <typename Root>
bool small_integer_distance(RootSquaredDistanceValue<Root> const& value, std::uint64_t& output)
{
    if (value.literal) return false;
    std::int64_t ax, ay, bx, by;
    if (!small_integer_coordinate(value.a, 0, ax) || !small_integer_coordinate(value.a, 1, ay)
        || !small_integer_coordinate(value.b, 0, bx) || !small_integer_coordinate(value.b, 1, by)) return false;
    // Coordinates lie in [-2^30,2^30], hence |dx|,|dy|<=2^31 and the sum<=2^63.
    auto const dx = bx - ax, dy = by - ay;
    auto const x = static_cast<std::uint64_t>(dx < 0 ? -dx : dx);
    auto const y = static_cast<std::uint64_t>(dy < 0 ? -dy : dy);
    output = x * x + y * y;
    return true;
}

template <typename A, typename B>
bool compare_small_integer_distances(A const&, B const&, int&) { return false; }
template <typename R, typename S>
bool compare_small_integer_distances(RootSquaredDistanceValue<R> const& a,
    RootSquaredDistanceValue<S> const& b, int& output)
{
    std::uint64_t first, second;
    if (!small_integer_distance(a, first) || !small_integer_distance(b, second)) return false;
    output = int(first > second) - int(first < second);
    return true;
}
}

template <typename A, typename B>
std::enable_if_t<borrowed_predicates_detail::is_measure<A>::value
    && borrowed_predicates_detail::is_measure<B>::value, int>
root_compare_distance(A const& a, B const& b)
{
    if constexpr (std::is_same<A, B>::value) if (&a == &b) return 0;
    int result;
    auto first = a.bounds(), second = b.bounds();
    if (forward_error::try_compare_scalars(first.approximate, first.error, second.approximate, second.error, result)) return result;
    if (borrowed_predicates_detail::compare_small_integer_distances(a, b, result))
    {
        return result;
    }
    first = a.bounds(true); second = b.bounds(true);
    if (forward_error::try_compare_scalars(first.approximate, first.error, second.approximate, second.error, result)) return result;
    typename A::compact_pair_type compact_first;
    typename B::compact_pair_type compact_second;
    if (a.compact_pair(compact_first) && b.compact_pair(compact_second))
    {
        return fixed_pair_compare(compact_first, compact_second);
    }
    return fixed_pair_compare(a.fixed_pair(), b.fixed_pair());
}

template <typename A>
std::enable_if_t<borrowed_predicates_detail::is_measure<A>::value, int>
root_compare_distance(A const& a, int zero)
{ borrowed_predicates_detail::require_zero(zero); return a.sign(); }

template <typename A>
std::enable_if_t<borrowed_predicates_detail::is_measure<A>::value, int>
root_compare_distance(int zero, A const& a) { return -root_compare_distance(a, zero); }

// More-specific overloads coexist with static measure operators.
#define BOOST_GEOMETRY_ROOT_MEASURE_COMPARE(TYPE) \
template <typename R, typename B> \
inline auto compare_distance(TYPE<R> const& a, B const& b) -> decltype(root_compare_distance(a, b)) \
{ return root_compare_distance(a, b); } \
template <typename R> \
inline int compare_distance(int zero, TYPE<R> const& b) { return root_compare_distance(zero, b); }

BOOST_GEOMETRY_ROOT_MEASURE_COMPARE(RootSquaredDistanceValue)
BOOST_GEOMETRY_ROOT_MEASURE_COMPARE(RootSignedMeasure)
#undef BOOST_GEOMETRY_ROOT_MEASURE_COMPARE

#define BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, OP) \
template <typename R, typename B> \
inline auto operator OP(TYPE<R> const& a, B const& b) -> decltype(root_compare_distance(a, b) OP 0) \
{ return root_compare_distance(a, b) OP 0; } \
template <typename R> \
inline bool operator OP(int zero, TYPE<R> const& b) { return root_compare_distance(zero, b) OP 0; }

#define BOOST_GEOMETRY_ROOT_MEASURE_OPERATORS(TYPE) \
BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, <) \
BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, >) \
BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, <=) \
BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, >=) \
BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, ==) \
BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR(TYPE, !=)

BOOST_GEOMETRY_ROOT_MEASURE_OPERATORS(RootSquaredDistanceValue)
BOOST_GEOMETRY_ROOT_MEASURE_OPERATORS(RootSignedMeasure)
#undef BOOST_GEOMETRY_ROOT_MEASURE_OPERATORS
#undef BOOST_GEOMETRY_ROOT_MEASURE_OPERATOR

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
