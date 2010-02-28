// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DIFFERENCE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DIFFERENCE_HPP

#include <algorithm>


#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/reverse.hpp>


/*!
\defgroup difference difference: difference of two geometries
*/


namespace boost { namespace geometry
{


/*!
    \ingroup difference
    \tparam Geometry geometry type
    \param geometry the geometry to make difference
*/
template
<
    typename Geometry1,
    typename Geometry2,
    typename Collection
>
inline void difference(Geometry1 const& geometry1,
            Geometry2 geometry2, Collection& output_collection)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2>();

    reverse(geometry2);

    intersection(geometry1, geometry2, output_collection);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DIFFERENCE_HPP
