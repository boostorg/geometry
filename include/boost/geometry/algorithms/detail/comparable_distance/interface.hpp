// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_COMPARABLE_DISTANCE_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_COMPARABLE_DISTANCE_INTERFACE_HPP

#include <boost/geometry/strategies/default_comparable_distance_result.hpp>

#include <boost/geometry/algorithms/detail/distance/interface.hpp>

namespace boost { namespace geometry
{


// MK::need to add qbk documentation
template <typename Geometry1, typename Geometry2, typename Strategy>
inline typename strategy::distance::services::return_type
                <
                    typename strategy::distance::services::comparable_type
                        <
                            Strategy
                        >::type,
                    typename point_type<Geometry1>::type,
                    typename point_type<Geometry2>::type
                >::type
comparable_distance(Geometry1 const& geometry1, Geometry2 const& geometry2,
                    Strategy const& strategy)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    return distance(geometry1, geometry2,
                    strategy::distance::services::get_comparable
                        <
                            Strategy
                        >::apply(strategy)
                    );
}



/*!
\brief \brief_calc2{comparable distance measurement}
\ingroup distance
\details The free function comparable_distance does not necessarily calculate the distance,
    but it calculates a distance measure such that two distances are comparable to each other.
    For example: for the Cartesian coordinate system, Pythagoras is used but the square root
    is not taken, which makes it faster and the results of two point pairs can still be
    compared to each other.
\tparam Geometry1 first geometry type
\tparam Geometry2 second geometry type
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return \return_calc{comparable distance}

\qbk{[include reference/algorithms/comparable_distance.qbk]}
 */
template <typename Geometry1, typename Geometry2>
inline typename default_comparable_distance_result<Geometry1, Geometry2>::type
comparable_distance(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    // Define the default-distance-strategy
    typedef typename strategy::distance::services::comparable_type
        <
            typename detail::distance::default_strategy
                <
                    Geometry1, Geometry2
                >::type
        >::type default_comparable_strategy_type;

    return distance(geometry1, geometry2, default_comparable_strategy_type());
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_COMPARABLE_DISTANCE_INTERFACE_HPP
