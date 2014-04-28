// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_OGC_DETAIL_IS_VALID_MULTI_RANGE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OGC_DETAIL_IS_VALID_MULTI_RANGE_HPP

#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>

#include <boost/geometry/algorithms/ogc/dispatch/is_valid.hpp>


namespace boost { namespace geometry { namespace ogc
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{
template <typename RangeValue>
struct is_valid_multi_range
{
    template <typename RangeIterator>
    static inline bool apply(RangeIterator first, RangeIterator beyond)
    {
        for (RangeIterator it = first; it != beyond; ++it)
        {
            if ( !dispatch::is_valid<RangeValue>::apply(*it) )
            {
                return false;
            }
        }
        return true;
    };
};


}} //namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A MultiLinestring is a MultiCurve
// A MultiCurve is simple if all of its elements are simple and the
// only intersections between any two elements occur at Points that
// are on the boundaries of both elements.
//
// Reference: OGC 06-103r4 (§6.1.8.1; Fig. 9)
template <typename MultiLinestring>
struct is_valid<MultiLinestring, multi_linestring_tag>
{
    static inline bool apply(MultiLinestring const& multilinestring)
    {
        return detail::is_valid::is_valid_multi_range
            <
                typename boost::range_value<MultiLinestring>::type
            >::apply(boost::begin(multilinestring),
                     boost::end(multilinestring));
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}}} // namespace boost::geometry::ogc


#endif // BOOST_GEOMETRY_ALGORITHMS_OGC_DETAIL_IS_VALID_MULTI_RANGE_HPP
