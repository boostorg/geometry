// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_CONVERSION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_CONVERSION_HPP

#include <type_traits>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/strategies/cartesian/exact/point.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_predicates.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Coordinate, typename OutputPolicy = strategies::relate::approximate_output>
struct point_converter
{
    template <typename Source>
    static void apply(Source const& source, exact_point<Coordinate>& destination)
    {
        destination = make_point<Coordinate>(source);
    }

    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination)
    {
        if constexpr (static_predicates_detail::exact_input<std::remove_cv_t<Source>>::value)
        {
            assign_output<OutputPolicy>(source, destination);
        }
        else
        {
            // Convert both coordinates before writing either; a narrowing cast can throw.
            model::point<geometry::coordinate_type_t<Destination>, 2, cs::cartesian> xy;
            geometry::detail::conversion::convert_point_to_point(source, xy);
            geometry::set<0>(destination, geometry::get<0>(xy));
            geometry::set<1>(destination, geometry::get<1>(xy));
        }
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
