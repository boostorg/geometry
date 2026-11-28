// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.
// Copyright (c) 2014-2015 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2014-2022.
// Modifications copyright (c) 2014-2022 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_EQUALS_IMPLEMENTATION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_EQUALS_IMPLEMENTATION_HPP


#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <boost/range/size.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/is_empty.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>
#include <boost/geometry/algorithms/detail/equals/collect_vectors.hpp>
#include <boost/geometry/algorithms/detail/equals/interface.hpp>
#include <boost/geometry/algorithms/detail/relate/implementation.hpp>
#include <boost/geometry/algorithms/detail/relate/relate_impl.hpp>

#include <boost/geometry/strategies/relate/cartesian.hpp>
#include <boost/geometry/strategies/relate/geographic.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>
#include <boost/geometry/strategies/relate/spherical.hpp>
#include <boost/geometry/views/detail/indexed_point_view.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace equals
{


struct point_point
{
    template <typename Point1, typename Point2, typename Strategy>
    static inline bool apply(Point1 const& point1, Point2 const& point2,
                             Strategy const& strategy)
    {
        return equals::equals_point_point(point1, point2, strategy);
    }
};


template
<
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct box_box
{
    template <typename Box1, typename Box2, typename Strategy>
    static inline bool apply(Box1 const& box1, Box2 const& box2, Strategy const& strategy)
    {
        if (!geometry::math::equals(get<min_corner, Dimension>(box1), get<min_corner, Dimension>(box2))
            || !geometry::math::equals(get<max_corner, Dimension>(box1), get<max_corner, Dimension>(box2)))
        {
            return false;
        }
        return box_box<Dimension + 1, DimensionCount>::apply(box1, box2, strategy);
    }
};

template <std::size_t DimensionCount>
struct box_box<DimensionCount, DimensionCount>
{
    template <typename Box1, typename Box2, typename Strategy>
    static inline bool apply(Box1 const& , Box2 const& , Strategy const& )
    {
        return true;
    }
};

struct box_equals
{
    template <typename A, typename B, typename Strategy>
    static auto apply_impl(A const& a, B const& b, Strategy const& strategy, int)
        -> decltype(strategy.box_topology().equals().apply(a, b))
    { return strategy.box_topology().equals().apply(a, b); }

    template <typename A, typename B, typename Strategy>
    static bool apply_impl(A const& a, B const& b, Strategy const& strategy, long)
    { return box_box<0, dimension<A>::value>::apply(a, b, strategy); }

    template <typename A, typename B, typename Strategy>
    static bool apply(A const& a, B const& b, Strategy const& strategy)
    { return apply_impl(a, b, strategy, 0); }
};


struct segment_segment
{
    template <typename Segment1, typename Segment2, typename Strategy>
    static inline bool apply(Segment1 const& segment1, Segment2 const& segment2,
                             Strategy const& strategy)
    {
        indexed_point_view<Segment1 const, 0> const a0(segment1);
        indexed_point_view<Segment1 const, 1> const a1(segment1);
        indexed_point_view<Segment2 const, 0> const b0(segment2);
        indexed_point_view<Segment2 const, 1> const b1(segment2);
        return equals::equals_point_point(a0, b0, strategy)
            ? equals::equals_point_point(a1, b1, strategy)
            : equals::equals_point_point(a0, b1, strategy)
                && equals::equals_point_point(a1, b0, strategy);
    }
};


struct area_check
{
    template <typename Geometry1, typename Geometry2, typename Strategy>
    static inline bool apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        return geometry::math::equals(geometry::area(geometry1, strategy),
                                      geometry::area(geometry2, strategy));
    }
};


// Small helper structure do decide to use collect_vectors, or not
template <typename Strategy, typename CsTag>
struct use_collect_vectors
{
    static constexpr bool value = false;
};

template <typename Strategy>
struct use_collect_vectors<Strategy, cartesian_tag>
{
    static constexpr bool value = true;

    template <typename T, typename Point>
    using type = collected_vector_cartesian<T>;
};

template <typename CV>
struct use_collect_vectors<strategy::side::spherical_side_formula<CV>, spherical_tag>
{
    static constexpr bool value = true;

    template <typename T, typename Point>
    using type = collected_vector_spherical<T, Point>;
};


