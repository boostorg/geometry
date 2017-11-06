
// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_ANDOYER_INVERSE_PARAMETRIC_HPP
#define BOOST_GEOMETRY_FORMULAS_ANDOYER_INVERSE_PARAMETRIC_HPP

#include <boost/geometry/formulas/andoyer_inverse.hpp>


namespace boost { namespace geometry { namespace formula
{

/*!
\brief The solution of the inverse problem of geodesics on latlong coordinates,
       Forsyth-Andoyer-Lambert type approximation with first order terms.
\author See
    - Technical Report: PAUL D. THOMAS, MATHEMATICAL MODELS FOR NAVIGATION SYSTEMS, 1965
      http://www.dtic.mil/docs/citations/AD0627893
    - Technical Report: PAUL D. THOMAS, SPHEROIDAL GEODESICS, REFERENCE SYSTEMS, AND LOCAL GEOMETRY, 1970
      http://www.dtic.mil/docs/citations/AD703541
*/
template <
    typename CT,
    bool EnableDistance,
    bool EnableAzimuth,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false
>
class andoyer_inverse_parametric
{

public:
    typedef result_inverse<CT> result_type;

    template <typename T1, typename T2, typename Spheroid>
    static inline result_type apply(T1 const& lon1,
                                    T1 const& la1,
                                    T2 const& lon2,
                                    T2 const& la2,
                                    Spheroid const& spheroid)
    {
        return andoyer_inverse<
                CT,
                EnableDistance,
                EnableAzimuth,
                EnableReverseAzimuth,
                EnableReducedLength,
                EnableGeodesicScale
        >::apply(lon1, la1, lon2, la2, spheroid, true);
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_ANDOYER_INVERSE_PARAMETRIC_HPP
