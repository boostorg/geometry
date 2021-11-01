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

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

// TODO: divide into geographic_linestring and geographic_ring

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = geometry::srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_range
{
public:
    using model_type = Spheroid;

    geographic_range()
        : m_spheroid()
    {}

    explicit geographic_range(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename Range, typename Box>
    void apply(Range const& range, Box& mbr) const
    {
        detail::spheroidal_range(range, mbr,
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

}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGY_GEOGRAPHIC_ENVELOPE_RANGE_HPP
