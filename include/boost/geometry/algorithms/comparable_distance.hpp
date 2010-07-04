// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_COMPARABLE_DISTANCE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_COMPARABLE_DISTANCE_HPP


#include <boost/geometry/algorithms/distance.hpp>


namespace boost { namespace geometry
{


/*!
    \brief Calculate comparable distance measurement between two geometries
    \ingroup distance
    \details The default strategy is used, belonging to the corresponding coordinate system of the geometries
        and the comparable strategy is used
    \tparam G1 first geometry type
    \tparam G2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return the comparable distance
 */
template <typename Geometry1, typename Geometry2>
inline typename distance_result<Geometry1, Geometry2>::type comparable_distance(
                Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    typedef typename point_type<Geometry1>::type point1_type;
    typedef typename point_type<Geometry2>::type point2_type;

    // Define a point-point-distance-strategy
    // for either the normal case, either the reversed case

    typedef typename strategy::distance::services::comparable_type
        <
            typename boost::mpl::if_c
                <
                    geometry::reverse_dispatch  
                        <Geometry1, Geometry2>::type::value,
                    typename default_distance_strategy
                            <point2_type, point1_type>::type,
                    typename default_distance_strategy
                            <point1_type, point2_type>::type
                >::type
        >::type strategy_type;

    return distance(geometry1, geometry2, strategy_type());
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_COMPARABLE_DISTANCE_HPP
