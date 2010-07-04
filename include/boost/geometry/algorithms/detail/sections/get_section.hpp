// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_SECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_SECTION_HPP


#include <boost/range.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/iterators/range_type.hpp>
#include <boost/geometry/util/add_const_if_c.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// Generic case (linestring/linear ring)
template <typename Tag, typename Geometry, typename Section, bool IsConst>
struct get_section
{
    typedef typename add_const_if_c
        <
            IsConst,
            Geometry
        >::type geometry_type;

    typedef typename boost::range_iterator
        <
            typename add_const_if_c
            <
                IsConst,
                typename geometry::range_type<Geometry>::type
            >::type 
        >::type iterator_type;

    static inline void apply(geometry_type& geometry, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        begin = boost::begin(geometry) + section.begin_index;
        end = boost::begin(geometry) + section.end_index + 1;
    }
};

template <typename Polygon, typename Section, bool IsConst>
struct get_section<polygon_tag, Polygon, Section, IsConst>
{
    typedef typename add_const_if_c
        <
            IsConst,
            Polygon
        >::type polygon_type;

    typedef typename boost::range_iterator
        <
            typename add_const_if_c
            <
                IsConst,
                typename geometry::range_type<Polygon>::type
            >::type 
        >::type iterator_type;

    static inline void apply(polygon_type& polygon, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        typename add_const_if_c
            <
                IsConst,
                typename geometry::ring_type<Polygon>::type
            >::type& ring = section.ring_index < 0
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
 */
template <typename Geometry, typename Section>
inline void get_section(Geometry const& geometry, Section const& section,
    typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type const
        >::type& begin,
    typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type const
        >::type& end)
{
    concept::check<Geometry const>();

    dispatch::get_section
        <
            typename tag<Geometry>::type,
            Geometry,
            Section,
            true
        >::apply(geometry, section, begin, end);
}


// non const version
template <typename Geometry, typename Section>
inline void get_section(Geometry& geometry, Section const& section,
    typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type
        >::type& begin,
    typename boost::range_iterator
        <
            typename geometry::range_type<Geometry>::type
        >::type& end)
{
    concept::check<Geometry>();

    dispatch::get_section
        <
            typename tag<Geometry>::type,
            Geometry,
            Section,
            false
        >::apply(geometry, section, begin, end);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_GET_SECTION_HPP
