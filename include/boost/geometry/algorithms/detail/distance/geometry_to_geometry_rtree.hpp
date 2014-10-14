// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_GEOMETRY_TO_GEOMETRY_RTREE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_GEOMETRY_TO_GEOMETRY_RTREE_HPP

#include <cstddef>
#include <algorithm>
#include <iterator>
#include <utility>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/core/ignore_unused.hpp>

#include <boost/geometry/iterators/point_iterator.hpp>
#include <boost/geometry/iterators/segment_iterator.hpp>
#include <boost/geometry/iterators/has_one_element.hpp>

#include <boost/geometry/algorithms/intersects.hpp>

#include <boost/geometry/strategies/distance.hpp>

#include <boost/geometry/algorithms/dispatch/distance.hpp>

#include <boost/geometry/algorithms/detail/closest_feature/range_to_range.hpp>
#include <boost/geometry/algorithms/detail/distance/is_comparable.hpp>
#include <boost/geometry/algorithms/detail/distance/iterator_selector.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace distance
{



template
<
    typename PointOrSegmentIterator,
    typename Geometry,
    typename Strategy
>
class point_or_segment_range_to_geometry_rtree
{
private:
    typedef typename std::iterator_traits
        <
            PointOrSegmentIterator
        >::value_type point_or_segment_type;

    typedef iterator_selector<Geometry const> selector_type;

    typedef detail::closest_feature::range_to_range_rtree range_to_range;

public:
    typedef typename strategy::distance::services::return_type
        <
            Strategy,
            typename point_type<point_or_segment_type>::type,
            typename point_type<Geometry>::type
        >::type return_type;

    static inline return_type apply(PointOrSegmentIterator first,
                                    PointOrSegmentIterator last,
                                    Geometry const& geometry,
                                    Strategy const& strategy)
    {
        namespace sds = strategy::distance::services;

        BOOST_ASSERT( first != last );

        if ( geometry::has_one_element(first, last) )
        {
            return dispatch::distance
                <
                    point_or_segment_type, Geometry, Strategy
                >::apply(*first, geometry, strategy);
        }

        point_or_segment_type rtree_value_min;
        typename selector_type::iterator_type qit_min;
        typename sds::return_type
            <
                typename sds::comparable_type<Strategy>::type,
                typename point_type<point_or_segment_type>::type,
                typename point_type<Geometry>::type
            >::type cd_min;

        std::pair
            <
                point_or_segment_type,
                typename selector_type::iterator_type
            > cf = range_to_range::apply(first,
                                         last,
                                         selector_type::begin(geometry),
                                         selector_type::end(geometry),
                                         sds::get_comparable
                                             <
                                                 Strategy
                                             >::apply(strategy),
                                         cd_min);

        return
            is_comparable<Strategy>::value
            ?
            cd_min
            :
            dispatch::distance
                <
                    point_or_segment_type,                    
                    typename std::iterator_traits
                        <
                            typename selector_type::iterator_type
                        >::value_type,
                    Strategy
                >::apply(cf.first, *cf.second, strategy);
    }
};



template
<
    typename Geometry1,
    typename Geometry2,
    typename Strategy
>
class geometry_to_geometry_rtree
{
    // the following works with linear geometries seen as ranges of points
    //
    // we compute the r-tree for the points of one range and then,
    // compute nearest points for the segments of the other,
    // ... and ...
    // vice versa.

private:
    typedef typename strategy::distance::services::comparable_type
        <
            Strategy
        >::type comparable_strategy;

    typedef typename strategy::distance::services::return_type
       <
           comparable_strategy,
           typename point_type<Geometry1>::type,
           typename point_type<Geometry2>::type
       >::type comparable_return_type;

    typedef detail::closest_feature::range_to_range_rtree range_to_range;

public:
    typedef typename strategy::distance::services::return_type
        <
            Strategy,
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type
        >::type return_type;   

    static inline return_type apply(Geometry1 const& geometry1,
                                    Geometry2 const& geometry2,
                                    Strategy const& strategy,
                                    bool check_intersection = true)
    {
        typedef geometry::point_iterator<Geometry1 const> const_point_iterator1;
        typedef geometry::point_iterator<Geometry2 const> const_point_iterator2;

        const_point_iterator1 first1 = points_begin(geometry1);
        const_point_iterator1 last1 = points_end(geometry1);
        const_point_iterator2 first2 = points_begin(geometry2);
        const_point_iterator2 last2 = points_end(geometry2);
        
        if ( geometry::has_one_element(first1, last1) )
        {
            return dispatch::distance
                <
                    typename point_type<Geometry1>::type,
                    Geometry2,
                    Strategy
                >::apply(*first1, geometry2, strategy);
        }

        if ( geometry::has_one_element(first2, last2) )
        {
            return dispatch::distance
                <
                    typename point_type<Geometry2>::type,
                    Geometry1,
                    Strategy
                >::apply(*first2, geometry1, strategy);
        }

        if ( check_intersection && geometry::intersects(geometry1, geometry2) )
        {
            return return_type(0);
        }

        comparable_strategy cstrategy =
            strategy::distance::services::get_comparable
                <
                    Strategy
                >::apply(strategy);

        comparable_return_type cdist12;
        std::pair
            <
                typename point_type<Geometry1>::type,
                geometry::segment_iterator<Geometry2 const>
            > cf12 = range_to_range::apply(first1,
                                           last1,
                                           segments_begin(geometry2),
                                           segments_end(geometry2),
                                           cstrategy,
                                           cdist12);

        comparable_return_type cdist21;
        std::pair
            <
                typename point_type<Geometry2>::type,
                geometry::segment_iterator<Geometry1 const>
            > cf21 = range_to_range::apply(first2,
                                           last2,
                                           segments_begin(geometry1),
                                           segments_end(geometry1),
                                           cstrategy,
                                           cdist21);

        if ( is_comparable<Strategy>::value )
        {
            return (std::min)(cdist12, cdist21);
        }

        if ( cdist12 < cdist21 )
        {
            return dispatch::distance
                <
                    typename point_type<Geometry1>::type,
                    typename std::iterator_traits
                        <
                            geometry::segment_iterator<Geometry2 const>
                        >::value_type,
                    Strategy
                >::apply(cf12.first, *cf12.second, strategy);
        }

        return dispatch::distance
            <
                typename point_type<Geometry2>::type,
                typename std::iterator_traits
                    <
                        geometry::segment_iterator<Geometry1 const>
                    >::value_type,                
                Strategy
            >::apply(cf21.first, *cf21.second, strategy);
    }
};



}} // namespace detail::distance
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_GEOMETRY_TO_GEOMETRY_RTREE_HPP
