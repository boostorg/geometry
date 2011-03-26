// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Quickbook Example

//[length_with_strategy
//`The following example shows the length measured over a sphere, expressed in kilometers. To do that the radius of the sphere must be specified in the constructor of the strategy.

#include <iostream>
#include <boost/geometry/geometry.hpp>

int main()
{
    using namespace boost::geometry;
    typedef model::point<float, 2, cs::spherical<degree> > P;
    model::linestring<P> line;
    line.push_back(P(2, 41));
    line.push_back(P(2, 48));
    line.push_back(P(5, 52));
    double const mean_radius = 6371.0; /*< [@http://en.wikipedia.org/wiki/Earth_radius Wiki]  >*/
    std::cout << "length is "
        << length(line, strategy::distance::haversine<P>(mean_radius) )
        << " kilometers " << std::endl;

    return 0;
}

//]


//[length_with_strategy_output
/*`
Output:
[pre
length is 1272.03 kilometers
]
*/
//]
