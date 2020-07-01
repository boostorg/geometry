// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES2_STRATEGIES2_HPP
#define BOOST_GEOMETRY_STRATEGIES2_STRATEGIES2_HPP


#include <boost/geometry/strategies2/area.hpp>
#include <boost/geometry/strategies2/envelope.hpp>
#include <boost/geometry/strategies2/expand.hpp>


namespace boost { namespace geometry
{

    
namespace strategies
{


template <typename CalculationType = void>
struct cartesian : strategies::detail::cartesian_base
{
    // area

    template <typename Geometry>
    static auto area(Geometry const&)
    {
        return strategy::area::cartesian<CalculationType>();
    }

    // envelope

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_multi_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_box();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_segment<CalculationType>();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_non_trivial_linear_or_polygonal<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian<CalculationType>();
    }

    // expand

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_box();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_segment();
    }
};


template
<
    typename RadiusTypeOrSphere = double,
    typename CalculationType = void
>
class spherical : strategies::detail::spherical_base<RadiusTypeOrSphere>
{
    using base_t = strategies::detail::spherical_base<RadiusTypeOrSphere>;

public:
    spherical()
        : base_t()
    {}

    template <typename RadiusOrSphere>
    explicit spherical(RadiusOrSphere const& radius_or_sphere)
        : base_t(radius_or_sphere)
    {}

    // area

    template <typename Geometry>
    auto area(Geometry const&) const
    {
        return strategy::area::spherical
            <
                typename base_t::radius_type, CalculationType
            >(base_t::m_radius);
    }

    // envelope

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_multi_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_box();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_segment<CalculationType>();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_non_trivial_linear_or_polygonal<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical<CalculationType>();
    }

    // expand

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_box();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_segment<CalculationType>();
    }
};

template
<
    typename FormulaPolicy = strategy::andoyer,
    std::size_t SeriesOrder = strategy::default_order<FormulaPolicy>::value,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic : strategies::detail::geographic_base<Spheroid>
{
    using base_t = strategies::detail::geographic_base<Spheroid>;

public:
    geographic()
        : base_t()
    {}

    explicit geographic(Spheroid const& spheroid)
        : base_t(spheroid)
    {}

    // area

    template <typename Geometry>
    auto area(Geometry const&) const
    {
        return strategy::area::geographic
            <
                FormulaPolicy, SeriesOrder, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }

    // envelope

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_multi_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_box();
    }

    template <typename Geometry, typename Box>
    auto envelope(Geometry const&, Box const&,
                  typename detail::enable_if_segment<Geometry>::type * = nullptr) const
    {
        return strategy::envelope::geographic_segment
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }

    template <typename Geometry, typename Box>
    auto envelope(Geometry const&, Box const&,
                  typename detail::enable_if_non_trivial_linear_or_polygonal<Geometry>::type * = nullptr) const
    {
        return strategy::envelope::geographic
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }

    // expand

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_box();
    }

    template <typename Box, typename Geometry>
    auto expand(Box const&, Geometry const&,
                typename detail::enable_if_segment<Geometry>::type * = nullptr) const
    {
        return strategy::expand::geographic_segment
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }
};


} // namespace strategies


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES2_STRATEGIES2_HPP
