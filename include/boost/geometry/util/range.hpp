// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_UTIL_RANGE_HPP
#define BOOST_GEOMETRY_UTIL_RANGE_HPP

#include <algorithm>

#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/size.hpp>

#include <boost/geometry/core/mutable_range.hpp>

namespace boost { namespace geometry { namespace range {

// NOTE: For SinglePassRanges at could iterate over all elements until the i-th element is met.

/*!
\brief Short utility to conveniently return an element of a RandomAccessRange.
\ingroup utility
*/
template <typename RandomAccessRange>
inline typename boost::range_value<RandomAccessRange const>::type const&
at(RandomAccessRange const& rng,
   typename boost::range_size<RandomAccessRange const>::type i)
{
    BOOST_RANGE_CONCEPT_ASSERT(( boost::RandomAccessRangeConcept<RandomAccessRange const> ));
    BOOST_ASSERT(i < boost::size(rng));
    return *(boost::begin(rng) + i);
}

/*!
\brief Short utility to conveniently return an element of a RandomAccessRange.
\ingroup utility
*/
template <typename RandomAccessRange>
inline typename boost::range_value<RandomAccessRange>::type &
at(RandomAccessRange & rng,
   typename boost::range_size<RandomAccessRange>::type i)
{
    BOOST_RANGE_CONCEPT_ASSERT(( boost::RandomAccessRangeConcept<RandomAccessRange> ));
    BOOST_ASSERT(i < boost::size(rng));
    return *(boost::begin(rng) + i);
}

/*!
\brief Short utility to conveniently return the front element of a Range.
\ingroup utility
*/
template <typename Range>
inline typename boost::range_value<Range>::type const&
front(Range const& rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    return *boost::begin(rng);
}

/*!
\brief Short utility to conveniently return the front element of a Range.
\ingroup utility
*/
template <typename Range>
inline typename boost::range_value<Range>::type &
front(Range & rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    return *boost::begin(rng);
}

// NOTE: For SinglePassRanges back() could iterate over all elements until the last element is met.

/*!
\brief Short utility to conveniently return the back element of a BidirectionalRange.
\ingroup utility
*/
template <typename BidirectionalRange>
inline typename boost::range_value<BidirectionalRange>::type const&
back(BidirectionalRange const& rng)
{
    BOOST_RANGE_CONCEPT_ASSERT(( boost::BidirectionalRangeConcept<BidirectionalRange const> ));
    BOOST_ASSERT(!boost::empty(rng));
    return *(--boost::end(rng));
}

/*!
\brief Short utility to conveniently return the back element of a BidirectionalRange.
\ingroup utility
*/
template <typename BidirectionalRange>
inline typename boost::range_value<BidirectionalRange>::type &
back(BidirectionalRange & rng)
{
    BOOST_RANGE_CONCEPT_ASSERT(( boost::BidirectionalRangeConcept<BidirectionalRange> ));
    BOOST_ASSERT(!boost::empty(rng));
    return *(--boost::end(rng));
}


/*!
\brief Short utility to conveniently clear a mutable range.
       It uses traits::clear<>.
\ingroup utility
*/
template <typename Range>
inline void clear(Range & rng)
{
    // NOTE: this trait is probably not needed since it could be implemented using resize()
    geometry::traits::clear<Range>::apply(rng);
}

/*!
\brief Short utility to conveniently insert a new element at the end of a mutable range.
       It uses boost::geometry::traits::push_back<>.
\ingroup utility
*/
template <typename Range>
inline void push_back(Range & rng,
                      typename boost::range_value<Range>::type const& value)
{
    geometry::traits::push_back<Range>::apply(rng, value);
}

/*!
\brief Short utility to conveniently resize a mutable range.
       It uses boost::geometry::traits::resize<>.
\ingroup utility
*/
template <typename Range>
inline void resize(Range & rng,
                   typename boost::range_size<Range>::type new_size)
{
    geometry::traits::resize<Range>::apply(rng, new_size);
}


/*!
\brief Short utility to conveniently remove an element from the back of a mutable range.
       It uses resize().
\ingroup utility
*/
template <typename Range>
inline void pop_back(Range & rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    range::resize(rng, boost::size(rng) - 1);
}

/*!
\brief Short utility to conveniently remove an element from a mutable range.
       It uses std::copy() and resize(). It requires mutable iterator as parameter.
\ingroup utility
*/
template <typename Range>
inline typename boost::range_iterator<Range>::type
erase(Range & rng,
      typename boost::range_iterator<Range>::type it)
{
    BOOST_ASSERT(!boost::empty(rng));
    BOOST_ASSERT(it != boost::end(rng));

    typename boost::range_iterator<Range>::type
        next = it;
    ++next;

    std::copy(next, boost::end(rng), it);
    range::resize(rng, boost::size(rng) - 1);

    // NOTE: assuming that resize() doesn't invalidate the iterators
    return it;
}

/*!
\brief Short utility to conveniently remove a range of elements from a mutable range.
       It uses std::copy() and resize(). It requires mutable iterators as parameters.
\ingroup utility
*/
template <typename Range>
inline typename boost::range_iterator<Range>::type
erase(Range & rng,
      typename boost::range_iterator<Range>::type first,
      typename boost::range_iterator<Range>::type last)
{
    typename std::iterator_traits
        <
            typename boost::range_iterator<Range>::type
        >::difference_type const diff = std::distance(first, last);
    BOOST_ASSERT(diff >= 0);

    std::size_t const count = static_cast<std::size_t>(diff);
    BOOST_ASSERT(count <= boost::size(rng));
    
    if ( count > 0 )
    {
        std::copy(last, boost::end(rng), first);
        range::resize(rng, boost::size(rng) - count);
    }

    // NOTE: assuming that resize() doesn't invalidate the iterators
    return first;
}

}}} // namespace boost::geometry::range

#endif // BOOST_GEOMETRY_UTIL_RANGE_HPP
