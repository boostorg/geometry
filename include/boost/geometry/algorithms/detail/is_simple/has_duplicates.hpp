// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_SIMPLE_HAS_DUPLICATES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_SIMPLE_HAS_DUPLICATES_HPP

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/policies/compare.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_simple
{

template <typename Range>
struct has_duplicates
{
    static inline bool apply(Range const& range)
    {
        typedef typename boost::range_value<Range>::type point;
        typedef typename boost::range_iterator<Range const>::type iterator;

        BOOST_ASSERT( boost::size(range) > 0 );

        if ( boost::size(range) == 1 )
        {
            return false;
        }

        geometry::equal_to<point> equal;

        iterator it = boost::begin(range);
        iterator next = ++boost::begin(range);
        for (; next != boost::end(range); ++it, ++next)
        {
            if ( equal(*it, *next) )
            {
                return true;
            }
        }
        return false;
    }
};



}} // namespace detail::is_simple
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_SIMPLE_HAS_DUPLICATES_HPP
