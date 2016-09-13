// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AZIMUTH_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AZIMUTH_GEOGRAPHIC_HPP

namespace boost { namespace geometry
{

namespace strategy { namespace azimuth
{

template
<
    typename CalculationType,
    template <typename, bool, bool, bool, bool, bool> class Inverse =
              geometry::formula::thomas_inverse
>
class azimuth_geographic
{
public :

    inline azimuth_geographic()
    {}

    inline void apply(CalculationType const& lon1_rad,
                      CalculationType const& lat1_rad,
                      CalculationType const& lon2_rad,
                      CalculationType const& lat2_rad,
                      CalculationType& a1,
                      CalculationType& a2) const
    {
        typedef Inverse<CalculationType, false, true, true, false, false> inverse_type;
        typedef typename inverse_type::result_type inverse_result;
        inverse_result i_res = inverse_type::apply(lon1_rad, lat1_rad,
                                                   lon2_rad, lat2_rad,
                                                   geometry::srs::spheroid<CalculationType>());
        a1 = i_res.azimuth;
        a2 = i_res.reverse_azimuth;
    }

};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

template <typename CalculationType>
struct default_strategy<geographic_tag, CalculationType>
{
    typedef strategy::azimuth::azimuth_geographic<CalculationType> type;
};

}

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::azimuth


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AZIMUTH_GEOGRAPHIC_HPP
