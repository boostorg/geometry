// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_GET_SECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_GET_SECTION_HPP


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/iterators/range_type.hpp>

#include <boost/geometry/geometries/segment.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename Geometry, typename Section>
struct get_section
{
    typedef typename boost::range_const_iterator
        <
            typename geometry::range_type<Geometry>::type
        >::type iterator_type;

    static inline void apply(Geometry const& geometry, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        begin = boost::begin(geometry) + section.begin_index;
        end = boost::begin(geometry) + section.end_index + 1;
    }
};

template <typename Polygon, typename Section>
struct get_section<polygon_tag, Polygon, Section>
{
    typedef typename boost::range_const_iterator
        <
            typename geometry::range_type<Polygon>::type
        >::type iterator_type;

    static inline void apply(Polygon const& polygon, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;
        ring_type const& ring = section.ring_index < 0
            ? geometry::exterior_ring(polygon)
            : geometry::interior_rings(polygon)[section.ring_index];

        begin = boost::begin(ring) + section.begin_index;
        end = boost::begin(ring) + section.end_index + 1;
    }
};

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
inline void get_section(Geometry const& geometry, Section const& section,
    typename boost::range_const_iterator
        <
            typename geometry::range_type<Geometry>::type
        >::type& begin,
    typename boost::range_const_iterator
        <
            typename geometry::range_type<Geometry>::type
        >::type& end)
{
    concept::check<const Geometry>();

    dispatch::get_section
        <
            typename tag<Geometry>::type,
            Geometry,
            Section
        >::apply(geometry, section, begin, end);
}




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_GET_SECTION_HPP
