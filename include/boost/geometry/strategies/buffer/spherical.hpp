// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_BUFFER_SPHERICAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_BUFFER_SPHERICAL_HPP


#include <boost/geometry/strategies/buffer/services.hpp>
#include <boost/geometry/strategies/relate/spherical.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace buffer
{

template
<
    typename RadiusTypeOrSphere = double,
    typename CalculationType = void
>
class spherical
    : public strategies::relate::detail::spherical<RadiusTypeOrSphere, CalculationType>
{
    using base_t = strategies::relate::detail::spherical<RadiusTypeOrSphere, CalculationType>;

public:
    spherical() = default;

    template <typename RadiusOrSphere>
    explicit spherical(RadiusOrSphere const& radius_or_sphere)
        : base_t(radius_or_sphere)
    {}

    // Additional strategies for simplify()

    template <typename Geometry1, typename Geometry2>
    auto distance(Geometry1 const&, Geometry2 const&,
                  std::enable_if_t
                  <
                        util::is_pointlike<Geometry1>::value
                            && util::is_segmental<Geometry2>::value
                     || util::is_segmental<Geometry1>::value
                            && util::is_pointlike<Geometry2>::value
                     || util::is_segmental<Geometry1>::value
                            && util::is_segmental<Geometry2>::value
                  > * = nullptr) const
    {
        return strategy::distance::cross_track
            <
                CalculationType,
                strategy::distance::haversine<typename base_t::radius_type, CalculationType>
            >(base_t::radius());
    }
};


namespace services
{

template <typename Geometry>
struct default_strategy<Geometry, spherical_equatorial_tag>
{
    using type = strategies::buffer::spherical<>;
};


} // namespace services

}} // namespace strategies::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_SPHERICAL_HPP
