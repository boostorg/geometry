// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_DECIMAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_DECIMAL_HPP

#include <boost/geometry/algorithms/detail/equals/collect_vectors.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/policies/robustness/segment_ratio.hpp>
#include <boost/geometry/strategies/cartesian/exact/lazy_decimal_geometry.hpp>
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

// cartesian_segments and the stored turn fraction both select segment_ratio
// from the coordinate type. Do not build a scaled floating approximation,
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

// BG's ordinary areal turn extraction, ordering, winding and matrix analyser.
// The non-floating coordinate selects exact equality/order in BG math. Section
// pruning needs no epsilon: its box coordinates are lazy exact values too.
struct kernel : strategies::relate::cartesian<scalar>
{
    static strategies::detail::strict_cartesian_section section() { return {}; }

    template <typename C> using overlay_output_type = typename working_output<C>::type;

    template <typename Src, typename Dst>
    static void append_output(Src const& src, Dst& dst) { append(src, dst); }
};

}}}} // namespace strategies::detail::exact_cartesian::lazy_decimal_detail

namespace strategies { namespace relate
{

// Explicit strategy for 2D Cartesian exact_coordinate inputs. Expressions and
// their rational leaves live until the operation has exported its final result.
// Requires nearest-even IEEE binary64, gradual underflow and no reassociation
// or implicit contraction. Only final float/double output is rounded.
struct cartesian_decimal : strategies::detail::exact_cartesian::lazy_decimal_detail::kernel
{
    template <typename A, typename B, typename Apply>
    static decltype(auto) with_operands(A const& a, B const& b, Apply&& run)
    {
        namespace impl = strategies::detail::exact_cartesian::lazy_decimal_detail;
        impl::arena scope;
        return run(impl::operand(a), impl::operand(b), impl::kernel());
    }
};

}} // namespace strategies::relate
}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_DECIMAL_HPP
