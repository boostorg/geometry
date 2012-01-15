// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_THROW_ON_EMPTY_INPUT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_THROW_ON_EMPTY_INPUT_HPP

#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Geometry>
inline void throw_on_empty_input(Geometry const& geometry)
{
#if ! defined(BOOST_GEOMETRY_EMPTY_INPUT_NO_THROW)
    if (geometry::num_points(geometry) == 0)
    {
        throw empty_input_exception();
    }
#endif
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_THROW_ON_EMPTY_INPUT_HPP

