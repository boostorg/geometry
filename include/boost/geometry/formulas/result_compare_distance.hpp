// Boost.Geometry

// Copyright (c) 2015-2016 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_RESULT_COMPARE_DISTANCE_HPP
#define BOOST_GEOMETRY_FORMULAS_RESULT_COMPARE_DISTANCE_HPP


namespace boost { namespace geometry { namespace formula
{

template <typename T>
struct result_compare_distance
{
    result_compare_distance()
        : value(0)
    {}

    T value;
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_RESULT_INVERSE_HPP
