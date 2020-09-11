// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013-2020.
// Modifications copyright (c) 2013-2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_AREAL_AREAL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_AREAL_AREAL_HPP

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/algorithms/detail/disjoint/linear_linear.hpp>
#include <boost/geometry/algorithms/detail/disjoint/segment_box.hpp>

#include <boost/geometry/algorithms/for_each.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{

template <typename Geometry1, typename Geometry2, typename Strategy>
inline bool point_on_border_covered_by(Geometry1 const& geometry1,
                                       Geometry2 const& geometry2,
                                       Strategy const& strategy)
{
    typename geometry::point_type<Geometry1>::type pt;
    return geometry::point_on_border(pt, geometry1)
        && geometry::covered_by(pt, geometry2, strategy);
}


/*!
\tparam Strategy point_in_geometry strategy
*/
template <typename Geometry1, typename Geometry2, typename Strategy>
inline bool rings_containing(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
{
    // TODO: This will be removed when IntersectionStrategy is replaced with
    //       UmbrellaStrategy

    return geometry::detail::any_range_of(geometry2, [&](auto const& range)
    {
        return point_on_border_covered_by(range, geometry1, strategy);
    });
}

template <typename Geometry1, typename Geometry2>
struct areal_areal_with_info
{
    typedef typename point_type<Geometry1>::type p_type;
    typedef segment_intersection_points<p_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {
        intersection_return_type res_dis
                = disjoint_linear_with_info<Geometry1, Geometry2>
                       ::apply(geometry1, geometry2, strategy);

        if ( res_dis.count > 0 )
        {
            return res_dis;
        }

        // If there is no intersection of segments, they might located
        // inside each other

        // We check that using a point on the border (external boundary),
        // and see if that is contained in the other geometry. And vice versa.

        if ( rings_containing(geometry1, geometry2,
                              strategy.template get_point_in_geometry_strategy
                                       <Geometry2, Geometry1>()) )
        {
             res_dis.count = 1;
             res_dis.intersections[0] = *points_begin(geometry2);
             return res_dis;
        }
        if ( rings_containing(geometry2, geometry1,
                              strategy.template get_point_in_geometry_strategy
                                       <Geometry1, Geometry2>()) )
        {
            res_dis.count = 1;
            res_dis.intersections[0] = *points_begin(geometry1);
            return res_dis;
        }

        return intersection_return_type();
        //return true;
    }
};

template <typename Geometry1, typename Geometry2>
struct areal_areal
{
    /*!
    \tparam Strategy relate (segments intersection) strategy
    */
    template <typename Strategy>
    static inline bool apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        if ( ! disjoint_linear<Geometry1, Geometry2>
               ::apply(geometry1, geometry2, strategy) )
        {
            return false;
        }

        // If there is no intersection of segments, they might located
        // inside each other

        // We check that using a point on the border (external boundary),
        // and see if that is contained in the other geometry. And vice versa.

        if ( rings_containing(geometry1, geometry2,
                              strategy.template get_point_in_geometry_strategy
                                       <Geometry2, Geometry1>())
          || rings_containing(geometry2, geometry1,
                              strategy.template get_point_in_geometry_strategy
                                       <Geometry1, Geometry2>()) )
        {
            return false;
        }

        return true;
    }
};

template <typename Areal, typename Box>
struct areal_box_with_info
{
    typedef typename point_type<Areal>::type areal_point_type;
    typedef segment_intersection_points<areal_point_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Areal const& areal,
          Box const& box,
          Strategy const& strategy)
    {
        intersection_return_type res =
                check_segments_range(geometry::segments_begin(areal),
                                     geometry::segments_end(areal),
                                     box, strategy);
        if ( res.count != 0 )
        {
            return res;
        }

        // If there is no intersection of any segment and box,
        // the box might be located inside areal geometry

        if ( point_on_border_covered_by(box, areal,
                strategy.template get_point_in_geometry_strategy<Box, Areal>()) )
        {
            res.count = 1;
            typename point_type<Box>::type box_point;
            set<0>(box_point, get<1,0>(box));
            set<1>(box_point, get<1,1>(box));
            res.intersections[0] = box_point;
            return res;
        }

        return intersection_return_type();
    }

private:
    template <typename SegIter, typename Strategy>
    static inline intersection_return_type
    check_segments_range(SegIter first,
                         SegIter last,
                         Box const& box,
                         Strategy const& strategy)
    {
        for ( ; first != last ; ++first)
        {
            typedef typename std::iterator_traits<SegIter>::value_type Segment;

            Segment seg = *first;
            intersection_return_type res
                    = disjoint_segment_box_with_info<Segment,Box>
                                ::apply(seg, box, strategy);
            if (res.count != 0)
            {
                return res;
            }
        }
        return intersection_return_type();
    }
};


template <typename Areal, typename Box>
struct areal_box
{
    /*!
    \tparam Strategy relate (segments intersection) strategy
    */
    template <typename Strategy>
    static inline bool apply(Areal const& areal,
                             Box const& box,
                             Strategy const& strategy)
    {
//        if ( ! check_segments_range(geometry::segments_begin(areal),
//                                    geometry::segments_end(areal),
//                                    box,
//                                    strategy.get_disjoint_segment_box_strategy()) )
        // TODO: This will be removed when UmbrellaStrategy is supported
        auto const ds = strategy.get_disjoint_segment_box_strategy();
        if (! geometry::all_segments_of(areal, [&](auto const& s)
              {
                  return disjoint_segment_box::apply(s, box, ds);
              }) )
        {
            return false;
        }

        // If there is no intersection of any segment and box,
        // the box might be located inside areal geometry

        if ( point_on_border_covered_by(box, areal,
                strategy.template get_point_in_geometry_strategy<Box, Areal>()) )
        {
            return false;
        }

        return true;
    }
/*
private:
    template <typename SegIter, typename Strategy>
    static inline bool check_segments_range(SegIter first,
                                            SegIter last,
                                            Box const& box,
                                            Strategy const& strategy)
    {
        for ( ; first != last ; ++first)
        {
            if (! disjoint_segment_box::apply(*first, box, strategy))
            {
                return false;
            }
        }
        return true;
    }
*/
};


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Areal1, typename Areal2>
struct disjoint<Areal1, Areal2, 2, areal_tag, areal_tag, false>
    : detail::disjoint::areal_areal<Areal1, Areal2>
{};

template <typename Areal1, typename Areal2>
struct disjoint_with_info<Areal1, Areal2, 2, areal_tag, areal_tag, false>
    : detail::disjoint::areal_areal_with_info<Areal1, Areal2>
{};


template <typename Areal, typename Box>
struct disjoint<Areal, Box, 2, areal_tag, box_tag, false>
    : detail::disjoint::areal_box<Areal, Box>
{};

template <typename Areal, typename Box>
struct disjoint_with_info<Areal, Box, 2, areal_tag, box_tag, false>
    : detail::disjoint::areal_box_with_info<Areal, Box>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_AREAL_AREAL_HPP
