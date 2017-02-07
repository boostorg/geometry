// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AZIMUTH_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AZIMUTH_HPP

#include <boost/geometry/strategies/azimuth.hpp>

#include <boost/geometry/formulas/thomas_inverse.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace azimuth
{

template
<
    typename CalculationType,
    typename Spheroid = geometry::srs::spheroid<CalculationType>,
    template <typename, bool, bool, bool, bool, bool> class Inverse =
              geometry::formula::thomas_inverse
>
class geographic
{
public :

    typedef Spheroid model_type;

    inline geographic()
        : m_spheroid()
    {}

    explicit inline geographic(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    inline model_type const& model() const
    {
        return m_spheroid;
    }

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
                                                   m_spheroid);
        a1 = i_res.azimuth;
        a2 = i_res.reverse_azimuth;
    }

private :
    Spheroid m_spheroid;
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

template <typename CalculationType>
struct default_strategy<geographic_tag, CalculationType>
{
    typedef strategy::azimuth::geographic<CalculationType> type;
};

}

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::azimuth


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_AZIMUTH_HPP
