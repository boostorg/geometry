// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_MULTIPOINT_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_MULTIPOINT_GEOMETRY_HPP

#include <algorithm>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/less.hpp>

#if defined(BOOST_GEOMETRY_DISJOINT_USE_PARTITION)
#include <cstddef>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/partition.hpp>
#endif

#include <boost/geometry/algorithms/dispatch/disjoint.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{


template <typename Geometry>
class unary_disjoint_geometry_to_point
{
public:
    unary_disjoint_geometry_to_point(Geometry const& geometry)
        : m_geometry(geometry)
    {}

    template <typename Point>
    inline bool apply(Point const& point) const
    {
        return
            dispatch::disjoint<Point, Geometry>::apply(point, m_geometry);
    }

private:
    Geometry const& m_geometry;
};


template<typename MultiPoint, typename ConstantSizeGeometry>
struct multipoint_constant_size_geometry
{
    static inline bool apply(MultiPoint const& multipoint,
                             ConstantSizeGeometry const& constant_size_geometry)
    {
        detail::disjoint::unary_disjoint_geometry_to_point
            <
                ConstantSizeGeometry
            > predicate(constant_size_geometry);

        return detail::check_iterator_range
            <
                detail::disjoint::unary_disjoint_geometry_to_point
                    <
                        ConstantSizeGeometry
                    >
            >::apply(boost::begin(multipoint), boost::end(multipoint),
                     predicate);
    }

    static inline bool apply(ConstantSizeGeometry const& constant_size_geometry,
                             MultiPoint const& multipoint)
    {
        return apply(multipoint, constant_size_geometry);
    }
};


#if defined(BOOST_GEOMETRY_DISJOINT_USE_PARTITION)
template<typename MultiPoint1, typename MultiPoint2>
class multipoint_multipoint
{
private:
    struct point_expand
    {
        template <typename Box, typename Point>
        static inline void apply(Box& total, Point const& point)
        {
            geometry::expand(total, point);
        }
    };

    struct overlaps_point
    {
        template <typename Box, typename Point>
        static inline bool apply(Box const& box, Point const& point)
        {
            return !dispatch::disjoint<Point, Box>::apply(point, box);
        }
    };

    struct two_point_visitor
    {
        std::size_t count;

        two_point_visitor()
            : count(0)
        {}

        template <typename Point1, typename Point2>
        inline void apply(Point1 const& item1, Point2 const& item2)
        {
            if ( count == 0 && geometry::equals(item1, item2) )
            {
                ++count;
            }
        }
    };

public:
    static inline bool apply(MultiPoint1 const& multipoint1,
                             MultiPoint2 const& multipoint2)
    {
        BOOST_ASSERT( boost::size(multipoint1) <= boost::size(multipoint2) );

        typedef typename boost::range_value<MultiPoint1>::type point1_type;

        two_point_visitor visitor;

        geometry::partition
            <
                geometry::model::box
                    <
                        typename boost::range_value<MultiPoint1>::type
                    >,
                point_expand,
                overlaps_point
            >::apply(multipoint1, multipoint2, visitor);

        return visitor.count == 0;
    }
};
#else
template<typename MultiPoint1, typename MultiPoint2>
struct multipoint_multipoint
{
    static inline bool apply(MultiPoint1 const& multipoint1,
                             MultiPoint2 const& multipoint2)
    {
        BOOST_ASSERT( boost::size(multipoint1) <= boost::size(multipoint2) );

        typedef typename boost::range_value<MultiPoint1>::type point1_type;

        std::vector<point1_type> points1(boost::begin(multipoint1),
                                         boost::end(multipoint1));
        std::sort(points1.begin(), points1.end(), detail::relate::less());

        
        typedef typename boost::range_iterator
            <
                MultiPoint2 const
            >::type iterator2;

        for (iterator2 it = boost::begin(multipoint2);
             it != boost::end(multipoint2); ++it)
        {
            if ( std::binary_search(points1.begin(), points1.end(),
                                    *it, detail::relate::less()) )
            {
                return false;
            }
        }

        return true;
    }
};
#endif


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template<typename Point, typename MultiPoint, std::size_t DimensionCount>
struct disjoint
    <
        Point, MultiPoint, DimensionCount, point_tag, multi_point_tag, false
    > : detail::disjoint::multipoint_constant_size_geometry<MultiPoint, Point>
{};


template<typename MultiPoint, typename Segment, std::size_t DimensionCount>
struct disjoint
    <
        MultiPoint, Segment, DimensionCount, multi_point_tag, segment_tag, false
    > : detail::disjoint::multipoint_constant_size_geometry<MultiPoint, Segment>
{};


template<typename MultiPoint, typename Box, std::size_t DimensionCount>
struct disjoint
    <
        MultiPoint, Box, DimensionCount, multi_point_tag, box_tag, false
    > : detail::disjoint::multipoint_constant_size_geometry<MultiPoint, Box>
{};


template<typename MultiPoint1, typename MultiPoint2, std::size_t DimensionCount>
struct disjoint
    <
        MultiPoint1, MultiPoint2, DimensionCount,
        multi_point_tag, multi_point_tag, false
    >
{
    static inline bool apply(MultiPoint1 const& multipoint1,
                             MultiPoint2 const& multipoint2)
    {
        if ( boost::size(multipoint2) < boost::size(multipoint1) )
        {
            return detail::disjoint::multipoint_multipoint
                <
                    MultiPoint2, MultiPoint1
                >::apply(multipoint2, multipoint1);
        } 

        return detail::disjoint::multipoint_multipoint
            <
                MultiPoint1, MultiPoint2
            >::apply(multipoint1, multipoint2);
   }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_MULTIPOINT_GEOMETRY_HPP
