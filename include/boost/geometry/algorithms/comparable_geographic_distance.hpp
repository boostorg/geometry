//Boost.Geometry (aka GGL, Generic Geometry Library)

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

#ifndef BOOST_GEOMETRY_ALGORITHMS_COMPARABLE_GEOGRAPHIC_DISTANCE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_COMPARABLE_GEOGRAPHIC_DISTANCE_HPP

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/comparable_distance_result.hpp>
#include <boost/geometry/strategies/default_comparable_distance_result.hpp>
#include <boost/geometry/strategies/distance.hpp>

#include <boost/geometry/algorithms/detail/distance/interface.hpp>


namespace boost { namespace geometry
{

namespace resolve_strategy
{

struct comparable_geographic_distance
{
	template <typename Geometry1, typename Geometry2, typename Strategy>
		static inline
		typename comparable_distance_result<Geometry1, Geometry2, Strategy>::type
		apply(Geometry1 const& geometry1,
			  Geometry2 const& geometry2,
			  Strategy const& strategy)
		{
			typedef typename strategy::distance::services::comparable_type
				<
					Strategy
				>::type comparable_strategy_type;

			return dispatch::distance
				<
				Geometry1, Geometry2, comparable_strategy_type
				>::apply(geometry1,
						 geometry2,
						 strategy::distance::services::get_comparable
						 <
							Strategy
						 >::apply(strategy));
		}
};

} // namespace resolve_strategy


namespace resolve_variant
{


template <typename Geometry1, typename Geometry2, typename Strategy>
struct comparable_geographic_distance
{
	static inline
	typename comparable_distance_result<Geometry1, Geometry2, Strategy>::type
	apply(Geometry1 const& geometry1,
		  Geometry2 const& geometry2,
		  Strategy const& strategy)
	{
		return resolve_strategy::comparable_geographic_distance::apply(geometry1,
																	   geometry2,
															           strategy);
	}
};


} // namespace resolve_variant


template <typename Geometry1, typename Geometry2, typename Strategy>
inline typename comparable_distance_result<Geometry1, Geometry2, Strategy>::type
comparable_geographic_distance(Geometry1 const& geometry1, Geometry2 const& geometry2,
	            	Strategy const& strategy)
{
	concepts::check<Geometry1 const>();
	concepts::check<Geometry2 const>();

	return resolve_variant::comparable_geographic_distance
		<
			Geometry1,
			Geometry2,
			Strategy
		>::apply(geometry1, geometry2, strategy);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_COMPARABLE_DISTANCE_HPP
