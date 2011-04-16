// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[tag
//` Examine the tag of some geometry types

#include <iostream>
#include <typeinfo>
#include <boost/geometry/geometry.hpp>

int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::polygon<point_type> polygon_type;
    typedef boost::geometry::model::multi_polygon<polygon_type> mp_type;

    typedef boost::geometry::tag<point_type>::type tag1;
    typedef boost::geometry::tag<polygon_type>::type tag2;
    typedef boost::geometry::tag<mp_type>::type tag3;
    
    std::cout 
        << "tag 1: " << typeid(tag1).name() << std::endl
        << "tag 2: " << typeid(tag2).name() << std::endl
        << "tag 3: " << typeid(tag3).name() << std::endl
        ;

    return 0;
}

//]


//[tag_output
/*`
Output (in MSVC):
[pre
tag 1: struct boost::geometry::point_tag
tag 2: struct boost::geometry::polygon_tag
tag 3: struct boost::geometry::multi_polygon_tag
]
*/
//]
