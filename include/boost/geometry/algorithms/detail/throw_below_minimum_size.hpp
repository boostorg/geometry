// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_THROW_BELOW_MINIMUM_SIZE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_THROW_BELOW_MINIMUM_SIZE_HPP

#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/algorithms/is_below_minimum_size.hpp>

#if defined(BOOST_GEOMETRY_BELOW_MINIMUM_SIZE_NO_THROW)
#include <boost/core/ignore_unused.hpp>
#endif

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Geometry>
inline void throw_below_minimum_size(Geometry const& geometry)
{
#if ! defined(BOOST_GEOMETRY_BELOW_MINIMUM_SIZE_NO_THROW)
    if (geometry::is_below_minimum_size(geometry))
    {
        throw below_minimum_size_exception();
    }
#else
    boost::ignore_unused(geometry);
#endif
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_THROW_BELOW_MINIMUM_SIZE_HPP

