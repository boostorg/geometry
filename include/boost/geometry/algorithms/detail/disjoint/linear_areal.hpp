// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013-2018.
// Modifications copyright (c) 2013-2018, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_LINEAR_AREAL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_LINEAR_AREAL_HPP

#include <iterator>

#include <boost/range.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/algorithms/detail/disjoint/linear_linear.hpp>
#include <boost/geometry/algorithms/detail/disjoint/linear_segment_or_box.hpp>
#include <boost/geometry/algorithms/detail/disjoint/multirange_geometry.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_box.hpp>
#include <boost/geometry/algorithms/detail/disjoint/segment_box.hpp>

#include <boost/geometry/algorithms/dispatch/disjoint.hpp>
#include <boost/geometry/algorithms/dispatch/disjoint_with_info.hpp>

#include <boost/geometry/util/select_point_type.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{

template <typename Geometry1, typename Geometry2,
          typename Tag1 = typename tag<Geometry1>::type,
          typename Tag1OrMulti = typename tag_cast<Tag1, multi_tag>::type>
struct disjoint_no_intersections_policy
{
    /*!
    \tparam Strategy point_in_geometry strategy
    */
    typedef typename point_type<Geometry1>::type point1_type;

    template <typename Strategy>
    static inline bool apply(Geometry1 const& g1,
                             Geometry2 const& g2,
                             Strategy const& strategy)
    {
        point1_type p;
        geometry::point_on_border(p, g1);

        return !geometry::covered_by(p, g2, strategy);
    }

