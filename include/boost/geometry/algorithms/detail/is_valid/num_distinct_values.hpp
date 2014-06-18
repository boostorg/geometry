// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_NUM_DISTINCT_VALUES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_NUM_DISTINCT_VALUES_HPP

#include <cstddef>

#include <algorithm>

#include <boost/range.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


// returns the number of distinct values in the range;
// return values are 0u through N, where N corresponds to N or
// more distinct values
template
<
    typename Range,
    std::size_t N,
    bool AllowDuplicates /* true */,
    typename NotEqualTo
>
struct number_of_distinct_values
{
    static inline std::size_t apply(Range const& range)
    {
        typedef typename boost::range_iterator<Range const>::type iterator;

        std::size_t const size = boost::size(range);

        if ( size < 2u )
        {
            return (size < N) ? size : N;
        }

        iterator current = boost::begin(range);
        std::size_t counter(0);
        do
        {
            ++counter;
            iterator next = std::find_if(current,
                                         boost::end(range),
                                         NotEqualTo(*current));
            current = next;
        }
        while ( current != boost::end(range) && counter <= N );

        return counter;
    }
};


template <typename Range, std::size_t N, typename NotEqualTo>
struct number_of_distinct_values<Range, N, false, NotEqualTo>
{
    static inline std::size_t apply(Range const& range)
    {
        std::size_t const size = boost::size(range);
        return (size < N) ? size : N;
    }
};


}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_NUM_DISTINCT_VALUES_HPP
