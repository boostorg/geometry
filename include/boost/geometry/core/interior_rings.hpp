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

#include <boost/mpl/assert.hpp>
#include <boost/range.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/interior_type.hpp>
#include <boost/geometry/util/add_const_if_c.hpp>
#include <boost/geometry/util/ensure_const_reference.hpp>

namespace boost { namespace geometry
{

namespace traits
{


/*!
    \brief Traits class defining access to interior_rings of a polygon
    \details defines access (const and non const) to interior ring
    \ingroup traits
    \par Geometries:
        - polygon
    \par Specializations should provide:
        - static inline INTERIOR& get(POLY&)
        - static inline const INTERIOR& get(POLY const&)
    \tparam Geometry geometry
*/
template <typename Geometry>
struct interior_rings
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


} // namespace traits




#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template
<
    typename GeometryTag,
    typename Geometry
>
struct interior_rings {};


template <typename Polygon>
struct interior_rings<polygon_tag, Polygon>
{
    static
    typename geometry::interior_return_type<Polygon>::type
                apply(Polygon& polygon)
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
    static inline std::size_t apply(Ring const& )
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
                polygon_tag, Polygon const
            >::apply(polygon));
    }

};


} // namespace core_dispatch
#endif



/*!
\brief Function to get the interior rings of a polygon (non const version)
\ingroup interior_rings
\note OGC compliance: instead of InteriorRingN
\tparam Polygon polygon type
\param polygon the polygon to get the interior rings from
\return the interior rings (possibly a reference)
*/

template <typename Polygon>
inline typename interior_return_type<Polygon>::type interior_rings(Polygon& polygon)
{
    return core_dispatch::interior_rings
        <
            typename tag<Polygon>::type,
            Polygon
        >::apply(polygon);
}


/*!
\brief Function to get the interior rings of a polygon (const version)
\ingroup interior_rings
\note OGC compliance: instead of InteriorRingN
\tparam Polygon polygon type
\param polygon the polygon to get the interior rings from
\return the interior rings (possibly a const reference)

\qbk{distinguish,const version}
*/
template <typename Polygon>
inline typename interior_return_type<Polygon const>::type interior_rings(
            Polygon const& polygon)
{
    return core_dispatch::interior_rings
        <
            typename tag<Polygon>::type,
            Polygon const
        >::apply(polygon);
}


/*!
\brief \brief_calc{number of interior rings}
\ingroup num_interior_rings
\details \details_calc{num_interior_rings, number of interior rings}.
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_calc{number of interior rings}

\qbk{include,ref/algorithms/num_interior_rings.qbk}

\note Defined by OGC as "numInteriorRing". To be consistent with "numPoints"
    letter "s" is appended
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
