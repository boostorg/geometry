// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014, 2015.
// Modifications copyright (c) 2013-2015 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP

#include <cstddef>

#include <boost/concept_check.hpp>
#include <boost/range.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/strategies/concepts/within_concept.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/within.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/order_as_direction.hpp>
#include <boost/geometry/views/closeable_view.hpp>
#include <boost/geometry/views/reversible_view.hpp>

#include <boost/geometry/algorithms/detail/relate/result.hpp>

#include <boost/geometry/algorithms/detail/relate/point_point.hpp>
#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/linear_linear.hpp>
#include <boost/geometry/algorithms/detail/relate/linear_areal.hpp>
#include <boost/geometry/algorithms/detail/relate/areal_areal.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

// Those are used only to allow dispatch::relate to produce compile-time error

template <typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type>
struct is_supported_by_generic
{
    static const bool value
        = boost::is_same<Tag, linestring_tag>::value
       || boost::is_same<Tag, multi_linestring_tag>::value
       || boost::is_same<Tag, ring_tag>::value
       || boost::is_same<Tag, polygon_tag>::value
       || boost::is_same<Tag, multi_polygon_tag>::value;
};

template <typename Geometry1,
          typename Geometry2,
          typename Tag1 = typename geometry::tag<Geometry1>::type,
          typename Tag2 = typename geometry::tag<Geometry2>::type>
struct is_generic
{
    static const bool value = is_supported_by_generic<Geometry1>::value
                           && is_supported_by_generic<Geometry2>::value;
};


template <typename Point, typename Geometry, typename Tag>
struct is_generic<Point, Geometry, point_tag, Tag>
{
    static const bool value = is_supported_by_generic<Geometry>::value;
};

template <typename Geometry, typename Point, typename Tag>
struct is_generic<Geometry, Point, Tag, point_tag>
{
    static const bool value = is_supported_by_generic<Geometry>::value;
};

