// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_CLOSEABLE_VIEW_HPP
#define BOOST_GEOMETRY_UTIL_CLOSEABLE_VIEW_HPP


#include <boost/range.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/iterators/closing_iterator.hpp>


namespace boost { namespace geometry
{



template <typename Range, bool Close>
struct closeable_view {};



template <typename Range>
struct closeable_view<Range, false>
{
    closeable_view(Range& r)
        : m_range(r)
    {}

    typedef typename boost::range_iterator<Range const>::type const_iterator;
    typedef typename boost::range_iterator<Range>::type iterator;

    const_iterator begin() const { return boost::begin(m_range); }
    const_iterator end() const { return boost::end(m_range); }

    iterator begin() { return boost::begin(m_range); }
    iterator end() { return boost::end(m_range); }
private :
    Range& m_range;
};


template <typename Range>
struct closeable_view<Range, true>
{
    explicit closeable_view(Range& r)
        : m_range(r)
    {}


    typedef closing_iterator<Range> iterator;
    typedef closing_iterator<Range const> const_iterator;

    const_iterator begin() const { return const_iterator(m_range); }
    const_iterator end() const { return const_iterator(m_range, true); }

    iterator begin() { return iterator(m_range); }
    iterator end() { return iterator(m_range, true); }
private :
    Range& m_range;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_CLOSEABLE_VIEW_HPP
