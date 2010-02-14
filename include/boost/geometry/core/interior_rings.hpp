// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_CORE_INTERIOR_RINGS_HPP
#define BOOST_GEOMETRY_CORE_INTERIOR_RINGS_HPP

#include <cstddef>

#include <boost/range/functions.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/util/add_const_if_c.hpp>

namespace boost { namespace geometry
{

namespace traits
{

/*!
    \brief Traits class indicating interior container type of a polygon
    \details defines inner container type, so the container containing
            the interior rings
    \ingroup traits
    \par Geometries:
        - polygon
    \par Specializations should provide:
        - typedef X type ( e.g. std::vector&lt;myring&lt;P&gt;&gt; )
    \tparam Geometry geometry
*/
template <typename Geometry>
struct interior_type
{};


/*!
    \brief Traits class defining access to interior_rings of a polygon
    \details defines access (const and non const) to interior ring
    \ingroup traits
    \par Geometries:
        - polygon
    \par Specializations should provide:
        - static inline INTERIOR& get(POLY&)
        - static inline const INTERIOR& get(const POLY&)
    \tparam Geometry geometry
*/
template <typename Geometry>
struct interior_rings
{};


} // namespace traits




#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


template <typename GeometryTag, typename Geometry>
struct interior_type
{};


template <typename Polygon>
struct interior_type<polygon_tag, Polygon>
{
    typedef typename traits::interior_type
        <
            typename boost::remove_const<Polygon>::type
        >::type type;
};


template
<
    typename GeometryTag,
    typename Geometry,
    bool IsConst
>
struct interior_rings {};


template <typename Polygon, bool IsConst>
struct interior_rings<polygon_tag, Polygon, IsConst>
{
    static inline typename add_const_if_c
        <
            IsConst,
            typename interior_type
                    <
                        polygon_tag,
                        Polygon
                    >::type
        >::type& apply(typename add_const_if_c
            <
                IsConst,
                Polygon
            >::type& polygon)
    {
        return traits::interior_rings
            <
                typename boost::remove_const<Polygon>::type
            >::get(polygon);
    }
};


template <typename Tag, typename Geometry>
struct num_interior_rings
{};


template <typename Ring>
struct num_interior_rings<ring_tag, Ring>
{
    static inline std::size_t apply(Ring const& ring)
    {
        return 0;
    }
};


template <typename Polygon>
struct num_interior_rings<polygon_tag, Polygon>
{
    static inline std::size_t apply(Polygon const& polygon)
    {
        return boost::size(interior_rings
            <
                polygon_tag, Polygon, true
            >::apply(polygon));
    }

};


} // namespace core_dispatch
#endif


/*!
    \brief Meta-function defining container type
        of inner rings of (multi)polygon geometriy
    \details the interior rings should be organized as a container
        (std::vector, std::deque, boost::array) with
        boost range support. This meta function defines the type
            of that container.
    \ingroup core
*/
template <typename Geometry>
struct interior_type
{
    typedef typename core_dispatch::interior_type
        <
            typename tag<Geometry>::type,
            Geometry
        >::type type;
};


/*!
    \brief Function to get the interior rings of a polygon (non const version)
    \ingroup access
    \note OGC compliance: instead of InteriorRingN
    \tparam P polygon type
    \param polygon the polygon to get the interior rings from
    \return a reference to the interior rings
*/
template <typename Polygon>
inline typename interior_type<Polygon>::type& interior_rings(Polygon& polygon)
{
    return core_dispatch::interior_rings
        <
            typename tag<Polygon>::type,
            Polygon,
            false
        >::apply(polygon);
}


/*!
    \brief Function to get the interior rings of a polygon (const version)
    \ingroup access
    \note OGC compliance: instead of InteriorRingN
    \tparam P polygon type
    \param polygon the polygon to get the interior rings from
    \return a const reference to the interior rings
*/
template <typename Polygon>
inline const typename interior_type<Polygon>::type& interior_rings(
            Polygon const& polygon)
{
    return core_dispatch::interior_rings
        <
            typename tag<Polygon>::type,
            Polygon,
            true
        >::apply(polygon);
}



/*!
    \brief Function to get the number of interior rings of a polygon
    \ingroup access
    \note Defined by OGC as "numInteriorRing". To be consistent with "numPoints"
        letter "s" is appended
    \note Can be used for any geometry, returning 0 for geometries not having
        interior rings
    \tparam Geometry geometry type
    \param geometry the polygon or other geometry
    \return the number of interior rings of the geometry
*/
template <typename Geometry>
inline std::size_t num_interior_rings(Geometry const& geometry)
{
    return core_dispatch::num_interior_rings
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_INTERIOR_RINGS_HPP
