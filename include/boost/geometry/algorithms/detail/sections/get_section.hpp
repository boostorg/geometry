// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_SECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_SECTION_HPP

// OBSOLETE

/***

#include <boost/range.hpp>



namespace boost { namespace geometry
{



    \brief Get iterators for a specified section
    \ingroup sectionalize
    \tparam Range type
    \tparam Section type of section to get from
    \param range range (retrieved by "range_by_section") to take section of
    \param section structure with section
    \param begin begin-iterator (const iterator over points of section)
    \param end end-iterator (const iterator over points of section)

template <typename Range, typename Section>
inline void get_section(Range const& range, Section const& section,
    typename boost::range_iterator<Range const>::type& begin,
    typename boost::range_iterator<Range const>::type& end)
{
    begin = boost::begin(range) + section.begin_index;
    end = boost::begin(range) + section.end_index + 1;
}


// non const version
template <typename Range, typename Section>
inline void get_section(Range& range, Section const& section,
    typename boost::range_iterator<Range>::type& begin,
    typename boost::range_iterator<Range>::type& end)
{
    begin = boost::begin(range) + section.begin_index;
    end = boost::begin(range) + section.end_index + 1;
}

}} // namespace boost::geometry

****/

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_SECTION_HPP
