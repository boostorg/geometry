// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_BUFFER_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_BUFFER_GEOGRAPHIC_HPP


#include <boost/geometry/strategies/buffer/services.hpp>
#include <boost/geometry/strategies/relate/geographic.hpp>

#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace buffer
{


template
<
    typename FormulaPolicy = strategy::andoyer,
    std::size_t SeriesOrder = strategy::default_order<FormulaPolicy>::value,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic
    : public strategies::relate::geographic<FormulaPolicy, SeriesOrder, Spheroid, CalculationType>
{
    using base_t = strategies::relate::geographic<FormulaPolicy, SeriesOrder, Spheroid, CalculationType>;

public:
    geographic() = default;

    explicit geographic(Spheroid const& spheroid)
        : base_t(spheroid)
    {}

    // Additional strategies for simplify()

    template <typename Geometry1, typename Geometry2>
    auto distance(Geometry1 const&, Geometry2 const&,
                  std::enable_if_t
                  <
                        (util::is_pointlike<Geometry1>::value
                            && util::is_segmental<Geometry2>::value)
                     || (util::is_segmental<Geometry1>::value
                            && util::is_pointlike<Geometry2>::value)
                     || (util::is_segmental<Geometry1>::value
                            && util::is_segmental<Geometry2>::value)
                  > * = nullptr) const
    {
        return strategy::distance::geographic_cross_track
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }
};


namespace services
{

template <typename Geometry>
struct default_strategy<Geometry, geographic_tag>
{
    using type = strategies::buffer::geographic<>;
};


} // namespace services

}} // namespace strategies::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_GEOGRAPHIC_HPP
