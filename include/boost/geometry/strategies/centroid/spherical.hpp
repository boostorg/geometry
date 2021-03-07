// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CENTROID_SPHERICAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_CENTROID_SPHERICAL_HPP


#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/length/services.hpp>

#include <boost/geometry/strategies/spherical/distance_haversine.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace centroid
{

/*
template
<
    typename RadiusTypeOrSphere = double,
    typename CalculationType = void
>
class spherical
    : public strategies::detail::spherical_base<RadiusTypeOrSphere>
{
    using base_t = strategies::detail::spherical_base<RadiusTypeOrSphere>;

public:
    spherical() = default;

    template <typename RadiusOrSphere>
    explicit spherical(RadiusOrSphere const& radius_or_sphere)
        : base_t(radius_or_sphere)
    {}
};
*/

namespace services
{

} // namespace services

}} // namespace strategies::centroid

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CENTROID_SPHERICAL_HPP
