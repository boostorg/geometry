// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP

#include <boost/geometry/algorithms/detail/is_simple/always_simple.hpp>
#include <boost/geometry/algorithms/detail/is_simple/areal.hpp>
#include <boost/geometry/algorithms/detail/is_simple/linear.hpp>
#include <boost/geometry/algorithms/detail/is_simple/multipoint.hpp>

#include <boost/geometry/algorithms/dispatch/is_simple.hpp>


namespace boost { namespace geometry
{


template <typename Geometry>
inline bool is_simple(Geometry const& g)
{
    return dispatch::is_simple<Geometry>::apply(g);
}


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