    template <typename Point, typename Strategy>
    static inline bool apply(Geometry1 const& g1,
                             Geometry2 const& g2,
                             Point& p,
                             Strategy const& strategy)
    {
        geometry::point_on_border(p, g1);

        return !geometry::covered_by(p, g2, strategy);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag1>
struct disjoint_no_intersections_policy<Geometry1, Geometry2, Tag1, multi_tag>
{
    /*!
    \tparam Strategy point_in_geometry strategy
    */
    template <typename Strategy>
    static inline bool apply(Geometry1 const& g1,
                             Geometry2 const& g2,
                             Strategy const& strategy)
    {
        // TODO: use partition or rtree on g2
        typedef typename boost::range_iterator<Geometry1 const>::type iterator;
        for ( iterator it = boost::begin(g1) ; it != boost::end(g1) ; ++it )
        {
            typedef typename boost::range_value<Geometry1 const>::type value_type;
            if ( ! disjoint_no_intersections_policy<value_type const, Geometry2>
                    ::apply(*it, g2, strategy) )
            {
                return false;
            }
        }
        return true;
    }

    typedef typename point_type<Geometry1>::type point1_type;

    template <typename Point, typename Strategy>
    static inline bool apply(Geometry1 const& g1,
                             Geometry2 const& g2,
                             Point& p,
                             Strategy const& strategy)
    {
        // TODO: use partition or rtree on g2
        typedef typename boost::range_iterator<Geometry1 const>::type iterator;
        for ( iterator it = boost::begin(g1) ; it != boost::end(g1) ; ++it )
        {
            typedef typename boost::range_value<Geometry1 const>::type value_type;
            if ( ! disjoint_no_intersections_policy<value_type const, Geometry2>
                    ::apply(*it, g2, p, strategy) )
            {
                return false;
            }
        }
        return true;
    }
};

template<typename Geometry1, typename Geometry2,
         typename NoIntersectionsPolicy
                    = disjoint_no_intersections_policy
                      <Geometry1, Geometry2> >
struct disjoint_linear_areal_with_info
{
    typedef typename geometry::select_point_type<Geometry1, Geometry2>::type
            p_type;
    typedef segment_intersection_points<p_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Geometry1 const& g1, Geometry2 const& g2, Strategy const& strategy)
    {
        intersection_return_type res =
        disjoint_linear_with_info<Geometry1, Geometry2>::apply(g1, g2, strategy);

        if ( res.count != 0 )
        {
            return res;
        }

        p_type p;
        if (!NoIntersectionsPolicy
                ::apply(g1, g2, p,
                        strategy.template get_point_in_geometry_strategy
                        <
                            Geometry1,
                            Geometry2
                        >()))
        {
            res.count = 1;
            res.intersections[0] = p;
            return res;
        }

        return intersection_return_type();
    }
};


template<typename Geometry1, typename Geometry2,
         typename NoIntersectionsPolicy
                    = disjoint_no_intersections_policy<Geometry1, Geometry2> >
struct disjoint_linear_areal
{
    /*!
    \tparam Strategy relate (segments intersection) strategy
    */
    template <typename Strategy>
    static inline bool apply(Geometry1 const& g1,
                             Geometry2 const& g2,
                             Strategy const& strategy)
    {
        // if there are intersections - return false
        if ( !disjoint_linear<Geometry1, Geometry2>::apply(g1, g2, strategy) )
        {
            return false;
        }

        return NoIntersectionsPolicy
                ::apply(g1, g2,
                        strategy.template get_point_in_geometry_strategy
                                   <
                                     Geometry1,
                                     Geometry2
                                   >());
    }
};


template
<
    typename Segment,
    typename Areal,
    typename Tag = typename tag<Areal>::type
>
struct disjoint_segment_areal_with_info
    : not_implemented<Segment, Areal>
{};


template
<
    typename Segment,
    typename Areal,
    typename Tag = typename tag<Areal>::type
>
struct disjoint_segment_areal
    : not_implemented<Segment, Areal>
{};


template <typename Segment, typename Polygon>
class disjoint_segment_areal_with_info<Segment, Polygon, polygon_tag>
{
    typedef typename geometry::select_point_type<Segment, Polygon>::type p_type;
    typedef segment_intersection_points<p_type> intersection_return_type;

private:
    template <typename InteriorRings, typename Strategy>
    static inline
    intersection_return_type
    check_interior_rings(InteriorRings const& interior_rings,
                         Segment const& segment,
                         Strategy const& strategy)
    {
        typedef typename boost::range_value<InteriorRings>::type ring_type;

        typedef disjoint_geometry_to_query_geometry
            <
                Segment,
                Strategy,
                disjoint_range_segment_or_box_with_info
                    <
                        ring_type, closure<ring_type>::value, Segment
                    >
            > unary_predicate_type;

        intersection_return_type res = detail::check_iterator_range_with_info
            <
                Segment, unary_predicate_type
            >::apply(boost::begin(interior_rings),
                     boost::end(interior_rings),
                     unary_predicate_type(segment, strategy));

        return res;
    }


public:
    template <typename IntersectionStrategy>
    static inline intersection_return_type
    apply(Segment const& segment,
          Polygon const& polygon,
          IntersectionStrategy const& strategy)
    {
        typedef typename geometry::ring_type<Polygon>::type ring;

        intersection_return_type res = disjoint_range_segment_or_box_with_info
        <
            ring,
            closure<Polygon>::value,
            Segment
        >::apply(geometry::exterior_ring(polygon), segment, strategy);

        if ( res.count != 0 )
        {
            return res;
        }


        res = check_interior_rings(geometry::interior_rings(polygon),
                                   segment,
                                   strategy);

        if ( res.count != 0 )
        {
            return res;
        }

        typename geometry::select_point_type<Segment, Polygon>::type p;
        detail::assign_point_from_index<0>(segment, p);

        if (geometry::covered_by(p, polygon,
                    strategy.template get_point_in_geometry_strategy
                                      <
                                        Segment,
                                        Polygon
                                      >()))
        {
            intersection_return_type res;
            res.count = 1;
            res.intersections[0] = p;
            return res;
        }

        intersection_return_type res_empty;
        res_empty.count = 0;
        return res_empty;

    }
};


template <typename Segment, typename Polygon>
class disjoint_segment_areal<Segment, Polygon, polygon_tag>
{

public:
    template <typename IntersectionStrategy>
    static inline bool apply(Segment const& segment,
                             Polygon const& polygon,
                             IntersectionStrategy const& strategy)
    {
        return disjoint_segment_areal_with_info<Segment, Polygon>
                ::apply(segment, polygon, strategy).count == 0;
    }
};


template <typename Segment, typename MultiPolygon>
struct disjoint_segment_areal_with_info<Segment, MultiPolygon, multi_polygon_tag>
{
    typedef typename geometry::select_point_type<Segment, MultiPolygon>::type
            p_type;
    typedef segment_intersection_points<p_type> intersection_return_type;

