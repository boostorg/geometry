// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSIONS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSIONS_HPP

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template
<
    typename A11, typename A12,
    typename A21, typename A22
>
using det2x2 = difference
    <
        product<A11, A22>,
        product<A12, A21>
    >;

using orient2d = det2x2
    <
        difference <_1, _5>, difference<_2, _6>,
        difference <_3, _5>, difference<_4, _6>
    >;

template
<
    typename A11, typename A12, typename A13,
    typename A21, typename A22, typename A23,
    typename A31, typename A32, typename A33
>
struct det3x3_helper
{
private:
    using minor1 = product<A11, det2x2<A22, A23, A32, A33>>;
    using minor2 = product<A21, det2x2<A12, A13, A32, A33>>;
    using minor3 = product<A31, det2x2<A12, A13, A22, A23>>;
public:
    using type = sum<minor1, sum<minor2, minor3>>;
};

template
<
    typename A11, typename A12, typename A13,
    typename A21, typename A22, typename A23,
    typename A31, typename A32, typename A33
>
using det3x3 = typename det3x3_helper
    <
        A11, A12, A13,
        A21, A22, A23,
        A31, A32, A33
    >::type;

using orient3d = det3x3
    <
        difference<_1, _10>, difference<_2, _11>, difference<_3, _12>,
        difference<_4, _10>, difference<_5, _11>, difference<_6, _12>,
        difference<_7, _10>, difference<_8, _11>, difference<_9, _12>
    >;

struct incircle_helper
{
private:
    using adx = difference<_1, _7>;
    using ady = difference<_2, _8>;
    using bdx = difference<_3, _7>;
    using bdy = difference<_4, _8>;
    using cdx = difference<_5, _7>;
    using cdy = difference<_6, _8>;
    using alift = sum<product<adx, adx>, product<ady, ady>>;
    using blift = sum<product<bdx, bdx>, product<bdy, bdy>>;
    using clift = sum<product<cdx, cdx>, product<cdy, cdy>>;
public:
    using type = det3x3
        <
            alift, adx, ady,
            blift, bdx, bdy,
            clift, cdx, cdy
        >;
};

using incircle = incircle_helper::type;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSIONS_HPP
