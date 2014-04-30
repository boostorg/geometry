// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_SPIKES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_SPIKES_HPP

#include <algorithm>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/closure.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>

#include <boost/geometry/views/closeable_view.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{

template <typename Iterator>
struct equal_point
{
    Iterator m_it;

    equal_point(Iterator it)
        : m_it(it)
    {}

    template <typename Point>
    inline bool operator()(Point const& p) const
    {
        return geometry::equals(p, *m_it);
    }
};

template <typename Iterator>
struct not_equal_point
{
    Iterator m_it;

    not_equal_point(Iterator it)
        : m_it(it)
    {}

    template <typename Point>
    inline bool operator()(Point const& p) const
    {
        return !geometry::equals(p, *m_it);
    }
};



template <typename Range, closure_selector Closure = closed>
struct has_spikes
{
    static inline bool apply(Range const& range)
    {
        typedef typename boost::range_iterator<Range const>::type iterator; 
        typedef typename boost::range_reverse_iterator
            <
                Range const
            >::type reverse_iterator; 

        typedef not_equal_point<iterator> not_equal;

        BOOST_ASSERT( boost::size(range) > 2 );

        iterator prev = boost::begin(range);

        iterator cur = std::find_if(prev, boost::end(range), not_equal(prev));
        BOOST_ASSERT( cur != boost::end(range) );

        iterator next = std::find_if(cur, boost::end(range), not_equal(cur));
        BOOST_ASSERT( next != boost::end(range) );

        while ( next != boost::end(range) )
        {
            if ( geometry::detail::point_is_spike_or_equal(*prev,
                                                           *next,
                                                           *cur) )
            {
                return true;
            }
            prev = cur;
            cur = next;
            next = std::find_if(cur, boost::end(range), not_equal(cur));
        }

        if ( geometry::equals(*boost::begin(range), *boost::rbegin(range)) )
        {
            iterator cur = boost::begin(range);
            reverse_iterator prev =
                std::find_if(boost::rbegin(range),
                             boost::rend(range),
                             not_equal_point
                                 <
                                     reverse_iterator
                                 >(boost::rbegin(range))
                             );
            iterator next = std::find_if(cur, boost::end(range), not_equal(cur));
            return detail::point_is_spike_or_equal(*prev, *next, *cur);
        }

        return false;
    }
};


template <typename Range>
struct has_spikes<Range, open>
{
    static inline bool apply(Range const& range)
    {
        typedef typename closeable_view<Range, open>::type ClosedRange;

        ClosedRange crange(const_cast<Range&>(range));
        return has_spikes<ClosedRange, closed>::apply(crange);
    }

};

}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_SPIKES_HPP
