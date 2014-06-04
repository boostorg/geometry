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
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>

#include <boost/geometry/views/closeable_view.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{

template <typename Point>
struct equal_to
{
    Point const& m_point;

    equal_to(Point const& point)
        : m_point(point)
    {}

    template <typename OtherPoint>
    inline bool operator()(OtherPoint const& other) const
    {
        return geometry::equals(m_point, other);
    }
};

template <typename Point>
struct not_equal_to
{
    Point const& m_point;

    not_equal_to(Point const& point)
        : m_point(point)
    {}

    template <typename OtherPoint>
    inline bool operator()(OtherPoint const& other) const
    {
        return !geometry::equals(other, m_point);
    }
};



template <typename Range, closure_selector Closure = closed>
struct has_spikes
{
    static inline bool apply(Range const& range)
    {
        typedef typename point_type<Range>::type point; 
        typedef typename boost::range_iterator<Range const>::type iterator; 

        typedef not_equal_to<point> not_equal;

        BOOST_ASSERT( boost::size(range) > 2 );

        iterator prev = boost::begin(range);

        iterator cur = std::find_if(prev, boost::end(range), not_equal(*prev));
        BOOST_ASSERT( cur != boost::end(range) );

        iterator next = std::find_if(cur, boost::end(range), not_equal(*cur));
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
            next = std::find_if(cur, boost::end(range), not_equal(*cur));
        }

        if ( geometry::equals(*boost::begin(range), *boost::rbegin(range)) )
        {
            iterator cur = boost::begin(range);
            typename boost::range_reverse_iterator
                <
                    Range const
                >::type prev = std::find_if(boost::rbegin(range),
                                            boost::rend(range),
                                            not_equal(*boost::rbegin(range)));
            iterator next = std::find_if(cur, boost::end(range), not_equal(*cur));
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
        typedef typename closeable_view<Range, open>::type closed_view_type;

        closed_view_type closed_range(const_cast<Range&>(range));
        return has_spikes<closed_view_type, closed>::apply(closed_range);
    }

};

}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_SPIKES_HPP
