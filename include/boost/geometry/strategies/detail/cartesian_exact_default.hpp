// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_CARTESIAN_EXACT_DEFAULT_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_CARTESIAN_EXACT_DEFAULT_HPP

#include <boost/config.hpp>
#include <cfloat>

// Do not even include the exact implementation in incompatible builds.
// As for explicit exact calls, finite input, nearest-even evaluation, gradual
// underflow, no overflow, and no reassociation or implicit contraction are
// preconditions. Selection neither scans inputs nor changes the FP environment.
#if BOOST_CXX_VERSION >= 201703L && !defined(BOOST_NO_CXX17_IF_CONSTEXPR) \
    && !defined(BOOST_NO_EXCEPTIONS) \
    && !defined(__FAST_MATH__) && !defined(_M_FP_FAST) \
    && !defined(__ASSOCIATIVE_MATH__) && !defined(__RECIPROCAL_MATH__) \
    && !(defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0) \
    && defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD == 0 && FLT_RADIX == 2 \
    && DBL_MANT_DIG == 53 && DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024
#define BOOST_GEOMETRY_DETAIL_HAS_CARTESIAN_EXACT_DEFAULT

#include <limits>
#include <type_traits>
#include <boost/range/iterator.hpp>
#include <boost/range/value_type.hpp>
#include <boost/geometry/strategies/relate/cartesian_exact.hpp>
#include <boost/geometry/util/tuples.hpp>

namespace boost { namespace geometry { namespace detail { namespace cartesian_exact_default
{

namespace kernel = strategies::detail::exact_cartesian;

enum class operation { intersection, difference, union_, sym_difference };
// Ordinary point, line and areal families are ordered by topological dimension.
enum class family { unsupported, point, line, area, segment, box };

template <typename G>
constexpr family kind()
{
    using tag = tag_t<G>;
    if constexpr (std::is_same_v<tag, point_tag> || std::is_same_v<tag, multi_point_tag>)
        return family::point;
    else if constexpr (std::is_same_v<tag, linestring_tag> || std::is_same_v<tag, multi_linestring_tag>)
        return family::line;
    else if constexpr (std::is_same_v<tag, ring_tag> || std::is_same_v<tag, polygon_tag>
                       || std::is_same_v<tag, multi_polygon_tag>)
        return family::area;
    else if constexpr (std::is_same_v<tag, segment_tag>) return family::segment;
    else if constexpr (std::is_same_v<tag, box_tag>) return family::box;
    else return family::unsupported;
}

// Test the tag before asking BG for coordinate traits: tuples, dynamic
// geometries and output ranges are not themselves native point sources.
template <typename G, typename = void>
struct native : std::false_type {};

template <typename G>
struct native<G, std::enable_if_t<kind<G>() != family::unsupported && !std::is_pointer_v<G>>>
    : std::bool_constant<std::numeric_limits<double>::is_iec559 && sizeof(double) == 8
        && geometry::dimension<G>::value == 2
        && std::is_same_v<coordinate_system_t<G>, cs::cartesian>
        && kernel::fixed_native_supported<coordinate_type_t<G>>::value
        && (!std::is_same_v<coordinate_type_t<G>, float>
            || (std::numeric_limits<float>::is_iec559 && sizeof(float) == 4 && FLT_MANT_DIG == 24))
        && !std::is_same_v<coordinate_type_t<G>, bool>> {};

template <typename A, typename B, typename C>
constexpr bool lossless = kernel::lossless_coordinate_conversion<coordinate_type_t<A>, C>::value
                       && kernel::lossless_coordinate_conversion<coordinate_type_t<B>, C>::value;

template <typename A, typename B>
using common = std::common_type_t<coordinate_type_t<A>, coordinate_type_t<B>>;

template <typename A, typename B>
using coordinate = std::conditional_t<lossless<A, B, common<A, B>>, common<A, B>, double>;

template <typename A, typename B>
using strategy = strategies::relate::cartesian_exact<coordinate<A, B>, strategies::relate::bounded_ulp_output<4>>;

template <typename A, typename B>
constexpr bool native_pair()
{
    if constexpr (native<A>::value && native<B>::value)
        return lossless<A, B, coordinate<A, B>>
            && std::is_same_v<typename strategies::relate::services::default_strategy<A, B>::type,
                              strategies::relate::cartesian<>>;
    else return false;
}

template <typename A, typename B>
constexpr bool admits_relation = native_pair<A, B>()
    && kind<A>() <= family::area && kind<B>() <= family::area;

// A regular output collection emits one point, line, ring or polygon at a
// time. Do not ask boost::range_value for the element of a tuple or a box.
template <typename Out, typename = void> struct element { using type = void; };
template <typename Out>
struct element<Out, std::void_t<typename boost::range_iterator<Out>::type>>
{
    using type = typename boost::range_value<Out>::type;
};

template <operation Op, typename A, typename B, typename Out>
constexpr bool admits_output()
{
    constexpr auto a = kind<A>(), b = kind<B>();
    using tag = tag_t<Out>;
    if constexpr (!native_pair<A, B>() || std::is_const_v<Out> || tuples::is_tuple<Out>::value) return false;
    else if constexpr (std::is_same_v<tag, box_tag>)
        return Op == operation::intersection && a == family::box && b == family::box && native<Out>::value;
    else if constexpr (!(std::is_void_v<tag> || std::is_same_v<tag, multi_point_tag>
                         || std::is_same_v<tag, multi_linestring_tag> || std::is_same_v<tag, multi_polygon_tag>))
        return false;
    else
    {
        using out = typename element<Out>::type;
        using ot = tag_t<out>;
        constexpr auto o = kind<out>();
        if constexpr (!native<out>::value || !(std::is_same_v<ot, point_tag> || std::is_same_v<ot, linestring_tag>
                                             || std::is_same_v<ot, polygon_tag> || std::is_same_v<ot, ring_tag>))
            return false;
        else if constexpr (a == family::box || b == family::box)
        {
            constexpr auto other = a == family::box ? b : a;
            return Op == operation::intersection && o == family::line
                && (other == family::line || other == family::segment);
        }
        else if constexpr (a == family::segment || b == family::segment)
            return Op == operation::intersection && a == b && o == family::point;
        else if constexpr (Op == operation::intersection)
            return o == (a < b ? a : b) || o == family::point;
        else if constexpr (Op == operation::difference)
            return a <= b && o == a;
        else
            return a == b && o == a;
    }
}

}}}} // namespace boost::geometry::detail::cartesian_exact_default

#endif
#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_CARTESIAN_EXACT_DEFAULT_HPP
