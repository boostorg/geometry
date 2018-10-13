// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017-2018 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_ENVELOPE_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_ENVELOPE_SEGMENT_HPP

#include <boost/geometry/algorithms/detail/envelope/segment.hpp>
#include <boost/geometry/algorithms/detail/normalize.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/strategies/envelope.hpp>
#include <boost/geometry/strategies/spherical/azimuth.hpp>
#include <boost/geometry/strategies/spherical/expand_box.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

template
<
    typename CalculationType = void
>
class spherical_segment
{
public:
    typedef strategy::expand::spherical_box box_expand_strategy_type;
    static inline box_expand_strategy_type get_box_expand_strategy()
    {
        return box_expand_strategy_type();
    }

    template <typename Point1, typename Point2, typename Box>
    static inline void apply(Point1 const& point1, Point2 const& point2,
                             Box& box)
    {
        Point1 p1_normalized;
        strategy::normalize::spherical_point::apply(point1, p1_normalized);
        Point2 p2_normalized;
        strategy::normalize::spherical_point::apply(point2, p2_normalized);

        geometry::strategy::azimuth::spherical<CalculationType> azimuth_spherical;

        typedef typename geometry::detail::cs_angular_units<Point1>::type units_type;

        geometry::detail::envelope::envelope_segment_impl<spherical_equatorial_tag>
                ::template apply<units_type>(geometry::get<0>(p1_normalized),
                                             geometry::get<1>(p1_normalized),
                                             geometry::get<0>(p2_normalized),
                                             geometry::get<1>(p2_normalized),
                                             box,
                                             azimuth_spherical);

  }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

template <typename CalculationType>
struct default_strategy<segment_tag, spherical_equatorial_tag, CalculationType>
{
    typedef strategy::envelope::spherical_segment<CalculationType> type;
};


template <typename CalculationType>
struct default_strategy<segment_tag, spherical_polar_tag, CalculationType>
{
    typedef strategy::envelope::spherical_segment<CalculationType> type;
};

}

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_ENVELOPE_SEGMENT_HPP

