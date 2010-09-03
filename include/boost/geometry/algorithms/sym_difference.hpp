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
#include <boost/geometry/algorithms/reverse.hpp>


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
inline void sym_difference(Geometry1 geometry1,
            Geometry2 geometry2, Collection& output_collection)
{
    concept::check<Geometry1>();
    concept::check<Geometry2>();

    reverse(geometry2);
    intersection(geometry1, geometry2, output_collection);
    reverse(geometry2);
    reverse(geometry1);
    intersection(geometry1, geometry2, output_collection);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_SYM_DIFFERENCE_HPP