template <typename Point1, typename Point2>
struct is_generic<Point1, Point2, point_tag, point_tag>
{
    static const bool value = false;
};


}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {


template <typename Geometry1,
          typename Geometry2,
          typename Tag1 = typename geometry::tag<Geometry1>::type,
          typename Tag2 = typename geometry::tag<Geometry2>::type,
          int TopDim1 = geometry::topological_dimension<Geometry1>::value,
          int TopDim2 = geometry::topological_dimension<Geometry2>::value,
          bool IsGeneric = detail::relate::is_generic<Geometry1, Geometry2>::value
>
struct relate : not_implemented<Tag1, Tag2>
{};


template <typename Point1, typename Point2>
struct relate<Point1, Point2, point_tag, point_tag, 0, 0, false>
    : detail::relate::point_point<Point1, Point2>
{};

template <typename Point, typename MultiPoint>
struct relate<Point, MultiPoint, point_tag, multi_point_tag, 0, 0, false>
    : detail::relate::point_multipoint<Point, MultiPoint>
{};

template <typename MultiPoint, typename Point>
struct relate<MultiPoint, Point, multi_point_tag, point_tag, 0, 0, false>
    : detail::relate::multipoint_point<MultiPoint, Point>
{};

template <typename MultiPoint1, typename MultiPoint2>
struct relate<MultiPoint1, MultiPoint2, multi_point_tag, multi_point_tag, 0, 0, false>
    : detail::relate::multipoint_multipoint<MultiPoint1, MultiPoint2>
{};

//template <typename Point, typename Box, int TopDim2>
//struct relate<Point, Box, point_tag, box_tag, 0, TopDim2, false>
//    : detail::relate::point_box<Point, Box>
//{};
//
//template <typename Box, typename Point, int TopDim1>
//struct relate<Box, Point, box_tag, point_tag, TopDim1, 0, false>
//    : detail::relate::box_point<Box, Point>
//{};


template <typename Point, typename Geometry, typename Tag2, int TopDim2>
struct relate<Point, Geometry, point_tag, Tag2, 0, TopDim2, true>
    : detail::relate::point_geometry<Point, Geometry>
{};

template <typename Geometry, typename Point, typename Tag1, int TopDim1>
struct relate<Geometry, Point, Tag1, point_tag, TopDim1, 0, true>
    : detail::relate::geometry_point<Geometry, Point>
{};


template <typename Linear1, typename Linear2, typename Tag1, typename Tag2>
struct relate<Linear1, Linear2, Tag1, Tag2, 1, 1, true>
    : detail::relate::linear_linear<Linear1, Linear2>
{};


template <typename Linear, typename Areal, typename Tag1, typename Tag2>
struct relate<Linear, Areal, Tag1, Tag2, 1, 2, true>
    : detail::relate::linear_areal<Linear, Areal>
{};

template <typename Areal, typename Linear, typename Tag1, typename Tag2>
struct relate<Areal, Linear, Tag1, Tag2, 2, 1, true>
    : detail::relate::areal_linear<Areal, Linear>
{};


template <typename Areal1, typename Areal2, typename Tag1, typename Tag2>
struct relate<Areal1, Areal2, Tag1, Tag2, 2, 2, true>
    : detail::relate::areal_areal<Areal1, Areal2>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

template <typename Geometry1, typename Geometry2>
struct interruption_enabled
{
    static const bool value =
        dispatch::relate<Geometry1, Geometry2>::interruption_enabled;
};

template <typename Geometry1,
          typename Geometry2,
          typename Result,
          bool IsSequence = boost::mpl::is_sequence<Result>::value>
struct result_handler_type
    : not_implemented<Result>
{};

template <typename Geometry1, typename Geometry2>
struct result_handler_type<Geometry1, Geometry2, de9im::matrix, false>
{
    typedef matrix_handler<de9im::matrix> type;
};

template <typename Geometry1, typename Geometry2>
struct result_handler_type<Geometry1, Geometry2, de9im::mask, false>
{
    typedef mask_handler
        <
            de9im::mask,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

template <typename Geometry1, typename Geometry2, typename Head, typename Tail>
struct result_handler_type<Geometry1, Geometry2, boost::tuples::cons<Head, Tail>, false>
{
    typedef mask_handler
        <
            boost::tuples::cons<Head, Tail>,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

template <typename Geometry1, typename Geometry2,
          char II, char IB, char IE,
          char BI, char BB, char BE,
          char EI, char EB, char EE>
struct result_handler_type
    <
        Geometry1,
        Geometry2,
        de9im::static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE>,
        false
    >
{
    typedef static_mask_handler
        <
            de9im::static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE>,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

template <typename Geometry1, typename Geometry2, typename StaticSequence>
struct result_handler_type<Geometry1, Geometry2, StaticSequence, true>
{
    typedef static_mask_handler
        <
            StaticSequence,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Calculates the relation between a pair of geometries as defined in DE-9IM.
\ingroup relate
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return The DE-9IM matrix expressing the relation between geometries.

\qbk{[include reference/algorithms/relation.qbk]}
 */
template <typename Geometry1, typename Geometry2>
inline de9im::matrix relation(Geometry1 const& geometry1,
                              Geometry2 const& geometry2)
{
    typedef typename detail::relate::result_handler_type
        <
            Geometry1,
            Geometry2,
            de9im::matrix
        >::type handler_type;

    handler_type handler;
    dispatch::relate<Geometry1, Geometry2>::apply(geometry1, geometry2, handler);
    return handler.result();
}

/*!
\brief Checks relation between a pair of geometries defined by a mask.
\ingroup relate
\tparam Mask An intersection model Mask type.
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param mask An intersection model mask object.
\return true if the relation is compatible with the mask, false otherwise.

\qbk{[include reference/algorithms/relate.qbk]}
 */
template <typename Mask, typename Geometry1, typename Geometry2>
inline bool relate(Geometry1 const& geometry1,
                   Geometry2 const& geometry2,
                   Mask const& mask = Mask())
{
    typedef typename detail::relate::result_handler_type
        <
            Geometry1,
            Geometry2,
            Mask
        >::type handler_type;

    handler_type handler(mask);
    dispatch::relate<Geometry1, Geometry2>::apply(geometry1, geometry2, handler);
    return handler.result();
}

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

struct implemented_tag {};

template <template <typename, typename> class StaticMaskTrait,
          typename Geometry1,
          typename Geometry2>
struct relate_base
    : boost::mpl::if_
        <
            boost::mpl::or_
                <
                    boost::is_base_of
                        <
                            nyi::not_implemented_tag,
                            StaticMaskTrait<Geometry1, Geometry2>
                        >,
                    boost::is_base_of
                        <
                            nyi::not_implemented_tag,
                            dispatch::relate<Geometry1, Geometry2>
                        >
                >,
            not_implemented
                <
                    typename geometry::tag<Geometry1>::type,
                    typename geometry::tag<Geometry2>::type
                >,
            implemented_tag
        >::type
{
    static inline bool apply(Geometry1 const& g1, Geometry2 const& g2)
    {
        typedef typename detail::relate::result_handler_type
            <
                Geometry1,
                Geometry2,
                typename StaticMaskTrait<Geometry1, Geometry2>::type
            >::type handler_type;

        handler_type handler;
        dispatch::relate<Geometry1, Geometry2>::apply(g1, g2, handler);
        return handler.result();
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP
