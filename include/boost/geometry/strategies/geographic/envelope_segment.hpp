// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_ENVELOPE_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_ENVELOPE_SEGMENT_HPP

#include <boost/geometry/formulas/envelope_segment.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/strategies/geographic/azimuth_geographic.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

template
<
        typename CalculationType,
        typename Spheroid = geometry::srs::spheroid<CalculationType>,
        template <typename, bool, bool, bool, bool, bool> class Inverse =
                 geometry::formula::thomas_inverse
>
class segment_geographic
{
public :

    typedef Spheroid model_type;

    inline segment_geographic()
        : m_spheroid()
    {}

    explicit inline segment_geographic(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename Point1, typename Point2, typename Box>
    inline void
    apply(Point1 const& point1, Point2 const& point2, Box& box) const
    {

        geometry::strategy::azimuth::geographic
                        <
                            double,
                            geometry::srs::spheroid<double>,
                            Inverse
                        > azimuth_geographic;

        typedef typename coordinate_system<Point1>::type::units units_type;

        geometry::formula::envelope_segment<geometry::geographic_tag>::template apply<units_type>(
                    geometry::get<0>(point1),
                    geometry::get<1>(point1),
                    geometry::get<0>(point2),
                    geometry::get<1>(point2),
                    box,
                    azimuth_geographic);
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
    typedef strategy::envelope::segment_geographic<CalculationType> type;
};

}

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_ENVELOPE_SEGMENT_HPP
