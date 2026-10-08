// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expressions.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_a.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_d.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_b.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/staged_predicate.hpp>

using namespace boost::geometry::detail::generic_robust_predicates;

template <typename CalculationType>
void test_all()
{
    using ct = CalculationType;
    using expression = orient2d;
    using filter1 = stage_a_static<expression, ct>;
    using filter2 = stage_a_almost_static<expression, ct>;
    using filter3 = stage_a_semi_static<expression, ct>;
    using filter4 = stage_b<expression, ct>;
    using filter5 = stage_d<expression, ct>;
    using staged = staged_predicate<ct, filter1, filter2, filter3, filter4, filter5>;
    staged s(1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 0., 0., 0., 0., 0., 0.);
    s.update(1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 0., 0., 0., 0., 0., 0.);
    BOOST_CHECK_EQUAL(1, s.apply(1e-20, 1e-20, 1, 1-1e-10, 1e20, 1e20));
}

int test_main(int, char* [])
{
    test_all<double>();
    return 0;
}
