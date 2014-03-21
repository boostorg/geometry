// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_STRATEGY_AGNOSTIC_RELATE_HPP
#define BOOST_GEOMETRY_STRATEGY_AGNOSTIC_RELATE_HPP

#include <boost/geometry/algorithms/detail/relate/relate.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace relate
{

template <typename StaticMask>
struct relate
{
    template <typename Geometry1, typename Geometry2>
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        return detail::relate::relate<StaticMask>(geometry1, geometry2);
    }
};

} // namespace relate

namespace within
{

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

// P/L

//template <typename Point, typename Linestring, typename AnyCS1, typename AnyCS2>
//struct default_strategy<point_tag, linestring_tag, point_tag, linestring_tag, AnyCS1, AnyCS2, Point, Linestring>
//{
//    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
//};
//
//template <typename Point, typename MultiLinestring, typename AnyCS1, typename AnyCS2>
//struct default_strategy<point_tag, multi_linestring_tag, point_tag, multi_linestring_tag, AnyCS1, AnyCS2, Point, MultiLinestring>
//{
//    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
//};

// L/L

template <typename Linestring1, typename Linestring2, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, linestring_tag, linestring_tag, linestring_tag, AnyCS1, AnyCS2, Linestring1, Linestring2>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename Linestring, typename MultiLinestring, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, multi_linestring_tag, linestring_tag, multi_linestring_tag, AnyCS1, AnyCS2, Linestring, MultiLinestring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename MultiLinestring, typename Linestring, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, linestring_tag, multi_linestring_tag, linestring_tag, AnyCS1, AnyCS2, MultiLinestring, Linestring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

// L/A

template <typename Linestring, typename Ring, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, ring_tag, linestring_tag, areal_tag, AnyCS1, AnyCS2, Linestring, Ring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename MultiLinestring, typename Ring, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, ring_tag, multi_linestring_tag, areal_tag, AnyCS1, AnyCS2, MultiLinestring, Ring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename Linestring, typename Polygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, polygon_tag, linestring_tag, areal_tag, AnyCS1, AnyCS2, Linestring, Polygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename MultiLinestring, typename Polygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, polygon_tag, multi_linestring_tag, areal_tag, AnyCS1, AnyCS2, MultiLinestring, Polygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename Linestring, typename MultiPolygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, multi_polygon_tag, linestring_tag, areal_tag, AnyCS1, AnyCS2, Linestring, MultiPolygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

template <typename MultiLinestring, typename MultiPolygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, multi_polygon_tag, multi_linestring_tag, areal_tag, AnyCS1, AnyCS2, MultiLinestring, MultiPolygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_within> type;
};

} // namespace services

#endif


}} // namespace strategy::within



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace strategy { namespace covered_by { namespace services
{

// P/L

//template <typename Point, typename Linestring, typename AnyCS1, typename AnyCS2>
//struct default_strategy<point_tag, linestring_tag, point_tag, linestring_tag, AnyCS1, AnyCS2, Point, Linestring>
//{
//    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
//};
//
//template <typename Point, typename MultiLinestring, typename AnyCS1, typename AnyCS2>
//struct default_strategy<point_tag, multi_linestring_tag, point_tag, multi_linestring_tag, AnyCS1, AnyCS2, Point, MultiLinestring>
//{
//    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
//};

// L/L

template <typename Linestring1, typename Linestring2, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, linestring_tag, linestring_tag, linestring_tag, AnyCS1, AnyCS2, Linestring1, Linestring2>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename Linestring, typename MultiLinestring, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, multi_linestring_tag, linestring_tag, multi_linestring_tag, AnyCS1, AnyCS2, Linestring, MultiLinestring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename MultiLinestring, typename Linestring, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, linestring_tag, multi_linestring_tag, linestring_tag, AnyCS1, AnyCS2, MultiLinestring, Linestring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

// L/A

template <typename Linestring, typename Ring, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, ring_tag, linestring_tag, areal_tag, AnyCS1, AnyCS2, Linestring, Ring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename MultiLinestring, typename Ring, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, ring_tag, multi_linestring_tag, areal_tag, AnyCS1, AnyCS2, MultiLinestring, Ring>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename Linestring, typename Polygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, polygon_tag, linestring_tag, areal_tag, AnyCS1, AnyCS2, Linestring, Polygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename MultiLinestring, typename Polygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, polygon_tag, multi_linestring_tag, areal_tag, AnyCS1, AnyCS2, MultiLinestring, Polygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename Linestring, typename MultiPolygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<linestring_tag, multi_polygon_tag, linestring_tag, areal_tag, AnyCS1, AnyCS2, Linestring, MultiPolygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

template <typename MultiLinestring, typename MultiPolygon, typename AnyCS1, typename AnyCS2>
struct default_strategy<multi_linestring_tag, multi_polygon_tag, multi_linestring_tag, areal_tag, AnyCS1, AnyCS2, MultiLinestring, MultiPolygon>
{
    typedef strategy::relate::relate<detail::relate::static_mask_covered_by> type;
};

}}} // namespace strategy::covered_by::services
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGY_AGNOSTIC_RELATE_HPP
