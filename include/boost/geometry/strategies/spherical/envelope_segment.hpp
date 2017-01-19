// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_ENVELOPE_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_ENVELOPE_SEGMENT_HPP

#include <boost/geometry/strategies/spherical/azimuth_spherical.hpp>
#include <boost/geometry/formulas/envelope_segment.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

template<typename CalculationType>
class segment_spherical
{
public :

    inline segment_spherical()
    {}

    template <typename Point1, typename Point2, typename Box>
    inline void
    apply(Point1 const& point1, Point2 const& point2, Box& box) const
    {

        geometry::strategy::azimuth::spherical<CalculationType> azimuth_spherical;

        typedef typename coordinate_system<Point1>::type::units units_type;

        geometry::formula::envelope_segment<spherical_equatorial_tag>::template apply<units_type>(
                    geometry::get<0>(point1),
                    geometry::get<1>(point1),
                    geometry::get<0>(point2),
                    geometry::get<1>(point2),
                    box,
                    azimuth_spherical);
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

template <typename CalculationType>
struct default_strategy<spherical_equatorial_tag, CalculationType>
{
    typedef strategy::envelope::segment_spherical<CalculationType> type;
};

template <typename CalculationType>
struct default_strategy<spherical_polar_tag, CalculationType>
{
    typedef strategy::envelope::segment_spherical<CalculationType> type;
};

}

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_ENVELOPE_SEGMENT_HPP

