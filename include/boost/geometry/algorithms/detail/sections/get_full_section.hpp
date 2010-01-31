// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/iterators/range_type.hpp>

#include <boost/geometry/geometries/segment.hpp>



namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace section
{


template <typename Range, typename Section, typename Iterator>
struct full_section_range
{
    static inline void apply(Range const& range, Section const& section,
                Iterator& begin, Iterator& end)
    {
        begin = boost::begin(range);
        end = boost::end(range);
    }
};


template <typename Polygon, typename Section, typename Iterator>
struct full_section_polygon
{
    static inline void apply(Polygon const& polygon, Section const& section,
                Iterator& begin, Iterator& end)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;
        ring_type const& ring = section.ring_index < 0
            ? geometry::exterior_ring(polygon)
            : geometry::interior_rings(polygon)[section.ring_index];

        begin = boost::begin(ring);
        end = boost::end(ring);
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
    typename Section,
    typename Iterator
>
struct get_full_section
{};


template <typename LineString, typename Section, typename Iterator>
struct get_full_section<linestring_tag, LineString, Section, Iterator>
    : detail::section::full_section_range<LineString, Section, Iterator>
{};


template <typename Ring, typename Section, typename Iterator>
struct get_full_section<ring_tag, Ring, Section, Iterator>
    : detail::section::full_section_range<Ring, Section, Iterator>
{};


template <typename Polygon, typename Section, typename Iterator>
struct get_full_section<polygon_tag, Polygon, Section, Iterator>
    : detail::section::full_section_polygon<Polygon, Section, Iterator>
{};


} // namespace dispatch
#endif


/*!
    \brief Get iterators for a specified section
    \ingroup sectionalize
    \tparam Geometry type
    \tparam Section type of section to get from
    \param geometry geometry which might be located in the neighborhood
    \param section structure with section
    \param begin begin-iterator (const iterator over points of section)
    \param end end-iterator (const iterator over points of section)
    \todo Create non-const version as well

 */
template <typename Geometry, typename Section>
inline void get_full_section(Geometry const& geometry, Section const& section,
    typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type const
        >::type& begin,
    typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type const
        >::type& end)
{
    concept::check<const Geometry>();

    typedef typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type const
        >::type iterator_type;

    dispatch::get_full_section
        <
            typename tag<Geometry>::type,
            Geometry,
            Section,
            iterator_type
        >::apply(geometry, section, begin, end);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_FULL_SECTION_HPP