template <typename TrivialCheck>
struct equals_by_collection
{
    template <typename Geometry1, typename Geometry2, typename Strategy>
    static inline bool apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        if (! TrivialCheck::apply(geometry1, geometry2, strategy))
        {
            return false;
        }

        using calculation_type = typename geometry::select_most_precise
            <
                typename select_coordinate_type
                    <
                        Geometry1, Geometry2
                    >::type,
                double
            >::type;

        using collected_vector_type = typename use_collect_vectors
            <
                decltype(std::declval<Strategy>().side()),
                typename Strategy::cs_tag
            >::template type
                <
                    calculation_type,
                    geometry::point_type_t<Geometry1>
                >;

        std::vector<collected_vector_type> c1, c2;

        geometry::collect_vectors(c1, geometry1);
        geometry::collect_vectors(c2, geometry2);

        if (boost::size(c1) != boost::size(c2))
        {
            return false;
        }

        std::sort(c1.begin(), c1.end());
        std::sort(c2.begin(), c2.end());

        // Check if these vectors are equal.
        return std::equal(c1.begin(), c1.end(), c2.begin());
    }
};

template<typename Geometry1, typename Geometry2>
struct equals_by_relate
    : detail::relate::relate_impl
        <
            detail::de9im::static_mask_equals_type,
            Geometry1,
            Geometry2
        >
{};

template <typename Ring1, typename Ring2, typename Strategy>
bool matching_ring(Ring1 const& a, Ring2 const& b, Strategy const& strategy)
{
    if (geometry::point_order<Ring1>::value != geometry::point_order<Ring2>::value
        || geometry::closure<Ring1>::value != geometry::closure<Ring2>::value
        || boost::size(a) != boost::size(b))
        return false;

    auto first = boost::begin(a), end = boost::end(a);
    auto start = boost::begin(b), stop = boost::end(b);
    if (first == end)
        return true;
    if (geometry::closure<Ring1>::value == closed)
    {
        --end;
        --stop;
    }
    auto equal = [&](auto const& p, auto const& q)
    {
        return equals::equals_point_point(p, q, strategy);
    };
    auto current = std::find_if(start, stop, [&](auto const& p) { return equal(*first, p); });
    if (current == stop)
        return false;
    for (; first != end; ++first)
    {
        if (!equal(*first, *current))
            return false;
        if (++current == stop)
            current = start;
    }
    return true;
}

template <typename A, typename B, typename Strategy>
bool matching_boundary(A const& a, B const& b, Strategy const& strategy)
{
    if constexpr (std::is_same<tag_t<A>, ring_tag>::value && std::is_same<tag_t<B>, ring_tag>::value)
        return matching_ring(a, b, strategy);
    else if constexpr (std::is_same<tag_t<A>, polygon_tag>::value && std::is_same<tag_t<B>, polygon_tag>::value)
    {
        auto const& as = interior_rings(a);
        auto const& bs = interior_rings(b);
        return boost::size(as) == boost::size(bs)
            && matching_ring(exterior_ring(a), exterior_ring(b), strategy)
            && std::equal(boost::begin(as), boost::end(as), boost::begin(bs),
                          [&](auto const& p, auto const& q) { return matching_ring(p, q, strategy); });
    }
    else return false;
}

template <typename Geometry1, typename Geometry2, typename Strategy>
auto areal_can_match(Geometry1 const& a, Geometry2 const& b, Strategy const& strategy, int)
    -> decltype(strategy.areal_equality().bounds_match(a, b), bool())
{
    auto const filter = strategy.areal_equality();
    return filter.bounds_match(a, b) && filter.area_can_match(a, b);
}

template <typename Geometry1, typename Geometry2, typename Strategy>
bool areal_can_match(Geometry1 const&, Geometry2 const&, Strategy const&, long)
{ return true; }

template <typename Geometry1, typename Geometry2, typename Strategy>
bool equals_areal_exact(Geometry1 const& a, Geometry2 const& b, Strategy const& strategy)
{
    if (geometry::is_empty(a) || geometry::is_empty(b))
        return geometry::is_empty(a) && geometry::is_empty(b);
    // Matching vertices prove equality, but a different subdivision does not
    // disprove equality. Different exact envelopes do.
    if (matching_boundary(a, b, strategy))
        return true;
    return areal_can_match(a, b, strategy, 0)
        && equals_by_relate<Geometry1, Geometry2>::apply(a, b, strategy);
}

