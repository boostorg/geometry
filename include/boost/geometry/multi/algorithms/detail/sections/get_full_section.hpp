// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP


#include <boost/assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/algorithms/detail/sections/get_full_section.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace section
{


template
<
    typename MultiGeometry,
    typename Section,
    typename Iterator,
    typename Policy
>
struct full_section_multi
{
    static inline void apply(MultiGeometry const& multi,
                Section const& section,
                Iterator& begin, Iterator& end)
    {
        BOOST_ASSERT
            (
                section.multi_index >= 0
                && section.multi_index < boost::size(multi)
            );

        Policy::apply(multi[section.multi_index], section, begin, end);
    }
};


}} // namespace detail::section
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename MultiPolygon, typename Section, typename Iterator>
struct get_full_section<multi_polygon_tag, MultiPolygon, Section, Iterator>
    : detail::section::full_section_multi
        <
            MultiPolygon,
            Section,
            Iterator,
            detail::section::full_section_polygon
                <
                    typename boost::range_value<MultiPolygon>::type,
                    Section,
                    Iterator
                >
       >
{};


} // namespace dispatch
#endif




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
