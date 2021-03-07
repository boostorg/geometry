// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CENTROID_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_CENTROID_GEOGRAPHIC_HPP


#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/length/services.hpp>

#include <boost/geometry/strategies/geographic/distance.hpp>
// TODO - for backwards compatibility, remove?
#include <boost/geometry/strategies/geographic/distance_andoyer.hpp>
#include <boost/geometry/strategies/geographic/distance_thomas.hpp>
#include <boost/geometry/strategies/geographic/distance_vincenty.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace centroid
{

/*
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic
    : public strategies::detail::geographic_base<Spheroid>
{
    using base_t = strategies::detail::geographic_base<Spheroid>;

public:
    geographic() = default;

    explicit geographic(Spheroid const& spheroid)
        : base_t(spheroid)
    {}
};
*/

namespace services
{

} // namespace services

}} // namespace strategies::centroid

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CENTROID_GEOGRAPHIC_HPP
