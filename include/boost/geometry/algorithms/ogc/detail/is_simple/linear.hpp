// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_OGC_DETAIL_IS_SIMPLE_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OGC_DETAIL_IS_SIMPLE_LINEAR_HPP

#include <algorithm>
#include <deque>

#include <boost/range.hpp>

#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/policies/robustness/segment_ratio.hpp>

#include <boost/geometry/algorithms/ogc/is_valid.hpp>
#include <boost/geometry/algorithms/ogc/detail/is_simple/has_duplicates.hpp>

#include <boost/geometry/algorithms/ogc/dispatch/is_simple.hpp>

#include <boost/geometry/multi/multi.hpp>
#include <boost/geometry/multi/algorithms/detail/sections/sectionalize.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>

#include <boost/geometry/io/dsv/write.hpp>

namespace boost { namespace geometry { namespace ogc
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_simple
{


template <typename Range>
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

        if ( has_duplicates<Range>::apply(range) )
        {
            return false;
        }

        return !geometry::intersects(range);
    }
};



template <typename MultiRange>
struct is_simple_multi_range
{
    static inline bool apply(MultiRange const& multi_range)
    {
        typedef typename boost::range_value<MultiRange>::type range;
        typedef typename boost::range_iterator
            <
                MultiRange const
            >::type iterator;

        if ( boost::size(multi_range) == 0 )
        {
            return false;
        }

        for (iterator it = boost::begin(multi_range);
             it != boost::end(multi_range); ++it)
        {
            if ( !is_simple_range<range>::apply(*it) )
            {
                return false;
            }
        }
        return true;
    }
};



template <typename MultiLinestring>
struct is_simple_multilinestring
{
    template <typename Turn, typename Method, typename Operation>
    static inline bool check_turn(Turn const& turn,
                                  Method method,
                                  Operation operation)
    {
        return turn.method == method
            && turn.operations[0].operation == operation
            && turn.operations[1].operation == operation;
    }

    template <typename Turn>
    static inline bool is_acceptable_turn(Turn const& turn)
    {
        return check_turn(turn,
                          geometry::detail::overlay::method_none,
                          geometry::detail::overlay::operation_continue)
            || check_turn(turn,
                          geometry::detail::overlay::method_touch,
                          geometry::detail::overlay::operation_intersection)
            || check_turn(turn,
                          geometry::detail::overlay::method_touch,
                          geometry::detail::overlay::operation_blocked)
            ;
    }

    template <typename Point, typename Linestring>
    static inline bool is_endpoint_of(Point const& point,
                                      Linestring const& linestring)
    {
        BOOST_ASSERT( boost::size(linestring) > 1 );
        return geometry::equals(point, *boost::begin(linestring))
            || geometry::equals(point, *boost::rbegin(linestring));
    }

    template <typename Linestring1, typename Linestring2>
    static inline bool have_same_endpoints(Linestring1 const& ls1,
                                           Linestring2 const& ls2)
    {
        return
            (geometry::equals(*boost::begin(ls1), *boost::begin(ls2))
             && geometry::equals(*boost::rbegin(ls1), *boost::rbegin(ls2)))
            ||
            (geometry::equals(*boost::begin(ls1), *boost::rbegin(ls2))
             && geometry::equals(*boost::rbegin(ls1), *boost::begin(ls2)))
            ;
    }




    static inline bool apply(MultiLinestring const& multilinestring)
    {
        if ( !is_simple_multi_range<MultiLinestring>::apply(multilinestring) )
        {
            return false;
        }

        typedef typename boost::range_value<MultiLinestring>::type linestring;
        typedef typename point_type<MultiLinestring>::type point_type;
        typedef point_type point;

        typedef geometry::detail::overlay::turn_info
            <
                point_type,
                geometry::segment_ratio
                <
                    typename geometry::coordinate_type<point>::type
                >
            > turn_info;

        std::deque<turn_info> turns;

        typedef geometry::detail::overlay::get_turn_info
            <
                geometry::detail::disjoint::assign_disjoint_policy
            > turn_policy;

        geometry::detail::self_get_turn_points::no_interrupt_policy
            interrupt_policy;

        geometry::detail::self_get_turn_points::get_turns
            <
                turn_policy
            >::apply(multilinestring,
                     geometry::detail::no_rescale_policy(),
                     turns,
                     interrupt_policy);

        std::cout << "turns:";
        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {
            std::cout << " [" << geometry::method_char(tit->method);
            std::cout << "," << geometry::operation_char(tit->operations[0].operation);
            std::cout << "/" << geometry::operation_char(tit->operations[1].operation);
            std::cout << " " << geometry::dsv(tit->point);
            std::cout << "] ";
        }
        std::cout << std::endl << std::endl;

        if ( turns.size() == 0 )
        {
            return true;
        }

#ifdef GEOMETRY_TEST_DEBUG
        std::vector<point_type> endpoints;
        typename boost::range_iterator<MultiLinestring const>::type it;
        for (it = boost::begin(multilinestring);
             it != boost::end(multilinestring); ++it)
        {
            BOOST_ASSERT ( boost::size(*it) != 1 );
            if ( boost::size(*it) != 0 )
            {
                endpoints.push_back( *boost::begin(*it) );
                endpoints.push_back( *(--boost::end(*it)) );
            }
        }


        std::sort(endpoints.begin(), endpoints.end(), geometry::less<point>());

        std::cout << "endpoints: ";
        for (typename std::vector<point>::iterator pit = endpoints.begin();
             pit != endpoints.end(); ++pit)
        {
            std::cout << " " << geometry::dsv(*pit);
        }
        std::cout << std::endl << std::endl;
#endif

        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {            
#if 0
            if ( !is_acceptable_turn(*tit) )
            {
                return false;
            }
#endif
            linestring const& ls1 =
                *(boost::begin(multilinestring)
                  + tit->operations[0].seg_id.multi_index);
            linestring const& ls2 =
                *(boost::begin(multilinestring)
                  + tit->operations[0].other_id.multi_index);

            if ( !is_endpoint_of(tit->point, ls1)
                 || !is_endpoint_of(tit->point, ls2) )
            {
                return false;
            }

#if 1
            if ( boost::size(ls1) == 2
                 && boost::size(ls2) == 2
                 && have_same_endpoints(ls1, ls2) )
            {
                return false;
            }
#endif
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


}}} // namespace boost::geometry::ogc


#endif // BOOST_GEOMETRY_ALGORITHMS_OGC_DETAIL_IS_SIMPLE_LINEAR_HPP
