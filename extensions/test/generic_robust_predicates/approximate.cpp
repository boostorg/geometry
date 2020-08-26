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

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/approximate.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

template <typename CalculationType>
void test_all()
{
    using bg::detail::generic_robust_predicates::_1;
    using bg::detail::generic_robust_predicates::_2;
    using bg::detail::generic_robust_predicates::_3;
    using bg::detail::generic_robust_predicates::_4;
    using bg::detail::generic_robust_predicates::sum;
    using bg::detail::generic_robust_predicates::difference;
    using bg::detail::generic_robust_predicates::product;
    using bg::detail::generic_robust_predicates::max;
    using bg::detail::generic_robust_predicates::abs;
    using bg::detail::generic_robust_predicates::post_order;
    using bg::detail::generic_robust_predicates::approximate_value;
    using bg::detail::generic_robust_predicates::get_approx;
    using bg::detail::generic_robust_predicates::approximate_interim;
    using bg::detail::generic_robust_predicates::argument_list;
    using ct = CalculationType;
    ct r1 = approximate_value<sum<_1, _2>, ct>(
            std::array<ct, 2>{1.0, 2.0});
    BOOST_CHECK_EQUAL(3.0, r1);
    ct r2 = approximate_value<max<abs<_1>, abs<_2>>, ct>(
            std::array<ct, 2>{-10.0, 2.0});
    BOOST_CHECK_EQUAL(10.0, r2);

    using expression = product
        <
            difference<_1, _2>,
            difference<_3, _4>
        >;
    using evals = post_order<expression>;
    using arg_list_input = argument_list<4>;
    using arg_list = boost::mp11::mp_list<evals, arg_list_input>;
    std::array<ct, boost::mp11::mp_size<evals>::value> r;
    std::array<ct, 4> input {5.0, 3.0, 2.0, 8.0};
    approximate_interim<evals, arg_list, ct>(r, input);
    ct r3 = get_approx<typename expression::left, arg_list, ct>(r, input);
    BOOST_CHECK_EQUAL(2.0, r3);
    ct r4 = get_approx<typename expression::right, arg_list, ct>(r, input);
    BOOST_CHECK_EQUAL(-6.0, r4);
    ct r5 = get_approx<expression, arg_list, ct>(r, input);
    BOOST_CHECK_EQUAL(-12.0, r5);
}


int test_main(int, char* [])
{
    test_all<double>();
    return 0;
}
