// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_THOMAS_FIRST_ORDER_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_THOMAS_FIRST_ORDER_DIRECT_HPP


#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/formulas/differential_quantities.hpp>
#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/result_direct.hpp>


namespace boost { namespace geometry { namespace formula
{


/*!
\brief The solution of the direct problem of geodesics on latlong coordinates,
       Forsyth-Andoyer-Lambert type approximation with first order terms.
\author See
    - Technical Report: PAUL D. THOMAS, MATHEMATICAL MODELS FOR NAVIGATION SYSTEMS, 1965
      http://www.dtic.mil/docs/citations/AD0627893
    - Technical Report: PAUL D. THOMAS, SPHEROIDAL GEODESICS, REFERENCE SYSTEMS, AND LOCAL GEOMETRY, 1970
      http://www.dtic.mil/docs/citations/AD0703541
*/
template <
    typename CT,
    bool EnableCoordinates = true,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false
>
class thomas_first_order_direct
{

public:
    typedef result_direct<CT> result_type;

    template <typename T, typename Dist, typename Azi, typename Spheroid>
    static inline result_type apply(T const& lo1,
                                    T const& la1,
                                    Dist const& distance,
                                    Azi const& azimuth12,
                                    Spheroid const& spheroid)
    {   return thomas_direct<
                CT,
                EnableCoordinates,
                EnableReverseAzimuth,
                EnableReducedLength,
                EnableGeodesicScale
        >::apply(lo1, la1, distance, azimuth12, spheroid, false);
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_THOMAS_FIRST_ORDER_DIRECT_HPP