// Use either collect_vectors or relate
// NOTE: the result could be conceptually different for invalid
// geometries in different coordinate systems because collect_vectors
// and relate treat invalid geometries differently.
template<typename TrivialCheck>
struct equals_by_collection_or_relate
{
    template <typename Strategy>
    using use_vectors = std::integral_constant<bool,
        !strategies::relate::services::exact_predicates<Strategy>::value
        && use_collect_vectors
        <
            decltype(std::declval<Strategy>().side()),
            typename Strategy::cs_tag
        >::value>;

    template
    <
        typename Geometry1, typename Geometry2, typename Strategy,
        std::enable_if_t<use_vectors<Strategy>::value, int> = 0
    >
    static inline bool apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        return equals_by_collection<TrivialCheck>::apply(geometry1, geometry2, strategy);
    }

    template
    <
        typename Geometry1, typename Geometry2, typename Strategy,
        std::enable_if_t<! use_vectors<Strategy>::value, int> = 0
    >
    static inline bool apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        if constexpr (strategies::relate::services::exact_predicates<Strategy>::value)
            return equals_areal_exact(geometry1, geometry2, strategy);
        else
            return equals_by_relate<Geometry1, Geometry2>::apply(geometry1, geometry2, strategy);
    }
};

struct equals_always_false
{
    template <typename Geometry1, typename Geometry2, typename Strategy>
    static inline bool apply(Geometry1 const& , Geometry2 const& , Strategy const& )
    {
        return false;
    }
};


}} // namespace detail::equals
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename P1, typename P2, std::size_t DimensionCount, bool Reverse>
struct equals<P1, P2, point_tag, point_tag, pointlike_tag, pointlike_tag, DimensionCount, Reverse>
    : detail::equals::point_point
{};

template <typename MultiPoint1, typename MultiPoint2, std::size_t DimensionCount, bool Reverse>
struct equals<MultiPoint1, MultiPoint2, multi_point_tag, multi_point_tag, pointlike_tag, pointlike_tag, DimensionCount, Reverse>
    : detail::equals::equals_by_relate<MultiPoint1, MultiPoint2>
{};

template <typename MultiPoint, typename Point, std::size_t DimensionCount, bool Reverse>
struct equals<Point, MultiPoint, point_tag, multi_point_tag, pointlike_tag, pointlike_tag, DimensionCount, Reverse>
    : detail::equals::equals_by_relate<Point, MultiPoint>
{};

template <typename Box1, typename Box2, std::size_t DimensionCount, bool Reverse>
struct equals<Box1, Box2, box_tag, box_tag, areal_tag, areal_tag, DimensionCount, Reverse>
    : detail::equals::box_equals
{};


template <typename Ring1, typename Ring2, bool Reverse>
struct equals<Ring1, Ring2, ring_tag, ring_tag, areal_tag, areal_tag, 2, Reverse>
    : detail::equals::equals_by_collection_or_relate<detail::equals::area_check>
{};


template <typename Polygon1, typename Polygon2, bool Reverse>
struct equals<Polygon1, Polygon2, polygon_tag, polygon_tag, areal_tag, areal_tag, 2, Reverse>
    : detail::equals::equals_by_collection_or_relate<detail::equals::area_check>
{};


template <typename Polygon, typename Ring, bool Reverse>
struct equals<Polygon, Ring, polygon_tag, ring_tag, areal_tag, areal_tag, 2, Reverse>
    : detail::equals::equals_by_collection_or_relate<detail::equals::area_check>
{};


template <typename Ring, typename Box, bool Reverse>
struct equals<Ring, Box, ring_tag, box_tag, areal_tag, areal_tag, 2, Reverse>
    : detail::equals::equals_by_collection<detail::equals::area_check>
{};


template <typename Polygon, typename Box, bool Reverse>
struct equals<Polygon, Box, polygon_tag, box_tag, areal_tag, areal_tag, 2, Reverse>
    : detail::equals::equals_by_collection<detail::equals::area_check>
{};

