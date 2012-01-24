// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP


#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

// Appends points to an output range (always a ring).
// This is only the first phase, on the way specific points will be "marked"
// as a begin-point, and some segments will be intersected with all previous segments
// up to the "marked" point. If there is an intersection then points in between will
// be deleted and the intersection point will be added instead.

// This file is renamed from "intersecting_inserter" but we choose "append" now
// because that is also how the algorithm geometry::append is called. And it appends at the back (push_back)
template<typename Range>
struct buffer_appender
{
    typedef Range range_type;

    typedef typename geometry::point_type<Range>::type point_type;

    inline buffer_appender(Range& r)
        : m_range(r)
    {}

    inline void append(point_type const& point)
    {
        m_range.push_back(point);
    }

private :
    Range& m_range;
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
