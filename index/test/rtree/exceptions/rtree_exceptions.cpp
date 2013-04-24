// Boost.Geometry Index
// Unit Test

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/exceptions/test_rtree_exceptions.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

// test value exceptions
template <typename Parameters>
void test_rtree_value_exceptions(Parameters const& parameters = Parameters())
{
    typedef std::pair<bg::model::point<float, 2, bg::cs::cartesian>, throwing_value> Value;
    typedef bgi::rtree<Value, Parameters> Tree;
    typedef typename Tree::box_type B;

    throwing_value::reset_calls_counter();
    throwing_value::set_max_calls((std::numeric_limits<size_t>::max)());
    std::vector<Value> input;
    B qbox;
    generate::input<2>::apply(input, qbox);

    for ( size_t i = 0 ; i < 50 ; i += 2 )
    {
        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(10000);

        Tree tree(parameters);

        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(i);

        BOOST_CHECK_THROW( tree.insert(input.begin(), input.end()), throwing_value_copy_exception );
    }

    for ( size_t i = 0 ; i < 20 ; i += 2 )
    {
        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(10000);

        Tree tree(parameters);

        tree.insert(input.begin(), input.end());

        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(i);

        BOOST_CHECK_THROW( tree.remove(input.begin(), input.end()), throwing_value_copy_exception );
    }

    for ( size_t i = 0 ; i < 20 ; i += 2 )
    {
        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(10000);

        Tree tree(parameters);

        tree.insert(input.begin(), input.end());

        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(i);

        BOOST_CHECK_THROW( Tree tree2(tree), throwing_value_copy_exception );
    }

    for ( size_t i = 0 ; i < 20 ; i += 2 )
    {
        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(10000);

        Tree tree(parameters);
        Tree tree2(parameters);

        tree.insert(input.begin(), input.end());

        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(i);

        BOOST_CHECK_THROW(tree2 = tree, throwing_value_copy_exception );
    }
}

// test value exceptions
template <typename Parameters>
void test_rtree_elements_exceptions(Parameters const& parameters = Parameters())
{
    typedef std::pair<bg::model::point<float, 2, bg::cs::cartesian>, throwing_value> Value;
    typedef bgi::rtree<Value, Parameters> Tree;
    typedef typename Tree::box_type B;

    throwing_value::reset_calls_counter();
    throwing_value::set_max_calls((std::numeric_limits<size_t>::max)());

    std::vector<Value> input;
    B qbox;
    generate::input<2>::apply(input, qbox, 2);

    for ( size_t i = 0 ; i < 100 ; i += 2 )
    {
        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(10000);
        
        Tree tree(parameters);

        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(i);

        BOOST_CHECK_THROW( tree.insert(input.begin(), input.end()), throwing_varray_exception );
    }
    
    for ( size_t i = 0 ; i < 50 ; i += 2 )
    {
        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(10000);

        Tree tree(parameters);

        tree.insert(input.begin(), input.end());

        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(i);

        BOOST_CHECK_THROW( tree.remove(input.begin(), input.end()), throwing_varray_exception );
    }
    
    for ( size_t i = 0 ; i < 50 ; i += 2 )
    {
        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(10000);

        Tree tree(parameters);

        tree.insert(input.begin(), input.end());

        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(i);

        BOOST_CHECK_THROW( Tree tree2(tree), throwing_varray_exception );
    }

    for ( size_t i = 0 ; i < 50 ; i += 2 )
    {
        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(10000);

        Tree tree(parameters);
        Tree tree2(parameters);

        tree.insert(input.begin(), input.end());

        throwing_varray_settings::reset_calls_counter();
        throwing_varray_settings::set_max_calls(i);

        BOOST_CHECK_THROW(tree2 = tree, throwing_varray_exception );
    }
}

int test_main(int, char* [])
{
    test_rtree_value_exceptions< bgi::linear<4, 2> >();
    test_rtree_value_exceptions(bgi::dynamic_linear(4, 2));
    test_rtree_value_exceptions< bgi::quadratic<4, 2> >();
    test_rtree_value_exceptions(bgi::dynamic_quadratic(4, 2));
    test_rtree_value_exceptions< bgi::rstar<4, 2, 0, 2> >();
    test_rtree_value_exceptions(bgi::dynamic_rstar(4, 2, 0, 2));

    test_rtree_elements_exceptions< bgi::linear_throwing<4, 2> >();
    test_rtree_elements_exceptions< bgi::quadratic_throwing<4, 2> >();
    test_rtree_elements_exceptions< bgi::rstar_throwing<4, 2, 0, 2> >();

    return 0;
}
