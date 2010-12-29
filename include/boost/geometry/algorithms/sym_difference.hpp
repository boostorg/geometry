// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_SYM_DIFFERENCE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_SYM_DIFFERENCE_HPP

#include <algorithm>


#include <boost/geometry/algorithms/intersection.hpp>


namespace boost { namespace geometry
{


/*!
\brief \brief_calc2{symmetric difference}
\ingroup sym_difference
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Collection output collection, either a multi-geometry,
    or a std::vector<Geometry> / std::deque<Geometry> etc
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param output_collection the output collection
*/
template
<
    typename Geometry1,
    typename Geometry2,
    typename Collection
>
inline void sym_difference(Geometry1 const& geometry1,
            Geometry2 const& geometry2, Collection& output_collection)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    typedef typename boost::range_value<Collection>::type geometry_out;
    concept::check<geometry_out>();

    typedef strategy_intersection
        <
            typename cs_tag<geometry_out>::type,
            Geometry1,
            Geometry2,
            typename geometry::point_type<geometry_out>::type
        > strategy;


    detail::intersection::inserter<geometry_out, false, true, false, overlay_difference>(
            geometry1, geometry2, 
            std::back_inserter(output_collection),
            strategy());
    detail::intersection::inserter<geometry_out, false, true, false, overlay_difference>(
            geometry2, geometry1, 
            std::back_inserter(output_collection),
            strategy());
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_SYM_DIFFERENCE_HPP
