// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_GEOMETRIES_DETAIL_REBIND_GEOMETRY_HPP
#define BOOST_GEOMETRY_GEOMETRIES_DETAIL_REBIND_GEOMETRY_HPP

#include <tuple>
#include <vector>
#include <boost/range/value_type.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/geometries/adapted/boost_variant.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/util/tuples.hpp>

namespace boost { namespace geometry { namespace detail
{

template <typename Point>
struct working_models
{
    using point = Point;
    using line = model::linestring<point>;
    template <bool CW, bool Closed> using ring = model::ring<point, CW, Closed>;
    template <bool CW, bool Closed> using polygon = model::polygon<point, CW, Closed>;
    template <typename> struct box { using type = model::box<point>; };
    template <typename G> struct other { using type = G; };
};

template <typename Models, typename G, typename Tag = tag_t<G>>
struct rebind_working : Models::template other<G> {};

template <typename M, typename G> struct rebind_working<M, G, point_tag>
{ using type = typename M::point; };
template <typename M, typename G> struct rebind_working<M, G, linestring_tag>
{ using type = typename M::line; };
template <typename M, typename G> struct rebind_working<M, G, box_tag>
    : M::template box<G> {};
template <typename M, typename G> struct rebind_working<M, G, ring_tag>
{ using type = typename M::template ring<geometry::point_order<G>::value == clockwise,
                                        geometry::closure<G>::value == closed>; };
template <typename M, typename G> struct rebind_working<M, G, polygon_tag>
{ using type = typename M::template polygon<geometry::point_order<G>::value == clockwise,
                                            geometry::closure<G>::value == closed>; };
template <typename M, typename G> struct rebind_working<M, G, multi_point_tag>
{ using type = model::multi_point<typename M::point>; };
template <typename M, typename G> struct rebind_working<M, G, multi_linestring_tag>
{ using type = model::multi_linestring<typename M::line>; };
template <typename M, typename G> struct rebind_working<M, G, multi_polygon_tag>
{ using type = model::multi_polygon<typename rebind_working<M, typename boost::range_value<G>::type>::type>; };

template <typename M, typename Sequence> struct rebind_working_variant;
template <typename M, typename... G>
struct rebind_working_variant<M, util::type_sequence<G...>>
{ using type = boost::variant<typename rebind_working<M, G>::type...>; };
template <typename M, typename G> struct rebind_working<M, G, dynamic_geometry_tag>
    : rebind_working_variant<M, typename traits::geometry_types<G>::type> {};
template <typename M, typename G> struct rebind_working<M, G, geometry_collection_tag>
{ using type = model::geometry_collection<typename rebind_working_variant<M, typename traits::geometry_types<G>::type>::type>; };

template <typename M, typename G> struct rebind_working<M, G, void>
{ using type = std::vector<typename rebind_working<M, typename boost::range_value<G>::type>::type>; };
template <typename M, typename... G> struct rebind_working<M, std::tuple<G...>, void>
{ using type = std::tuple<typename rebind_working<M, G>::type...>; };
template <typename M, typename A, typename B> struct rebind_working<M, std::pair<A, B>, void>
{ using type = std::pair<typename rebind_working<M, A>::type, typename rebind_working<M, B>::type>; };
template <typename M> struct rebind_working<M, boost::tuples::null_type, void>
{ using type = boost::tuples::null_type; };
template <typename M, typename H, typename T> struct rebind_working<M, boost::tuples::cons<H, T>, void>
{ using type = boost::tuples::cons<typename rebind_working<M, H>::type, typename rebind_working<M, T>::type>; };
template <typename M, typename... G> struct rebind_working<M, boost::tuples::tuple<G...>, void>
{ using type = boost::tuples::tuple<typename rebind_working<M, G>::type...>; };

template <typename M, typename G, bool IsTuple = tuples::is_tuple<G>::value>
struct rebind_output_tuple : rebind_working<M, G> {};

template <typename M, typename G>
struct rebind_output_tuple<M, G, true>
{
    template <std::size_t... I>
    static auto type_of(std::index_sequence<I...>)
        -> std::tuple<typename rebind_output_tuple<M, typename tuples::element<I, G>::type>::type...>;
    using type = decltype(type_of(std::make_index_sequence<tuples::size<G>::value>()));
};

}}} // namespace boost::geometry::detail

#endif
