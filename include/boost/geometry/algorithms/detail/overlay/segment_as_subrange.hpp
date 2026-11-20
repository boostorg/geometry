// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019-2019 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SEGMENT_AS_SUBRANGE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SEGMENT_AS_SUBRANGE_HPP


#include <boost/geometry/strategies/detail/segment_as_subrange.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Segment>
using segment_as_subrange = strategies::detail::segment_as_subrange<Segment>;

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SEGMENT_AS_SUBRANGE_HPP
