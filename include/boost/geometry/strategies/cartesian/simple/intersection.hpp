// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIMPLE_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIMPLE_INTERSECTION_HPP

#include <cstddef>
#include <type_traits>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/strategies/cartesian/detail/segment_events.hpp>
#include <boost/geometry/strategies/cartesian/simple/predicates.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace simple_cartesian
{

template <typename Point>
struct intersection_result
{
    using point_type = Point;
    std::size_t count = 0;
    Point intersections[2];
};

template <typename Source, typename Destination>
inline void copy_point(Source const& source, Destination& destination)
{
    check_geometry<Destination>();
    check_geometry<Source>();
    geometry::detail::conversion::convert_point_to_point(source, destination);
}

struct copy_endpoint
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination) { copy_point(source, destination); }
};

template <typename CalculationType>
struct segments
{
    using cs_tag = cartesian_tag;
    struct construction_info
    {
        int endpoint;
        template <typename Output, typename A, typename B>
        void calculate(Output& output, A const& a, B const& b) const
        {
            switch (endpoint)
            {
            case 0: copy_point(a.first, output); return;
            case 1: copy_point(a.second, output); return;
            case 2: copy_point(b.first, output); return;
            case 3: copy_point(b.second, output); return;
            }
            using T = calculation_t<CalculationType, A, B>;
            using ops = linear_formula::arithmetic_ops<T>;
            auto const origin = ordinates<T>(a.first);
            auto const terms = linear_formula::affine_intersection(ops{},
                origin, ordinates<T>(a.second), ordinates<T>(b.first), ordinates<T>(b.second));
            T const parameter = terms.numerator / terms.denominator;
            auto const point = linear_formula::affine_point(origin, parameter, terms.direction, ops::multiply_add);
            check_geometry<Output>();
            using out = geometry::coordinate_type_t<Output>;
            geometry::set<0>(output, boost::numeric_cast<out>(point[0]));
            geometry::set<1>(output, boost::numeric_cast<out>(point[1]));
        }
    };

    template <typename Range1, typename Range2, typename Policy>
    static typename Policy::return_type apply(Range1 const& p, Range2 const& q, Policy const&)
    {
        using T = calculation_t<CalculationType, std::decay_t<decltype(p.at(0))>,
                                                std::decay_t<decltype(q.at(0))>>;
        using position = segment_position<T>;
        using selected = cartesian_segment_events::endpoint_policy<Policy, position, copy_endpoint>;
        auto const equal = [](auto const& a, auto const& b) { return point_equal<T>::apply(a, b); };
        return cartesian_segment_events::classify<selected>(
            p.at(0), p.at(1), q.at(0), q.at(1),
            [](auto const& a, auto const& b, auto const& point)
            {
                return side<T>::apply(a, b, point);
            },
            equal,
            [&](auto const& a, auto const& b)
            {
                return cartesian_segment_events::collinear<selected, position, position_tag<T>>(a, b, equal);
            },
            [](int endpoint) { return construction_info{endpoint}; },
            cartesian_segment_events::use_input_endpoint{},
            [](auto const& a, auto const& b)
            {
                T const dx = coordinate<0, T>(a.second) - coordinate<0, T>(a.first);
                T const dy = coordinate<1, T>(a.second) - coordinate<1, T>(a.first);
                T const ex = coordinate<0, T>(b.second) - coordinate<0, T>(b.first);
                T const ey = coordinate<1, T>(b.second) - coordinate<1, T>(b.first);
                return dx * ey - dy * ex != 0;
            });
    }
};

}}}}}

#endif
