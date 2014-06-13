// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_SIMPLE_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_SIMPLE_LINEAR_HPP

#include <algorithm>
#include <deque>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/policies/robustness/no_rescale_policy.hpp>
#include <boost/geometry/policies/robustness/segment_ratio.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/detail/disjoint/linear_linear.hpp>
#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>
#include <boost/geometry/algorithms/detail/is_simple/has_duplicates.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>

#include <boost/geometry/algorithms/detail/is_simple/debug_linear.hpp>

#include <boost/geometry/algorithms/dispatch/is_simple.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_simple
{


template <typename Range, bool CheckSelfIntersections = true>
struct is_simple_range
{
    static inline bool apply(Range const& range)
    {
        // the second argument to apply refers to spikes:
        // spikes are disallowed
        if ( !dispatch::is_valid<Range>::apply(range, false) )
        {
            return false;
        }

        return !has_duplicates<Range>::apply(range)
            && !(CheckSelfIntersections && geometry::intersects(range));
    }
};



template <typename MultiLinestring>
struct is_simple_multilinestring
{
    template <typename Point, typename Linestring>
    static inline bool is_endpoint_of(Point const& point,
                                      Linestring const& linestring)
    {
        BOOST_ASSERT( boost::size(linestring) > 1 );
        return geometry::equals(point, range::front(linestring))
            || geometry::equals(point, range::back(linestring));
    }

    template <typename Linestring1, typename Linestring2>
    static inline bool have_same_endpoints(Linestring1 const& ls1,
                                           Linestring2 const& ls2)
    {
        return
            geometry::equals(range::front(ls1), range::front(ls2))
            ?
            geometry::equals(range::back(ls1), range::back(ls2))
            :
            (geometry::equals(range::front(ls1), range::back(ls2))
             &&
             geometry::equals(range::back(ls1), range::front(ls2))
             )
            ;
    }


    static inline bool apply(MultiLinestring const& multilinestring)
    {
        typedef typename boost::range_value<MultiLinestring>::type linestring;
        typedef typename point_type<MultiLinestring>::type point_type;
        typedef point_type point;


        // check each of the linestrings for simplicity
        if ( !detail::check_iterator_range
                 <
                     is_simple_range<linestring>
                 >::apply(boost::begin(multilinestring),
                          boost::end(multilinestring))
             )
        {
            return false;
        }


        // compute self turns
        typedef detail::overlay::turn_info
            <
                point_type,
                geometry::segment_ratio
                <
                    typename geometry::coordinate_type<point>::type
                >
            > turn_info;

        std::deque<turn_info> turns;

        typedef detail::overlay::get_turn_info
            <
                detail::disjoint::assign_disjoint_policy
            > turn_policy;

        detail::self_get_turn_points::no_interrupt_policy
            interrupt_policy;

        detail::self_get_turn_points::get_turns
            <
                turn_policy
            >::apply(multilinestring,
                     detail::no_rescale_policy(),
                     turns,
                     interrupt_policy);

        debug_print_turns(turns.begin(), turns.end());
        debug_print_endpoints(multilinestring);

        // check if the generated turns are all endpoints of the
        // linestrings in the multi-linestring
        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {            
            linestring const& ls1 =
                range::at(multilinestring,
                          tit->operations[0].seg_id.multi_index);

            linestring const& ls2 =
                range::at(multilinestring,
                          tit->operations[0].other_id.multi_index);

            if ( !is_endpoint_of(tit->point, ls1)
                 || !is_endpoint_of(tit->point, ls2) )
            {
                return false;
            }

            if ( boost::size(ls1) == 2
                 && boost::size(ls2) == 2
                 && have_same_endpoints(ls1, ls2) )
            {
                return false;
            }
        }

        return true;
    }

};



}} // namespace detail::is_simple
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// A linestring is a curve.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Linestring>
struct is_simple<Linestring, linestring_tag>
    : detail::is_simple::is_simple_range<Linestring>
{};


// A MultiLinestring is a MultiCurve
// A MultiCurve is simple if all of its elements are simple and the
// only intersections between any two elements occur at Points that
// are on the boundaries of both elements.
//
// Reference: OGC 06-103r4 (§6.1.8.1; Fig. 9)
template <typename MultiLinestring>
struct is_simple<MultiLinestring, multi_linestring_tag>
    : detail::is_simple::is_simple_multilinestring<MultiLinestring>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_SIMPLE_LINEAR_HPP
