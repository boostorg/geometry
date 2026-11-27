
// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_DECIMAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_DECIMAL_HPP

#include <boost/geometry/strategies/cartesian/exact/filtered_rational.hpp>

namespace boost { namespace geometry
{

namespace strategies { namespace relate
{

// Explicit strategy for 2D Cartesian exact_coordinate inputs. Expressions and
// their rational leaves live until the operation has exported its final result.
// Requires nearest-even IEEE binary64, gradual underflow and no reassociation
// or implicit contraction. Only final float/double output is rounded.
struct cartesian_decimal : strategies::detail::exact_cartesian::lazy_decimal_detail::kernel<false>
{
    template <typename A, typename B, typename Apply>
    static decltype(auto) with_operands(A const& a, B const& b, Apply&& run)
    {
        return strategies::detail::exact_cartesian::lazy_decimal_detail::apply<false>(
            a, b, std::forward<Apply>(run));
    }
};

}} // namespace strategies::relate
}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_DECIMAL_HPP
