// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_LINEAR_FORMULA_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_LINEAR_FORMULA_HPP

#include <array>
#include <cstddef>
#include <utility>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace linear_formula
{

template <typename T>
using xy = std::array<T, 2>;

// Keep the two products available to the filter or the exact-tail evaluator.
template <bool Dot = false, typename A, typename B, typename Multiply>
inline auto bilinear_products(A const& a, B const& b, Multiply multiply)
{
    constexpr std::size_t i = Dot ? 0 : 1;
    using product = decltype(multiply(a[0], b[i]));
    return xy<product>{{multiply(a[0], b[i]), multiply(a[1], b[1 - i])}};
}

template <typename A, typename B, typename Subtract>
inline auto difference(A const& a, B const& b, Subtract subtract)
{
    using scalar = decltype(subtract(b[0], a[0]));
    return xy<scalar>{{subtract(b[0], a[0]), subtract(b[1], a[1])}};
}

// A recipe with one late-bound operand. Keep the origin in its original
// representation (including any certificate), not as an evaluated value.
struct orientation
{
private:
    template <typename Ops, typename A, typename U, typename P>
    static auto evaluate_from(Ops const& ops, A const& a, U const& u, P const& p)
    { return ops.cross(u, ops.difference(a, p)); }

public:
    template <typename Ops, typename A, typename B, typename P>
    static auto evaluate(Ops const& ops, A const& a, B const& b, P const& p)
    {
        return evaluate_from(ops, a, ops.difference(a, b), p);
    }

    template <typename Ops, typename A, typename U>
    struct bound
    {
        Ops ops;
        A origin;
        U direction;

        template <typename P>
        auto evaluate(P const& p) const
        { return orientation::evaluate_from(ops, origin, direction, p); }
    };

    template <typename Ops, typename A, typename B>
    static auto prepare(Ops const& ops, A const& a, B const& b)
    {
        auto direction = ops.difference(a, b);
        return bound<Ops, A, decltype(direction)>{ops, a, std::move(direction)};
    }
};

template <typename Ops, typename A, typename B, typename P>
inline auto direction(Ops const& ops, A const& a, B const& b, P const& p)
{ return ops.dot(ops.difference(a, b), ops.difference(b, p)); }

template <typename Ops, typename A, typename B>
inline auto squared_distance(Ops const& ops, A const& a, B const& b)
{
    auto const delta = ops.difference(a, b);
    return ops.dot(delta, delta);
}

template <typename U, typename T, typename D>
struct intersection_terms
{
    U direction;
    T numerator;
    D denominator;
};

// A + (T / D) * U, where U = B-A, T = (C-A) x (D-C), D = U x (D-C).
// Lowering the two crosses can produce heads, magnitudes, or exact scalars.
template <typename Ops, typename A, typename B, typename C, typename D>
inline auto affine_intersection(Ops const& ops, A const& a, B const& b,
                               C const& c, D const& d)
{
    auto u = ops.difference(a, b);
    auto const v = ops.difference(c, d);
    auto numerator = ops.cross(ops.difference(a, c), v);
    auto denominator = ops.cross(u, v);
    return intersection_terms<decltype(u), decltype(numerator), decltype(denominator)>
        {std::move(u), std::move(numerator), std::move(denominator)};
}

template <typename Point, typename Ratio, typename Direction, typename MultiplyAdd>
inline auto affine_point(Point const& origin, Ratio const& parameter,
                         Direction const& direction, MultiplyAdd multiply_add)
{
    using scalar = decltype(multiply_add(parameter, direction[0], origin[0]));
    return xy<scalar>{{multiply_add(parameter, direction[0], origin[0]),
                       multiply_add(parameter, direction[1], origin[1])}};
}

template <typename T>
struct arithmetic_ops
{
    static inline xy<T> difference(xy<T> const& a, xy<T> const& b)
    { return linear_formula::difference(a, b, [](T const& x, T const& y) -> T { return x - y; }); }

    static inline T cross(xy<T> const& a, xy<T> const& b)
    {
        auto const terms = bilinear_products(a, b, [](T const& x, T const& y) -> T { return x * y; });
        return terms[0] - terms[1];
    }

    static inline T multiply_add(T const& a, T const& b, T const& c) { return a * b + c; }
};

// Projective rewrite of the same construction. Preserve (X,Y,W) without
// division; the caller owns the binary scale and sign normalization.
template <typename Linear, typename Constant>
struct line { Linear a, b; Constant c; };

template <typename Ops, typename P>
inline auto through(Ops const& ops, P const& a, P const& b)
{
    auto normal_x = ops.subtract(a[1], b[1]);
    auto normal_y = ops.subtract(b[0], a[0]);
    auto constant = ops.product_difference(a[0], b[1], a[1], b[0]);
    return line<decltype(normal_x), decltype(constant)>
        {std::move(normal_x), std::move(normal_y), std::move(constant)};
}

template <typename X, typename W>
struct projective_point { X x, y; W w; };

template <typename Ops, typename L>
inline auto meet(Ops const& ops, L const& a, L const& b)
{
    auto x = ops.product_difference(a.b, b.c, a.c, b.b);
    auto y = ops.product_difference(a.c, b.a, a.a, b.c);
    auto w = ops.product_difference(a.a, b.b, a.b, b.a);
    return projective_point<decltype(x), decltype(w)>{std::move(x), std::move(y), std::move(w)};
}

}}}}} // namespace boost::geometry::strategies::detail::linear_formula

#endif
