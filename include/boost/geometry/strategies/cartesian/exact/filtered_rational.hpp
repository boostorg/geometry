// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FILTERED_RATIONAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FILTERED_RATIONAL_HPP

#include <boost/geometry/algorithms/detail/equals/collect_vectors.hpp>
#include <boost/geometry/algorithms/detail/overlay/output_conversion.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/policies/robustness/segment_ratio.hpp>
#include <boost/geometry/strategies/cartesian/exact/lazy_decimal_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/rational_topology.hpp>
#include <boost/geometry/strategies/detail/section.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>

namespace boost { namespace geometry
{

// Areal equals needs a canonical direction, not a Euclidean length.
// L1 normalization remains in the rational field even on oblique edges.
template <>
inline bool collected_vector_cartesian<strategies::detail::exact_cartesian::lazy_decimal_scalar>::normalize()
{
    auto const magnitude = abs(dx) + abs(dy);
    if (magnitude == 0) return false;
    dx /= magnitude;
    dy /= magnitude;
    return true;
}

// cartesian_segments uses these fractions only during intersection.
// Do not build a scaled floating approximation,
// even for ordering: the denominator can be outside the range of binary64.
template <>
class segment_ratio<strategies::detail::exact_cartesian::lazy_decimal_scalar>
{
public:
    using int_type = strategies::detail::exact_cartesian::lazy_decimal_scalar;

    segment_ratio() = default;
    segment_ratio(int_type const& n, int_type const& d) { assign(n, d); }

    void assign(int_type const& n, int_type const& d)
    {
        if (d < 0) { m_n = -n; m_d = -d; }
        else { m_n = n; m_d = d; }
    }
    int_type const& numerator() const { return m_n; }
    int_type const& denominator() const { return m_d; }
    bool is_zero() const { return m_n == 0; }
    bool is_one() const { return m_n == m_d; }
    bool on_segment() const { return m_n >= 0 && m_n <= m_d; }
    bool in_segment() const { return m_n > 0 && m_n < m_d; }
    bool on_end() const { return is_zero() || is_one(); }
    bool left() const { return m_n < 0; }
    bool right() const { return m_n > m_d; }
    bool operator<(segment_ratio const& b) const { return m_n*b.m_d < b.m_n*m_d; }
    bool operator==(segment_ratio const& b) const { return m_n*b.m_d == b.m_n*m_d; }
    template <typename T> bool possibly_collinear(T const&) const { return m_d == 0; }

    int_type edge_value() const
    {
        if (!in_segment()) return 1;
        auto t = (2*m_n - m_d) / m_d;
        return t < 0 ? -t : t;
    }
    static segment_ratio zero() { return {}; }
    static segment_ratio one() { return {1, 1}; }

private:
    int_type m_n{0}, m_d{1};
};

namespace strategies { namespace detail { namespace exact_cartesian { namespace lazy_decimal_detail
{

// The non-floating coordinate selects exact equality/order in BG math.
template <bool Approximate>
struct kernel : strategies::relate::cartesian<scalar>
{
    static constexpr bool uses_overlay_tolerance = false;
    using segment_position_type = lazy_decimal_detail::segment_position;
    template <typename P>
    using intersection_result_type = geometry::segment_intersection_points<P, geometry::segment_ratio<scalar>>;
    template <typename G>
    using ring_measure_type = strategies::detail::topological_ring_tag<point>;

    static auto segment_position() { return segment_position_type{}; }
    auto topology() const
    { return lazy_decimal_detail::topology<strategies::relate::cartesian<scalar>>{*this}; }
    static auto ring_topology()
    { return cartesian_ring_topology<scalar, point, ring_topology_detail::predicates<scalar>>{}; }

    static strategies::detail::strict_cartesian_section section() { return {}; }

    template <typename C>
    using overlay_output_type = std::conditional_t<util::is_box<C>::value,
        C, typename working_output<C>::type>;

    struct intersect_boxes
    {
        template <std::size_t D, typename A, typename B, typename Out>
        static bool axis(A const& a, B const& b, Out& out)
        {
            auto const low = (std::max)(get<min_corner, D>(a), get<min_corner, D>(b));
            auto const high = (std::min)(get<max_corner, D>(a), get<max_corner, D>(b));
            if (high < low) return false;
            using target = coordinate_type_t<Out>;
            set<min_corner, D>(out, output_point<false>::template ordinate<target>(low));
            set<max_corner, D>(out, output_point<false>::template ordinate<target>(high));
            return true;
        }

        template <typename A, typename B, typename Out>
        static bool apply(A const& a, B const& b, Out& out)
        { return axis<0>(a, b, out) && axis<1>(a, b, out); }
    };

    static auto box_intersection() { return intersect_boxes{}; }

    template <typename Output>
    static void convert_output_point(point const& source, Output& output)
    {
        if constexpr (std::is_same<Output, point>::value) output = source;
        else output_point<Approximate>::apply(source, output);
    }

    template <typename Src, typename Dst>
    static void append_output(Src const& src, Dst& dst)
    {
        geometry::detail::overlay::append_converted_output(src, dst, convert_output<Approximate>());
    }
};

template <bool Approximate, typename A, typename B, typename Apply>
decltype(auto) apply(A const& a, B const& b, Apply&& run)
{
    arena scope;
    constexpr bool borrow_a = borrow_cached_input<A>::value;
    constexpr bool borrow_b = borrow_cached_input<B>::value;
    return std::forward<Apply>(run)(operand<borrow_a>(a), operand<borrow_b>(b), kernel<Approximate>{});
}

}}}} // namespace strategies::detail::exact_cartesian::lazy_decimal_detail


}} // namespace boost::geometry

#endif
