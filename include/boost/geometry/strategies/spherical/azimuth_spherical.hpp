// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AZIMUTH_SPHERICAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AZIMUTH_SPHERICAL_HPP

#include <boost/geometry/strategies/azimuth.hpp>
#include <boost/geometry/formulas/spherical.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace azimuth
{

template
<
    typename CalculationType
>
class spherical
{
public :

    inline spherical()
    {}

    inline void apply(CalculationType const& lon1_rad,
                      CalculationType const& lat1_rad,
                      CalculationType const& lon2_rad,
                      CalculationType const& lat2_rad,
                      CalculationType& a1,
                      CalculationType& a2) const
    {
        geometry::formula::result_spherical<CalculationType> result = geometry::formula::
            spherical_azimuth<CalculationType, true>(lon1_rad, lat1_rad, lon2_rad, lat2_rad);

        a1 = result.azimuth;
        a2 = result.reverse_azimuth;
    }

};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

template <typename CalculationType>
struct default_strategy<spherical_equatorial_tag, CalculationType>
{
    typedef strategy::azimuth::spherical<CalculationType> type;
};

/*
template <typename CalculationType>
struct default_strategy<spherical_polar_tag, CalculationType>
{
    typedef strategy::azimuth::spherical<CalculationType> type;
};
*/
}

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::azimuth


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_AZIMUTH_SPHERICAL_HPP
