// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_MULTI_LINESTRING_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_MULTI_LINESTRING_HPP

#include <boost/range.hpp>

#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>
#include <boost/geometry/algorithms/dispatch/is_valid.hpp>


namespace boost { namespace geometry
{


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
        return detail::check_iterator_range
            <
                dispatch::is_valid
                    <
                        typename boost::range_value<MultiLinestring>::type
                    >
            >::apply(boost::begin(multilinestring),
                     boost::end(multilinestring));
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_MULTI_LINESTRING_HPP
