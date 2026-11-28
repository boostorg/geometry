// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.
// Copyright (c) 2023 Adam Wulkiewicz, Lodz, Poland.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// This file was modified by Oracle on 2021.
// Modifications copyright (c) 2021, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_ERROR_FREE_TRANSFORMS_HPP
#define BOOST_GEOMETRY_UTIL_ERROR_FREE_TRANSFORMS_HPP

#include <array>
#include <cmath>

// The following code is based on "Adaptive Precision Floating-Point Arithmetic
// and Fast Robust Geometric Predicates" by Richard Shewchuk,
// J. Discrete Comput Geom (1997) 18: 305. https://doi.org/10.1007/PL00009321

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry
{

namespace detail { namespace precise_math
{

// Use round-to-nearest binary arithmetic without reassociation, excess precision
// or implicit contraction. Products use an explicit FMA for the residual.
// Two-component results store the rounded head first and the exact tail second.
// See Theorem 6, page 6
template
<
    typename RealNumber
>
inline std::array<RealNumber, 2> fast_two_sum(RealNumber const a,
                                              RealNumber const b)
{
    RealNumber x = a + b;
    RealNumber b_virtual = x - a;
    return {{x, b - b_virtual}};
}

// See Theorem 7, page 7 - 8
template
<
    typename RealNumber
>
inline std::array<RealNumber, 2> two_sum(RealNumber const a,
                                         RealNumber const b)
{
    RealNumber x = a + b;
    RealNumber b_virtual = x - a;
    RealNumber a_virtual = x - b_virtual;
    RealNumber b_roundoff = b - b_virtual;
    RealNumber a_roundoff = a - a_virtual;
    RealNumber y = a_roundoff + b_roundoff;
    return {{ x,  y }};
}

// See bottom of page 8
template
<
    typename RealNumber
>
inline RealNumber two_diff_tail(RealNumber const a,
                                RealNumber const b,
                                RealNumber const x)
{
    RealNumber b_virtual = a - x;
    RealNumber a_virtual = x + b_virtual;
    RealNumber b_roundoff = b_virtual - b;
    RealNumber a_roundoff = a - a_virtual;
    return a_roundoff + b_roundoff;
}

// see bottom of page 8
template
<
    typename RealNumber
>
inline std::array<RealNumber, 2> two_diff(RealNumber const a,
                                          RealNumber const b)
{
    RealNumber x = a - b;
    RealNumber y = two_diff_tail(a, b, x);
    return {{ x, y }};
}

// see theorem 18, page 19
template
<
    typename RealNumber
>
inline RealNumber two_product_tail(RealNumber const a,
                                   RealNumber const b,
                                   RealNumber const x)
{
    return std::fma(a, b, -x);
}

// see theorem 18, page 19
template
<
    typename RealNumber
>
inline std::array<RealNumber, 2> two_product(RealNumber const a,
                                             RealNumber const b)
{
    RealNumber x = a * b;
    RealNumber y = two_product_tail(a, b, x);
    return {{ x , y }};
}

}} // namespace detail::precise_math
}} // namespace boost::geometry

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
