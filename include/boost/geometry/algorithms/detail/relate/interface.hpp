// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014, 2015.
// Modifications copyright (c) 2013-2015 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_INTERFACE_HPP


#include <boost/type_traits/is_same.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/topological_dimension.hpp>

#include <boost/geometry/algorithms/detail/relate/de9im.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>


namespace boost { namespace geometry {


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
struct result_handler_type<Geometry1, Geometry2, geometry::de9im::mask, false>
{
    typedef mask_handler
        <
            geometry::de9im::mask,
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
        geometry::de9im::static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE>,
        false
    >
{
    typedef static_mask_handler
        <
            geometry::de9im::static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE>,
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
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();
    assert_dimension_equal<Geometry1, Geometry2>();

    typename detail::relate::result_handler_type
        <
            Geometry1,
            Geometry2,
            Mask
        >::type handler(mask);

    dispatch::relate<Geometry1, Geometry2>::apply(geometry1, geometry2, handler);

    return handler.result();
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_INTERFACE_HPP
