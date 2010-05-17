// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_REVERSIBLE_VIEW_HPP
#define BOOST_GEOMETRY_UTIL_REVERSIBLE_VIEW_HPP


#include <boost/range.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{

enum iterate_direction { iterate_forward, iterate_reverse };


template <typename Range, iterate_direction Direction>
struct reversible_view {};



template <typename Range>
struct reversible_view<Range, iterate_forward>
{
    reversible_view(Range& r)
        : m_range(r)
    {}

    typedef typename boost::range_iterator<Range const>::type const_iterator;
    typedef typename boost::range_iterator<Range>::type iterator;

    const_iterator begin() const { return boost::begin(this->m_range); }
    const_iterator end() const { return boost::end(this->m_range); }

    iterator begin() { return boost::begin(this->m_range); }
    iterator end() { return boost::end(this->m_range); }
private :
    Range& m_range;
};


template <typename Range>
struct reversible_view<Range, iterate_reverse>
{
    reversible_view(Range& r)
        : m_range(r)
    {}

    typedef typename boost::range_reverse_iterator<Range const>::type const_iterator;
    typedef typename boost::range_reverse_iterator<Range>::type iterator;

    const_iterator begin() const { return boost::rbegin(this->m_range); }
    const_iterator end() const { return boost::rend(this->m_range); }

    iterator begin() { return boost::rbegin(this->m_range); }
    iterator end() { return boost::rend(this->m_range); }
private :
    Range& m_range;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_REVERSIBLE_VIEW_HPP
