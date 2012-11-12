// Boost.Geometry Index
// Unit Test

// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/test_rtree_exceptions.hpp>

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
    generate_input<2>::apply(input, qbox);

    for ( size_t i = 10 ; i < 100 ; i += 5 )
    {
        throwing_value::reset_calls_counter();
        throwing_value::set_max_calls(i);

        Tree tree(parameters);
        
        BOOST_CHECK_THROW( tree.insert(input.begin(), input.end()), throwing_value_copy_exception );
    }
}

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

int test_main(int, char* [])
{
    test_rtree_value_exceptions< bgi::linear<4, 2> >();
    test_rtree_value_exceptions(bgi::runtime::linear(4, 2));
    test_rtree_value_exceptions< bgi::quadratic<4, 2> >();
    test_rtree_value_exceptions(bgi::runtime::quadratic(4, 2));
    test_rtree_value_exceptions< bgi::rstar<4, 2> >();
    test_rtree_value_exceptions(bgi::runtime::rstar(4, 2));

    //std::cout << "----------------------------------------\n";
    //_CrtDumpMemoryLeaks();
    //std::cin.get();

    return 0;
}
