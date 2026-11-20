// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STERBENZ_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STERBENZ_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

#include <boost/geometry/algorithms/for_each.hpp>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/util/error_free_transforms.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace sterbenz
{

// Only these native geometries feed segment stages directly (or through a
// boundary view). No scan for operations that cannot use the certificate.
template <typename G>
constexpr bool source = std::is_same<geometry::coordinate_type_t<G>, double>::value
    && (std::is_same<geometry::tag_t<G>, segment_tag>::value
        || std::is_same<geometry::tag_t<G>, linestring_tag>::value
        || std::is_same<geometry::tag_t<G>, ring_tag>::value
        || std::is_same<geometry::tag_t<G>, polygon_tag>::value
        || std::is_same<geometry::tag_t<G>, multi_linestring_tag>::value
        || std::is_same<geometry::tag_t<G>, multi_polygon_tag>::value);

template <typename A, typename B>
bool certify_sources(A const& a, B const& b)
{
    if (!std::numeric_limits<double>::is_iec559
        || std::numeric_limits<double>::radix != 2
        || std::numeric_limits<double>::digits != 53) return false;

    double const infinity = (std::numeric_limits<double>::infinity)();
    double xmin = infinity, ymin = infinity, xmax = -infinity, ymax = -infinity;
    auto expand = [&](auto const& point)
    {
        double const x = geometry::get<0>(point), y = geometry::get<1>(point);
        if (!std::isfinite(x) || !std::isfinite(y)) return false;
        xmin = (std::min)(xmin, x); xmax = (std::max)(xmax, x);
        ymin = (std::min)(ymin, y); ymax = (std::max)(ymax, y);
        return true;
    };
    if (!geometry::all_points_of(a, expand) || !geometry::all_points_of(b, expand))
        return false;
    auto axis = [](double lo, double hi)
    {
        if (lo > hi) return false; // empty
        double const m = lo > 0 ? lo : -hi;
        double const M = lo > 0 ? hi : -lo;
        // Use the construction expansion exponent band. Every coordinate is
        // on the 2^-202 grid. Differences are exact by Sterbenz; products
        // and their residuals are on the 2^-404 grid and have magnitude
        // <= 2^302. All nonzero intermediates are therefore normal.
        return m >= 0x1p-150 && M <= 0x1p150 && M <= 2 * m;
    };
    return axis(xmin, xmax) && axis(ymin, ymax);
}

// Only for native operands from certified sources, under the strategy's
// binary64 RN / no implicit contraction contract.
template <typename A, typename B, typename P>
int orientation(A const& a, B const& b, P const& p)
{
    using namespace arithmetic_detail;
    double const ax = geometry::get<0>(a), ay = geometry::get<1>(a);
    double const bx = rounded_sub(geometry::get<0>(b), ax);
    double const by = rounded_sub(geometry::get<1>(b), ay);
    double const px = rounded_sub(geometry::get<0>(p), ax);
    double const py = rounded_sub(geometry::get<1>(p), ay);
    double const left = rounded_mul(bx, py), right = rounded_mul(by, px);
    // Monotonic rounding preserves strict order of distinct products.
    if (left != right) return left > right ? 1 : -1;
    // No underflow: both exact products were zero.
    if (left == 0) return 0;
    using geometry::detail::precise_math::two_product_tail;
    double const left_tail = two_product_tail(bx, py, left);
    double const right_tail = two_product_tail(by, px, right);
    return (left_tail > right_tail) - (left_tail < right_tail);
}

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::sterbenz

#endif