template <typename Segment1, typename Segment2, std::size_t DimensionCount, bool Reverse>
struct equals<Segment1, Segment2, segment_tag, segment_tag, linear_tag, linear_tag, DimensionCount, Reverse>
    : detail::equals::segment_segment
{};

template <typename LineString1, typename LineString2, bool Reverse>
struct equals<LineString1, LineString2, linestring_tag, linestring_tag, linear_tag, linear_tag, 2, Reverse>
    : detail::equals::equals_by_relate<LineString1, LineString2>
{};

template <typename LineString, typename MultiLineString, bool Reverse>
struct equals<LineString, MultiLineString, linestring_tag, multi_linestring_tag, linear_tag, linear_tag, 2, Reverse>
    : detail::equals::equals_by_relate<LineString, MultiLineString>
{};

template <typename MultiLineString1, typename MultiLineString2, bool Reverse>
struct equals<MultiLineString1, MultiLineString2, multi_linestring_tag, multi_linestring_tag, linear_tag, linear_tag, 2, Reverse>
    : detail::equals::equals_by_relate<MultiLineString1, MultiLineString2>
{};

template <typename LineString, typename Segment, bool Reverse>
struct equals<LineString, Segment, linestring_tag, segment_tag, linear_tag, linear_tag, 2, Reverse>
    : detail::equals::equals_by_relate<LineString, Segment>
{};

template <typename MultiLineString, typename Segment, bool Reverse>
struct equals<MultiLineString, Segment, multi_linestring_tag, segment_tag, linear_tag, linear_tag, 2, Reverse>
    : detail::equals::equals_by_relate<MultiLineString, Segment>
{};


template <typename MultiPolygon1, typename MultiPolygon2, bool Reverse>
struct equals
    <
        MultiPolygon1, MultiPolygon2,
        multi_polygon_tag, multi_polygon_tag,
        areal_tag, areal_tag,
        2,
        Reverse
    >
    : detail::equals::equals_by_collection_or_relate<detail::equals::area_check>
{};


template <typename Polygon, typename MultiPolygon, bool Reverse>
struct equals
    <
        Polygon, MultiPolygon,
        polygon_tag, multi_polygon_tag,
        areal_tag, areal_tag,
        2,
        Reverse
    >
    : detail::equals::equals_by_collection_or_relate<detail::equals::area_check>
{};

template <typename MultiPolygon, typename Ring, bool Reverse>
struct equals
    <
        MultiPolygon, Ring,
        multi_polygon_tag, ring_tag,
        areal_tag, areal_tag,
        2,
        Reverse
    >
    : detail::equals::equals_by_collection_or_relate<detail::equals::area_check>
{};


// NOTE: degenerated linear geometries, e.g. segment or linestring containing
//   2 equal points, are considered to be invalid. Though theoretically
//   degenerated segments and linestrings could be treated as points and
//   multi-linestrings as multi-points.
//   This reasoning could also be applied to boxes.

template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2, std::size_t DimensionCount>
struct equals<Geometry1, Geometry2, Tag1, Tag2, pointlike_tag, linear_tag, DimensionCount, false>
    : detail::equals::equals_always_false
{};

template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2, std::size_t DimensionCount>
struct equals<Geometry1, Geometry2, Tag1, Tag2, linear_tag, pointlike_tag, DimensionCount, false>
    : detail::equals::equals_always_false
{};

template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2, std::size_t DimensionCount>
struct equals<Geometry1, Geometry2, Tag1, Tag2, pointlike_tag, areal_tag, DimensionCount, false>
    : detail::equals::equals_always_false
{};

template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2, std::size_t DimensionCount>
struct equals<Geometry1, Geometry2, Tag1, Tag2, areal_tag, pointlike_tag, DimensionCount, false>
    : detail::equals::equals_always_false
{};

template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2, std::size_t DimensionCount>
struct equals<Geometry1, Geometry2, Tag1, Tag2, linear_tag, areal_tag, DimensionCount, false>
    : detail::equals::equals_always_false
{};

template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2, std::size_t DimensionCount>
struct equals<Geometry1, Geometry2, Tag1, Tag2, areal_tag, linear_tag, DimensionCount, false>
    : detail::equals::equals_always_false
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_EQUALS_IMPLEMENTATION_HPP

