// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Custom point / fusion Example
// NOT FINISHED

#include <iostream>

#include <boost/fusion/adapted/struct/adapt_struct_named.hpp>
#include <boost/fusion/include/adapt_struct_named.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/next.hpp>


#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include <boost/geometry/geometries/adapted/fusion.hpp>
#include <boost/geometry/geometries/adapted/fusion_cartesian.hpp>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/util/write_dsv.hpp>


// Sample point, having x/y
struct my_2d
{
    float x,y;
};


BOOST_FUSION_ADAPT_STRUCT(my_2d, 
    (float, x)
    (float, y))



int main()
{
    my_2d p1 = {1, 5};
    my_2d p2 = {3, 4};

    std::cout << boost::fusion::at_c<1>(p1) << std::endl;

    //std::cout << boost::geometry::get<1>(p1) << std::endl;

    return 0;
}
