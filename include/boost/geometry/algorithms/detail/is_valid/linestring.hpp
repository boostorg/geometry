// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINESTRING_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINESTRING_HPP

#include <algorithm>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/equals.hpp>

#include <boost/geometry/algorithms/detail/is_valid/has_spikes.hpp>

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


template <typename Range>
struct is_single_point
{
    static inline bool apply(Range const& range)
    {
        typedef typename boost::range_iterator<Range const>::type iterator;

        BOOST_ASSERT( boost::size(range) > 1 );

        return std::find_if(++boost::begin(range),
                            boost::end(range),
                            not_equal_point<iterator>(boost::begin(range))
                            )
            == boost::end(range);
    }
};

template <typename Range>
struct has_three_distinct_points
{
    static inline bool apply(Range const& range)
    {
        typedef typename boost::range_iterator<Range const>::type iterator;

        BOOST_ASSERT( boost::size(range) > 1 );

        iterator it1 =
            std::find_if(boost::begin(range),
                         boost::end(range),
                         not_equal_point<iterator>(boost::begin(range)));

        BOOST_ASSERT( it1 != boost::end(range) );

        iterator it2 = 
            std::find_if(it1,
                         boost::end(range),
                         not_equal_point<iterator>(it1));

        return it2 != boost::end(range)
            && !geometry::equals(*boost::begin(range), *it2);
    }
};


template <typename Linestring>
struct is_valid_linestring
{
    static inline bool apply(Linestring const& linestring,
                             bool allow_spikes)
    {
        std::size_t linestring_size = boost::size(linestring);

        if ( linestring_size < 2 )
        {
            // if it has zero or one points is cannot be valid
            return false;
        }

        if ( is_single_point<Linestring>::apply(linestring) )
        {
            // if it is an one-point linestring its dimension is 0,
            // so not valid
            return false;
        }

        BOOST_ASSERT( boost::size(linestring) > 1 );

        if ( !has_three_distinct_points<Linestring>::apply(linestring) )
        {
            return !geometry::equals(*boost::begin(linestring),
                                     *--boost::end(linestring));
        }

        return allow_spikes || !has_spikes<Linestring>::apply(linestring);
    }
};


}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A linestring is a curve.
// A curve is 1-dimensional so it has to have at least two distinct
// points.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// There is an open issue as to whether spikes are allowed for
// linestrings; here we pass this as an additional parameter: allow_spikes
// If allow_spikes is set to true, spikes are allowed, false otherwise.
// By default, spikes are disallowed
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Linestring>
struct is_valid<Linestring, linestring_tag>
{
    static inline bool apply(Linestring const& linestring,
                             bool allow_spikes = false)
    {
        return detail::is_valid::is_valid_linestring
            <
                Linestring
            >::apply(linestring, allow_spikes);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINESTRING_HPP
