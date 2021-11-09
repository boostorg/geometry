// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGY_GEOGRAPHIC_ENVELOPE_RANGE_HPP
#define BOOST_GEOMETRY_STRATEGY_GEOGRAPHIC_ENVELOPE_RANGE_HPP

#include <boost/geometry/strategy/geographic/envelope_segment.hpp>
#include <boost/geometry/strategy/geographic/expand_segment.hpp>
#include <boost/geometry/strategy/spherical/envelope_range.hpp>

// Get rid of this dependency?
#include <boost/geometry/strategies/spherical/point_in_poly_winding.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = geometry::srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_linestring
{
public:
    using model_type = Spheroid;

    geographic_linestring()
        : m_spheroid()
    {}

    explicit geographic_linestring(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename Range, typename Box>
    void apply(Range const& range, Box& mbr) const
    {
        detail::spheroidal_linestring(range, mbr,
                                      envelope::geographic_segment
                                        <
                                            FormulaPolicy, Spheroid, CalculationType
                                        >(m_spheroid),
                                      expand::geographic_segment
                                        <
                                            FormulaPolicy, Spheroid, CalculationType
                                        >(m_spheroid));
    }

    Spheroid model() const
    {
        return m_spheroid;
    }

private:
    Spheroid m_spheroid;
};

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = geometry::srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_ring
{
public:
    using model_type = Spheroid;

    geographic_ring()
        : m_spheroid()
    {}

    explicit geographic_ring(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename Range, typename Box>
    void apply(Range const& range, Box& mbr) const
    {
        detail::spheroidal_ring(range, mbr,
                                envelope::geographic_segment
                                    <
                                        FormulaPolicy, Spheroid, CalculationType
                                    >(m_spheroid),
                                expand::geographic_segment
                                    <
                                        FormulaPolicy, Spheroid, CalculationType
                                    >(m_spheroid),
                                within::detail::spherical_winding_base
                                    <
                                        envelope::detail::side_of_pole<CalculationType>,
                                        CalculationType
                                    >());
    }

    Spheroid model() const
    {
        return m_spheroid;
    }

private:
    Spheroid m_spheroid;
};

}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGY_GEOGRAPHIC_ENVELOPE_RANGE_HPP
