// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html


#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_POINTLIKE_AREAL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_POINTLIKE_AREAL_HPP

#include <vector>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/algorithms/detail/not.hpp>
#include <boost/geometry/algorithms/detail/partition_lambda.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>

#include <boost/geometry/algorithms/detail/overlay/pointlike_linear.hpp>

#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>

// TEMP
#include <boost/geometry/strategies/envelope/cartesian.hpp>
#include <boost/geometry/strategies/envelope/geographic.hpp>
#include <boost/geometry/strategies/envelope/spherical.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


// difference/intersection of multipoint-multipolygon
template
<
    typename MultiPoint,
    typename MultiPolygon,
    typename PointOut,
    overlay_type OverlayType,
    typename Policy
>
class multipoint_multipolygon_point
{
private:

    template <typename Iterator, typename Box, typename SizeT, typename Strategy>
    static inline void fill_box_pairs(Iterator first, Iterator last,
                                      std::vector<std::pair<Box, SizeT> > & box_pairs,
                                      Strategy const& strategy)
    {
        SizeT index = 0;
        for (; first != last; ++first, ++index)
        {
            box_pairs.push_back(
                std::make_pair(geometry::return_envelope<Box>(*first, strategy),
                               index));
        }
    }

    template <typename OutputIterator, typename Strategy>
    static inline OutputIterator get_common_points(MultiPoint const& multipoint,
                                                   MultiPolygon const& multipolygon,
                                                   OutputIterator oit,
                                                   Strategy const& strategy)
    {
        typedef geometry::model::point
            <
                typename geometry::coordinate_type<MultiPoint>::type,
                geometry::dimension<MultiPoint>::value,
                typename geometry::coordinate_system<MultiPoint>::type
            > point_type;
        typedef geometry::model::box<point_type> box_type;
        typedef std::pair<box_type, std::size_t> box_pair;
        std::vector<box_pair> box_pairs;
        box_pairs.reserve(boost::size(multipolygon));

        fill_box_pairs(boost::begin(multipolygon),
                       boost::end(multipolygon),
                       box_pairs, strategy);

        partition_lambda
            <
                box_type
            >(multipoint, box_pairs,
                [&strategy](auto& box, auto const& point)
                 {
                    geometry::expand(box, point, strategy);
                 },
                [&strategy](auto const& box, auto const& point)
                {
                    return ! geometry::disjoint(point, box, strategy);
                },
                [&strategy](auto& box, auto const& pair)
                {
                    geometry::expand(box, pair.first, strategy);
                },
                [&strategy](auto const& box, auto const& pair)
                {
                    return ! geometry::disjoint(box, pair.first, strategy);
                },
                [&](auto const& point, auto const& pair)
                {
                    action_selector_pl
                    <
                        PointOut, overlay_intersection
                    >::apply(point,
                            Policy::apply(point,
                                          range::at(multipolygon, pair.second),
                                          strategy),
                            oit);
                    return true;
                }
            );

        return oit;
    }

public:
    template <typename RobustPolicy, typename OutputIterator, typename Strategy>
    static inline OutputIterator apply(MultiPoint const& multipoint,
                                       MultiPolygon const& multipolygon,
                                       RobustPolicy const& robust_policy,
                                       OutputIterator oit,
                                       Strategy const& strategy)
    {
        typedef std::vector
            <
                typename boost::range_value<MultiPoint>::type
            > point_vector_type;

        point_vector_type common_points;

        // compute the common points
        get_common_points(multipoint, multipolygon,
                          std::back_inserter(common_points),
                          strategy);

        return multipoint_multipoint_point
            <
                MultiPoint, point_vector_type, PointOut, OverlayType
            >::apply(multipoint, common_points, robust_policy, oit, strategy);
    }
};


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DISPATCH


#ifndef DOXYGEN_NO_DISPATCH
namespace detail_dispatch { namespace overlay
{

// dispatch struct for pointlike-areal difference/intersection computation
template
<
    typename PointLike,
    typename Areal,
    typename PointOut,
    overlay_type OverlayType,
    typename Tag1,
    typename Tag2
>
struct pointlike_areal_point
    : not_implemented<PointLike, Areal, PointOut>
{};


template
<
    typename Point,
    typename Areal,
    typename PointOut,
    overlay_type OverlayType,
    typename Tag2
>
struct pointlike_areal_point
    <
        Point, Areal, PointOut, OverlayType, point_tag, Tag2
    > : detail::overlay::point_single_point
        <
            Point, Areal, PointOut, OverlayType,
            detail::not_<detail::disjoint::reverse_covered_by>
        >
{};


// TODO: Consider implementing Areal-specific version
//   calculating envelope first in order to reject Points without
//   calling disjoint for Rings and Polygons
template
<
    typename MultiPoint,
    typename Areal,
    typename PointOut,
    overlay_type OverlayType,
    typename Tag2
>
struct pointlike_areal_point
    <
        MultiPoint, Areal, PointOut, OverlayType, multi_point_tag, Tag2
    > : detail::overlay::multipoint_single_point
        <
            MultiPoint, Areal, PointOut, OverlayType,
            detail::not_<detail::disjoint::reverse_covered_by>
        >
{};


template
<
    typename MultiPoint,
    typename MultiPolygon,
    typename PointOut,
    overlay_type OverlayType
>
struct pointlike_areal_point
    <
        MultiPoint, MultiPolygon, PointOut, OverlayType, multi_point_tag, multi_polygon_tag
    > : detail::overlay::multipoint_multipolygon_point
        <
            MultiPoint, MultiPolygon, PointOut, OverlayType,
            detail::not_<detail::disjoint::reverse_covered_by>
        >
{};


}} // namespace detail_dispatch::overlay
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_POINTLIKE_AREAL_HPP
