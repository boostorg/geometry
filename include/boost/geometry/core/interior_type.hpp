// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_CORE_INTERIOR_TYPE_HPP
#define BOOST_GEOMETRY_CORE_INTERIOR_TYPE_HPP


#include <boost/mpl/assert.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/util/ensure_const_reference.hpp>

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


template <typename GeometryTag, typename Geometry>
struct interior_type
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


template <typename Polygon>
struct interior_type<polygon_tag, Polygon>
{
    typedef typename boost::remove_reference
        <
            typename traits::interior_type
                <
                    typename boost::remove_const<Polygon>::type
                >::type
        > type;
};


template <typename GeometryTag, typename Geometry>
struct interior_return_type
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


template <typename Polygon>
struct interior_return_type<polygon_tag, Polygon>
{
    typedef typename traits::interior_type
        <
            typename boost::remove_const<Polygon>::type
        >::type type;
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

template <typename Geometry>
struct interior_return_type
{
    typedef typename core_dispatch::interior_return_type
        <
            typename tag<Geometry>::type,
            Geometry
        >::type ir_type;

    typedef typename mpl::if_
        <
            boost::is_const<Geometry>,
            typename ensure_const_reference<ir_type>::type,
            ir_type
        >::type type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_INTERIOR_TYPE_HPP
