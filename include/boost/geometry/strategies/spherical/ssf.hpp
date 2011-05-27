// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SSF_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SSF_HPP

#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/util/select_coordinate_type.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/side.hpp>
//#include <boost/geometry/strategies/concepts/side_concept.hpp>


namespace boost { namespace geometry
{


namespace strategy { namespace side
{


/*!
\brief Check at which side of a segment a point lies:
\details from a Great Circle segment between two points:
         left of segment (> 0), right of segment (< 0), on segment (0)
\ingroup strategies
\tparam CalculationType \tparam_calculation
 */
template <typename CalculationType = void>
class spherical_side_formula
{

public :
    template <typename P1, typename P2, typename P>
    static inline int apply(P1 const& p1, P2 const& p2, P const& p)
    {
        typedef typename boost::mpl::if_c
            <
                boost::is_void<CalculationType>::type::value,

                // Select at least a double...
                typename select_most_precise
                    <
                        typename select_most_precise
                            <
                                typename select_most_precise
                                    <
                                        typename coordinate_type<P1>::type,
                                        typename coordinate_type<P2>::type
                                    >::type,
                                typename coordinate_type<P>::type
                            >::type,
                        double
                    >::type,
                CalculationType
            >::type coordinate_type;

        // Convenient shortcuts
        typedef coordinate_type ct;
        ct const phi1 = get_as_radian<0>(p1);
        ct const theta1 = get_as_radian<1>(p1);
        ct const phi2 = get_as_radian<0>(p2);
        ct const theta2 = get_as_radian<1>(p2);
        ct const phi = get_as_radian<0>(p);
        ct const theta = get_as_radian<1>(p);

        // Create temporary points (vectors) on unit a sphere
        ct const sin_theta1 = sin(theta1);
        ct const c1x = sin_theta1 * cos(phi1);
        ct const c1y = sin_theta1 * sin(phi1);
        ct const c1z = cos(theta1);

        ct const sin_theta2 = sin(theta2);
        ct const c2x = sin_theta2 * cos(phi2);
        ct const c2y = sin_theta2 * sin(phi2);
        ct const c2z = cos(theta2);

        // (Third point is converted directly)
        ct const sin_theta = sin(theta);
        
        // Apply the "Spherical Side Formula" as presented on my blog
        ct const dist 
            = (c1y * c2z - c1z * c2y) * sin_theta * cos(phi) 
            + (c1z * c2x - c1x * c2z) * sin_theta * sin(phi)
            + (c1x * c2y - c1y * c2x) * cos(theta);
            
        return dist < 0 ? 1
            : dist > 0 ? -1
            : 0;
    }
};

}} // namespace strategy::side


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SSF_HPP
