// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP


#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/core/ring_type.hpp>
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
    typename Policy
>
struct full_section_multi
{
    typedef typename geometry::ring_type<MultiGeometry>::type ring_type;
    typedef typename closeable_view
        <
            ring_type const, closure<MultiGeometry>::value
        >::type view_type;

    static inline view_type apply(MultiGeometry const& multi,
                Section const& section)
    {
        BOOST_ASSERT
            (
                section.multi_index >= 0
                && section.multi_index < boost::size(multi)
            );

        return Policy::apply(multi[section.multi_index], section);
    }
};


}} // namespace detail::section
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename MultiPolygon, typename Section>
struct get_full_section<multi_polygon_tag, MultiPolygon, Section>
    : detail::section::full_section_multi
        <
            MultiPolygon,
            Section,
            detail::section::full_section_polygon
                <
                    typename boost::range_value<MultiPolygon>::type,
                    Section
                >
       >
{};


} // namespace dispatch
#endif




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
