// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_VIEWS_CLOSEABLE_VIEW_HPP
#define BOOST_GEOMETRY_VIEWS_CLOSEABLE_VIEW_HPP


#include <boost/range.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/iterators/closing_iterator.hpp>

#include <boost/geometry/views/identity_view.hpp>

namespace boost { namespace geometry
{

namespace detail
{

template <typename Range>
struct closing_view
{
    // Keep this explicit, important for nested views/ranges
    explicit inline closing_view(Range& r)
        : m_range(r)
    {}


    typedef closing_iterator<Range> iterator;
    typedef closing_iterator<Range const> const_iterator;

    inline const_iterator begin() const { return const_iterator(m_range); }
    inline const_iterator end() const { return const_iterator(m_range, true); }

    inline iterator begin() { return iterator(m_range); }
    inline iterator end() { return iterator(m_range, true); }
private :
    Range& m_range;
};

}


template <typename Range, closure_selector Close>
struct closeable_view {};



template <typename Range>
struct closeable_view<Range, closed>
{
    typedef identity_view<Range> type;
};


template <typename Range>
struct closeable_view<Range, open>
{
    typedef detail::closing_view<Range> type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_VIEWS_CLOSEABLE_VIEW_HPP
