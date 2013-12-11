// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_TOUCHES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_TOUCHES_HPP


#include <deque>

#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/num_geometries.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace touches
{

struct generic_interrupt_policy
{
    static bool const enabled = true;
    bool result;

    // dummy variable required by self_get_turn_points::get_turns
    static bool const has_intersections = false;

    inline generic_interrupt_policy()
        : result(false)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        typedef typename boost::range_iterator<Range const>::type iterator;
        for ( iterator it = boost::begin(range) ; it != boost::end(range) ; ++it )
        {
            if ( it->has(overlay::operation_intersection) )
            {
                result = false;
                return true;
            }

            switch(it->method)
            {
                case overlay::method_crosses:
                    result = false;
                    return true;
                case overlay::method_equal:
                    // Segment spatially equal means: at the right side
                    // the polygon internally overlaps. So return false.
                    result = false;
                    return true;
                case overlay::method_touch:
                case overlay::method_touch_interior:
                case overlay::method_collinear:
                    if ( ok_for_touch(*it) )
                    {
                        result = true;
                    }
                    else
                    {
                        result = false;
                        return true;
                    }
                    break;
                case overlay::method_none :
                case overlay::method_disjoint :
                case overlay::method_error :
                    break;
            }
        }

        return false;
    }

    template <typename Turn>
    inline bool ok_for_touch(Turn const& turn)
    {
        return turn.both(overlay::operation_union)
            || turn.both(overlay::operation_blocked)
            || turn.combination(overlay::operation_union, overlay::operation_blocked)
            ;
    }
};

template <std::size_t Max>
struct turns_count_interrupt_policy
{
    static bool const enabled = true;
    std::size_t turns_count;

    // dummy variable required by self_get_turn_points::get_turns
    static bool const has_intersections = false;

    inline turns_count_interrupt_policy()
        : turns_count(0)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        turns_count += boost::size(range);
        if ( Max < turns_count )
            return true;
        return false;
    }
};


template<typename Geometry>
struct check_each_ring_for_within
{
    bool has_within;
    Geometry const& m_geometry;

    inline check_each_ring_for_within(Geometry const& g)
        : has_within(false)
        , m_geometry(g)
    {}

    template <typename Range>
    inline void apply(Range const& range)
    {
        typename geometry::point_type<Range>::type p;
        geometry::point_on_border(p, range);
        if (geometry::within(p, m_geometry))
        {
            has_within = true;
        }
    }
};


template <typename FirstGeometry, typename SecondGeometry>
inline bool rings_containing(FirstGeometry const& geometry1,
                SecondGeometry const& geometry2)
{
    check_each_ring_for_within<FirstGeometry> checker(geometry1);
    geometry::detail::for_each_range(geometry2, checker);
    return checker.has_within;
}

template <typename Geometry1, typename Geometry2>
struct generic_touches
{

    static inline
    bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        typedef detail::overlay::turn_info
            <
                typename geometry::point_type<Geometry1>::type
            > turn_info;

        typedef detail::overlay::get_turn_info
            <
                detail::overlay::assign_null_policy
            > policy_type;

        std::deque<turn_info> turns;
        detail::touches::generic_interrupt_policy policy;
        boost::geometry::get_turns
                <
                    detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value,
                    detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value,
                    detail::overlay::assign_null_policy
                >(geometry1, geometry2, detail::no_rescale_policy(), turns, policy);

        return policy.result
            && ! geometry::detail::touches::rings_containing(geometry1, geometry2)
            && ! geometry::detail::touches::rings_containing(geometry2, geometry1)
            ;
    }
};

template <typename Point, typename Geometry>
struct point_geometry
{
    static inline
    bool apply(Point const& point, Geometry const& geometry)
    {
        return detail::within::point_in_geometry(point, geometry) == 0;
    }
};

template <typename Geometry1, typename Geometry2>
struct linestring_linestring
{
    static inline
    bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        std::size_t s1 = boost::size(geometry1);
        std::size_t s2 = boost::size(geometry2);
        // TODO: throw on empty input?
        if ( s1 == 0 || s2 == 0 )
            return false;

        typedef detail::overlay::turn_info
            <
                typename geometry::point_type<Geometry1>::type
            > turn_info;

        typedef detail::overlay::get_turn_info
            <
                detail::overlay::assign_null_policy
            > policy_type;

        std::deque<turn_info> turns;
        turns_count_interrupt_policy<2> policy;
        boost::geometry::get_turns
                <
                    detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value,
                    detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value,
                    detail::overlay::assign_null_policy
                >(geometry1, geometry2, detail::no_rescale_policy(), turns, policy);

        if ( 2 < policy.turns_count )
            return false;
        else if ( 2 == policy.turns_count )
        {
            return ( detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry1)-1))
                  && detail::equals::equals_point_point(turns[1].point, *(boost::end(geometry2)-1)) )
                || ( detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry2)-1))
                  && detail::equals::equals_point_point(turns[1].point, *(boost::end(geometry1)-1)) );
        }
        else if ( 1 == policy.turns_count )
        {
            return detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry1)-1))
                || detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry2)-1));
        }
        else
        {
            return detail::within::point_in_geometry(*boost::begin(geometry1), geometry2) >= 0
                || detail::within::point_in_geometry(*boost::begin(geometry2), geometry1) >= 0;
        }
    }
};

}}
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

template
<
    typename Geometry1, typename Geometry2,
    typename Tag1 = typename tag<Geometry1>::type,
    typename Tag2 = typename tag<Geometry2>::type
>
struct touches
    : detail::touches::generic_touches<Geometry1, Geometry2>
{};

template <typename Point, typename Geometry, typename Tag2>
struct touches<Point, Geometry, point_tag, Tag2>
    : detail::touches::point_geometry<Point, Geometry>
{};

template <typename Linestring1, typename Linestring2>
struct touches<Linestring1, Linestring2, linestring_tag, linestring_tag>
    : detail::touches::linestring_linestring<Linestring1, Linestring2>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
\brief \brief_check{has at least one touching point (self-tangency)}
\note This function can be called for one geometry (self-tangency) and
    also for two geometries (touch)
\ingroup touches
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_check{is self-touching}

\qbk{distinguish,one geometry}
\qbk{[def __one_parameter__]}
\qbk{[include reference/algorithms/touches.qbk]}
*/
template <typename Geometry>
inline bool touches(Geometry const& geometry)
{
    concept::check<Geometry const>();

    typedef detail::overlay::turn_info
        <
            typename geometry::point_type<Geometry>::type
        > turn_info;

    typedef detail::overlay::get_turn_info
        <
            detail::overlay::assign_null_policy
        > policy_type;

    std::deque<turn_info> turns;
    detail::touches::generic_interrupt_policy policy;
    detail::self_get_turn_points::get_turns
            <
                policy_type
            >::apply(geometry, detail::no_rescale_policy(), turns, policy);

    return policy.result;
}


/*!
\brief \brief_check2{have at least one touching point (tangent - non overlapping)}
\ingroup touches
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return \return_check2{touch each other}

\qbk{distinguish,two geometries}
\qbk{[include reference/algorithms/touches.qbk]}
 */
template <typename Geometry1, typename Geometry2>
inline bool touches(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    return dispatch::touches<Geometry1, Geometry2>::apply(geometry1, geometry2);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_TOUCHES_HPP
