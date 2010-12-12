// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP

// TODO rename to "range_by_section"

#include <boost/mpl/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/util/closeable_view.hpp>



namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace section
{




template <typename Range, typename Section>
struct full_section_range
{
    typedef typename closeable_view
        <
            Range const,
            closure<Range>::value
        >::type view_type;

    static inline view_type apply(Range const& range, Section const& section)
    {
        return view_type(range);
    }
};


template <typename Polygon, typename Section>
struct full_section_polygon
{
    typedef typename geometry::ring_type<Polygon>::type ring_type;
    typedef typename closeable_view
        <
            ring_type const,
            closure<ring_type>::value
        >::type view_type;

    static inline view_type apply(Polygon const& polygon, Section const& section)
    {

        ring_type const& ring = section.ring_index < 0
            ? geometry::exterior_ring(polygon)
            : geometry::interior_rings(polygon)[section.ring_index];

        return view_type(ring);
    }
};


}} // namespace detail::section
#endif


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Tag,
    typename Geometry,
    typename Section
>
struct get_full_section
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


template <typename LineString, typename Section>
struct get_full_section<linestring_tag, LineString, Section>
    : detail::section::full_section_range<LineString, Section>
{};


template <typename Ring, typename Section>
struct get_full_section<ring_tag, Ring, Section>
    : detail::section::full_section_range<Ring, Section>
{};


template <typename Polygon, typename Section>
struct get_full_section<polygon_tag, Polygon, Section>
    : detail::section::full_section_polygon<Polygon, Section>
{};


} // namespace dispatch
#endif


/*!
    \brief Get a closeable view indicated by the specified section
    \ingroup sectionalize
    \tparam Geometry type
    \tparam Section type of section to get from
    \param geometry geometry to take section of
    \param section structure with section
    \param begin begin-iterator (const iterator over points of section)
    \param end end-iterator (const iterator over points of section)
    \todo Create non-const version as well

 */
template <typename Range, typename Geometry, typename Section>
inline Range get_full_section(Geometry const& geometry, Section const& section)
{
    concept::check<Geometry const>();

    return dispatch::get_full_section
        <
            typename tag<Geometry>::type,
            Geometry,
            Section
        >::apply(geometry, section);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
