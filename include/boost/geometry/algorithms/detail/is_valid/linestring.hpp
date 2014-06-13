// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINESTRING_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_LINESTRING_HPP

#include <cstddef>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/detail/is_valid/has_spikes.hpp>

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


// returns the number of distinct points in the range;
// return values are zero, one, two, three_or_more
template <typename Range>
struct number_of_distinct_points
{
    static const std::size_t zero = 0;
    static const std::size_t one = 1;
    static const std::size_t two = 2;
    static const std::size_t three_or_more = 3;

    static inline std::size_t apply(Range const& range)
    {
        typedef typename point_type<Range>::type point;
        typedef typename boost::range_iterator<Range const>::type iterator;
        typedef typename boost::range_size<Range>::type size_type;

        size_type size = boost::size(range);

        if ( size < two )
        {
            return size;
        }

        iterator it1 =
            std::find_if(boost::begin(range),
                         boost::end(range),
                         not_equal_to<point>(range::front(range)));

        if ( it1 == boost::end(range) )
        {
            return one;
        }

        iterator it2 = 
            std::find_if(it1, boost::end(range), not_equal_to<point>(*it1));

        return
            ( it2 == boost::end(range)
              || geometry::equals(range::front(range), *it2) )
            ?
            two
            :
            three_or_more
            ;
    }
};


template <typename Linestring>
struct is_valid_linestring
{
    static inline bool apply(Linestring const& linestring,
                             bool allow_spikes)
    {
        typedef number_of_distinct_points<Linestring> num_distinct;

        std::size_t linestring_size = num_distinct::apply(linestring);

        if ( linestring_size < num_distinct::two )
        {
            return false;
        }

        if ( linestring_size == num_distinct::two )
        {
            return !geometry::equals(range::front(linestring),
                                     range::back(linestring));
        }

        return allow_spikes
            || !has_spikes<Linestring, closed>::apply(linestring);
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
