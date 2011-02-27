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
\brief \brief_calc2{symmetric difference}  \brief_strategy
\ingroup sym_difference
\details \details_calc2{symmetric difference, spatial set theoretic symmetric difference (XOR)}
    \brief_strategy. details_inserter{sym_difference}
\tparam GeometryOut output geometry type, must be specified
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Strategy \tparam_strategy_overlay
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param out \param_out{difference}
\param strategy \param_strategy{difference}
\return \return_out

\qbk{distinguish,with strategy}
*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator,
    typename Strategy
>
inline OutputIterator sym_difference_inserter(Geometry1 const& geometry1,
            Geometry2 const& geometry2, OutputIterator out,
            Strategy const& strategy)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();
    concept::check<GeometryOut>();

    out = detail::intersection::inserter<GeometryOut, false, true, false, overlay_difference>(
            geometry1, geometry2, out, strategy);
    out = detail::intersection::inserter<GeometryOut, false, true, false, overlay_difference>(
            geometry2, geometry1, out, strategy);
    return out;
}


/*!
\brief \brief_calc2{symmetric difference}
\ingroup sym_difference
\details \details_calc2{symmetric difference, spatial set theoretic symmetric difference (XOR)}
    \details_inserter{sym_difference}
\tparam GeometryOut output geometry type, must be specified
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param out \param_out{difference}
\return \return_out

*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator
>
inline OutputIterator sym_difference_inserter(Geometry1 const& geometry1,
            Geometry2 const& geometry2, OutputIterator out)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();
    concept::check<GeometryOut>();

    typedef strategy_intersection
        <
            typename cs_tag<GeometryOut>::type,
            Geometry1,
            Geometry2,
            typename geometry::point_type<GeometryOut>::type
        > strategy_type;

    return sym_difference_inserter<GeometryOut>(geometry1, geometry2, out, strategy_type());
}


/*!
\brief \brief_calc2{symmetric difference}
\ingroup sym_difference
\details \details_calc2{symmetric difference, spatial set theoretic symmetric difference (XOR)}.
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Collection output collection, either a multi-geometry,
    or a std::vector<Geometry> / std::deque<Geometry> etc
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param output_collection the output collection

\qbk{[include reference/algorithms/sym_difference.qbk]}
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

    sym_difference_inserter<geometry_out>(
            geometry1, geometry2,
            std::back_inserter(output_collection));
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_SYM_DIFFERENCE_HPP
