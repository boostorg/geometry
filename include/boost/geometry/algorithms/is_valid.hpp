// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_VALID_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_VALID_HPP

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>

#include <boost/geometry/algorithms/detail/is_valid/pointlike.hpp>
#include <boost/geometry/algorithms/detail/is_valid/linear.hpp>
#include <boost/geometry/algorithms/detail/is_valid/polygon.hpp>
#include <boost/geometry/algorithms/detail/is_valid/ring.hpp>
#include <boost/geometry/algorithms/detail/is_valid/segment.hpp>
#include <boost/geometry/algorithms/detail/is_valid/box.hpp>


namespace boost { namespace geometry
{


template <typename Geometry>
inline bool is_valid(Geometry const& g)
{
    return dispatch::is_valid<Geometry>::apply(g);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_IS_VALID_HPP
