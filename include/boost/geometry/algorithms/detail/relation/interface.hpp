// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014, 2015.
// Modifications copyright (c) 2013-2015 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATION_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATION_INTERFACE_HPP


#include <boost/geometry/algorithms/detail/relate/interface.hpp>


namespace boost { namespace geometry {


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {


template <typename Geometry1, typename Geometry2>
struct result_handler_type<Geometry1, Geometry2, geometry::de9im::matrix, false>
{
    typedef matrix_handler<geometry::de9im::matrix> type;
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
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();
    assert_dimension_equal<Geometry1, Geometry2>();

    typename detail::relate::result_handler_type
        <
            Geometry1,
            Geometry2,
            de9im::matrix
        >::type handler;

    dispatch::relate<Geometry1, Geometry2>::apply(geometry1, geometry2, handler);

    return handler.result();
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_INTERFACE_HPP
