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
#include <boost/geometry/algorithms/detail/distance/iterator_selector.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace distance
{



template
<
    typename Geometry,
    typename Strategy
>
class point_or_segment_range_to_geometry_rtree
{
private:
    typedef typename strategy::distance::services::comparable_type
        <
            Strategy
        >::type comparable_strategy;

public:
    template <typename PointOrSegmentIterator>
    static inline
    typename strategy::distance::services::return_type
        <
            Strategy,
            typename point_type
                <
                    typename std::iterator_traits
                        <
                            PointOrSegmentIterator
                        >::value_type
                >::type,
            typename point_type<Geometry>::type
        >::type
    apply(PointOrSegmentIterator first,
          PointOrSegmentIterator beyond,
          Geometry const& geometry,
          Strategy const& strategy)
    {
        typedef iterator_selector<Geometry const> selector_type;

        typedef detail::closest_feature::range_to_range_rtree
            <
                PointOrSegmentIterator,
                typename selector_type::iterator_type
            > closest_features_type;

        BOOST_ASSERT( first != beyond );

        if ( geometry::has_one_element(first, beyond) )
        {
            return dispatch::distance
                <
                    typename std::iterator_traits
                        <
                            PointOrSegmentIterator
                        >::value_type,
                    Geometry,
                    Strategy
                >::apply(*first, geometry, strategy);
        }

        comparable_strategy cstrategy
            = strategy::distance::services::get_comparable
                <
                    Strategy
                >::apply(strategy);

        typename closest_features_type::return_type cf
            = closest_features_type::apply(first,
                                           beyond,
                                           selector_type::begin(geometry),
                                           selector_type::end(geometry),
                                           cstrategy);

        
        return dispatch::distance
            <
                typename closest_features_type::return_type::first_type,
                typename std::iterator_traits
                    <
                        typename closest_features_type::return_type::second_type
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

        typedef detail::closest_feature::range_to_range_rtree
            <
                const_point_iterator1,
                geometry::segment_iterator<Geometry2 const>
            > closest_features_type12;

        typedef typename closest_features_type12::return_type return_type12;

        typedef detail::closest_feature::range_to_range_rtree
            <
                const_point_iterator2,
                geometry::segment_iterator<Geometry1 const>
            > closest_features_type21;

        typedef typename closest_features_type21::return_type return_type21;

        const_point_iterator1 first1 = points_begin(geometry1);
        const_point_iterator1 beyond1 = points_end(geometry1);
        const_point_iterator2 first2 = points_begin(geometry2);
        const_point_iterator2 beyond2 = points_end(geometry2);
        
        if ( geometry::has_one_element(first1, beyond1) )
        {
            return dispatch::distance
                <
                    typename point_type<Geometry1>::type,
                    Geometry2,
                    Strategy
                >::apply(*first1, geometry2, strategy);
        }

        if ( geometry::has_one_element(first2, beyond2) )
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

        return_type12 cf12
            = closest_features_type12::apply(first1,
                                             beyond1,
                                             segments_begin(geometry2),
                                             segments_end(geometry2),
                                             cstrategy);

        return_type21 cf21
            = closest_features_type21::apply(first2,
                                             beyond2,
                                             segments_begin(geometry1),
                                             segments_end(geometry1),
                                             cstrategy);

        return_type dist1 = dispatch::distance
            <
                typename point_type<Geometry1>::type,
                typename std::iterator_traits
                    <
                        typename return_type12::second_type
                    >::value_type,
                Strategy
            >::apply(cf12.first, *cf12.second, strategy);

        return_type dist2 = dispatch::distance
            <
                typename point_type<Geometry2>::type,
                typename std::iterator_traits
                    <
                        typename return_type21::second_type
                    >::value_type,                
                Strategy
            >::apply(cf21.first, *cf21.second, strategy);

        return (std::min)(dist1, dist2);
    }
};



}} // namespace detail::distance
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_GEOMETRY_TO_GEOMETRY_RTREE_HPP
