// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <array>

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expressions.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_a.hpp>

template <typename CalculationType>
void test_all()
{
    using bg::detail::generic_robust_predicates::orient3d;
    using bg::detail::generic_robust_predicates::stage_a_semi_static;
    using bg::detail::generic_robust_predicates::stage_a_almost_static;
    using bg::detail::generic_robust_predicates::stage_a_static;
    using ct = CalculationType;
    using semi_static = stage_a_semi_static<orient3d, ct>;
    BOOST_CHECK_EQUAL(1,
                      semi_static::apply(1, 0, 0,
                                         0, 1, 0,
                                         1, 1, 0,
                                         0, 0, 1));
    BOOST_CHECK_EQUAL(-1,
                      semi_static::apply(1, 0, 0,       
                                         0, 1, 0,                
                                         1, 1, 0,                
                                         0, 0, -1));
    BOOST_CHECK_EQUAL(0,
                      semi_static::apply(1, 0, 0,       
                                         0, 1, 0,                
                                         1, 1, 0,                
                                         0, 0, 0));
    stage_a_static<orient3d, ct> stat(10, 10, 10,
                                      10, 10, 10,
                                      10, 10, 10,
                                      10, 10, 10,
                                       0,  0,  0,
                                       0,  0,  0,
                                       0,  0,  0,
                                       0,  0,  0);
    BOOST_CHECK_EQUAL(1, stat.apply(1, 0, 0,
                                    0, 1, 0,
                                    1, 1, 0,
                                    0, 0, 1));

    stage_a_static<orient3d, ct> pessimistic(1e40, 1e40, 1e40,
                                             1e40, 1e40, 1e40,
                                             1e40, 1e40, 1e40,
                                             1e40, 1e40, 1e40, 
                                             0,    0,    0,  
                                             0,    0,    0, 
                                             0,    0,    0,
                                             0,    0,    0);
    BOOST_CHECK_EQUAL(bg::detail::generic_robust_predicates::sign_uncertain,
                      pessimistic.apply(1, 0, 0,
                                        0, 1, 0,
                                        1, 1, 0,
                                        0, 0, 1));
}

int test_main(int, char* [])
{
    test_all<double>();
    return 0;
}