    template <typename IntersectionStrategy>
    static inline intersection_return_type
    apply(Segment const& segment, MultiPolygon const& multipolygon,
          IntersectionStrategy const& strategy)
    {
        return multirange_constant_size_geometry_with_info
            <
                MultiPolygon, Segment
            >::apply(multipolygon, segment, strategy);
    }
};

template <typename Segment, typename MultiPolygon>
struct disjoint_segment_areal<Segment, MultiPolygon, multi_polygon_tag>
{
    template <typename IntersectionStrategy>
    static inline bool apply(Segment const& segment,
                             MultiPolygon const& multipolygon,
                             IntersectionStrategy const& strategy)
    {
        return multirange_constant_size_geometry
            <
                MultiPolygon, Segment
            >::apply(multipolygon, segment, strategy);
    }
};


template <typename Segment, typename Ring>
struct disjoint_segment_areal_with_info<Segment, Ring, ring_tag>
{
    typedef typename geometry::select_point_type<Segment, Ring>::type
            p_type;
    typedef segment_intersection_points<p_type> intersection_return_type;

    template <typename IntersectionStrategy>
    static inline intersection_return_type
    apply(Segment const& segment,
          Ring const& ring,
          IntersectionStrategy const& strategy)
    {
        intersection_return_type res = disjoint_range_segment_or_box_with_info
                         <
                             Ring, closure<Ring>::value, Segment
                         >::apply(ring, segment, strategy);

        if ( res.count != 0 )
        {
            return res;
        }

        typename point_type<Segment>::type p;
        detail::assign_point_from_index<0>(segment, p);

        if (geometry::covered_by(p, ring,
                    strategy.template get_point_in_geometry_strategy
                                       <
                                         Segment,
                                         Ring
                                       >()))
        {
            intersection_return_type res;
            res.count = 1;
            res.intersections[0] = p;
            return res;
        }

        intersection_return_type res_empty;
        res_empty.count = 0;
        return res_empty;
    }
};

template <typename Segment, typename Ring>
struct disjoint_segment_areal<Segment, Ring, ring_tag>
{
    template <typename IntersectionStrategy>
    static inline bool apply(Segment const& segment,
                             Ring const& ring,
                             IntersectionStrategy const& strategy)
    {
        return disjoint_segment_areal_with_info<Segment, Ring, ring_tag>
                ::apply(segment, ring, strategy).count == 0;
    }
};


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Linear, typename Areal>
struct disjoint<Linear, Areal, 2, linear_tag, areal_tag, false>
    : public detail::disjoint::disjoint_linear_areal<Linear, Areal>
{};


template <typename Areal, typename Linear>
struct disjoint<Areal, Linear, 2, areal_tag, linear_tag, false>
{
    template <typename Strategy>
    static inline bool apply(Areal const& areal, Linear const& linear,
                             Strategy const& strategy)
    {
        return detail::disjoint::disjoint_linear_areal
            <
                Linear, Areal
            >::apply(linear, areal, strategy);
    }
};


template <typename Areal, typename Segment>
struct disjoint<Areal, Segment, 2, areal_tag, segment_tag, false>
{
    template <typename Strategy>
    static inline bool apply(Areal const& g1, Segment const& g2,
                             Strategy const& strategy)
    {
        return detail::disjoint::disjoint_segment_areal
            <
                Segment, Areal
            >::apply(g2, g1, strategy);
    }
};


template <typename Segment, typename Areal>
struct disjoint<Segment, Areal, 2, segment_tag, areal_tag, false>
    : detail::disjoint::disjoint_segment_areal<Segment, Areal>
{};

//disjoint with info

template <typename Linear, typename Areal>
struct disjoint_with_info<Linear, Areal, 2, linear_tag, areal_tag, false>
    : public detail::disjoint::disjoint_linear_areal_with_info<Linear, Areal>
{};


template <typename Areal, typename Linear>
struct disjoint_with_info<Areal, Linear, 2, areal_tag, linear_tag, false>
{
    typedef typename point_type<Areal>::type p_type;

    typedef segment_intersection_points<p_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Areal const& areal, Linear const& linear,
          Strategy const& strategy)
    {
        return detail::disjoint::disjoint_linear_areal_with_info
            <
                Linear, Areal
            >::apply(linear, areal, strategy);
    }
};

template <typename Areal, typename Segment>
struct disjoint_with_info<Areal, Segment, 2, areal_tag, segment_tag, false>
{
    typedef typename point_type<Areal>::type p_type;

    typedef segment_intersection_points<p_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Areal const& g1, Segment const& g2,
          Strategy const& strategy)
    {
        return detail::disjoint::disjoint_segment_areal_with_info
            <
                Segment, Areal
            >::apply(g2, g1, strategy);
    }
};


template <typename Segment, typename Areal>
struct disjoint_with_info<Segment, Areal, 2, segment_tag, areal_tag, false>
    : detail::disjoint::disjoint_segment_areal_with_info<Segment, Areal>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_LINEAR_AREAL_HPP
