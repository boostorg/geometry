// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_OUTPUT_ALIAS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_OUTPUT_ALIAS_HPP

#include <functional>
#include <memory>
#include <type_traits>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace borrowed_output_alias
{

template <typename T> struct owning_point : std::false_type {};
template <typename C, std::size_t D, typename CS>
struct owning_point<model::point<C, D, CS>> : std::is_arithmetic<C> {};
template <typename C, typename CS>
struct owning_point<model::d2::point_xy<C, CS>> : std::is_arithmetic<C> {};

template <typename T> struct owning_polygon : std::false_type {};
template <typename P, bool CW, bool Closed>
struct owning_polygon<model::polygon<P, CW, Closed>> : owning_point<P> {};
template <typename T> struct owning_line : std::false_type {};
template <typename P> struct owning_line<model::linestring<P>> : owning_point<P> {};

// Deliberately recognize only the default std::vector-backed owning models,
// not arbitrary geometries advertising these same tags.
template <typename T> struct owning_multi : std::false_type { using member = void; };
template <typename P> struct owning_multi<model::multi_polygon<P>> : owning_polygon<P> { using member = P; };
template <typename L> struct owning_multi<model::multi_linestring<L>> : owning_line<L> { using member = L; };
template <typename P> struct owning_multi<model::multi_point<P>> : owning_point<P> { using member = P; };

template <typename Output, typename Input>
inline constexpr bool known_root = owning_multi<Output>::value
    && (std::is_same<Output, Input>::value || std::is_same<typename owning_multi<Output>::member, Input>::value);

template <typename Output, typename Input>
bool disjoint(Output const& output, Input const& input)
{
    if constexpr (std::is_same<Output, Input>::value)
        return std::addressof(output) != std::addressof(input);
    else
    {
        if (output.empty()) return true;
        std::less<Input const*> less;
        auto const* p = std::addressof(input);
        // A foreign address between these endpoints is conservatively treated
        // as an alias. All actual members are inside this ordered interval.
        return less(p, output.data()) || less(std::addressof(output.back()), p);
    }
}

}}}}}}

#endif
