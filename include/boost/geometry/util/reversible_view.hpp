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


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Range>
struct reversible_base_view
{
    Range& m_range;
    reversible_base_view(Range& r)
        : m_range(r)
    {}
};

}
#endif


enum iterate_direction { iterate_forward, iterate_reverse };


template <typename Range, iterate_direction Direction>
struct reversible_view {};



template <typename Range>
struct reversible_view<Range, iterate_forward> : detail::reversible_base_view<Range>
{
    reversible_view(Range& r)
        : reversible_base_view<Range>(r)
    {}

    typedef typename boost::range_iterator<Range const>::type const_iterator;
    typedef typename boost::range_iterator<Range>::type iterator;

    const_iterator begin() const { return boost::begin(m_range); }
    const_iterator end() const { return boost::end(m_range); }

    iterator begin() { return boost::begin(m_range); }
    iterator end() { return boost::end(m_range); }
};


template <typename Range>
struct reversible_view<Range, iterate_reverse> : detail::reversible_base_view<Range>
{
    reversible_view(Range& r)
        : reversible_base_view<Range>(r)
    {}

    typedef typename boost::range_reverse_iterator<Range const>::type const_iterator;
    typedef typename boost::range_reverse_iterator<Range>::type iterator;

    const_iterator begin() const { return boost::rbegin(m_range); }
    const_iterator end() const { return boost::rend(m_range); }

    iterator begin() { return boost::rbegin(m_range); }
    iterator end() { return boost::rend(m_range); }
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_REVERSIBLE_VIEW_HPP
