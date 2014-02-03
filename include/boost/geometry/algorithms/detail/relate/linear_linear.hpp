// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP

#include <boost/geometry/algorithms/detail/relate/result.hpp>
#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/turns.hpp>
#include <boost/geometry/algorithms/detail/sub_geometry.hpp>

#include <boost/geometry/algorithms/detail/range_helpers.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

// currently works only for linestrings
template <typename Geometry1, typename Geometry2>
struct linear_linear
{
    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;

    static inline result apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        // TODO: currently this only works for linestrings
        std::size_t s1 = boost::size(geometry1);
        std::size_t s2 = boost::size(geometry2);
        if ( s1 == 0 || s2 == 0 )
        {
            // throw on empty output?
            return result("FFFFFFFFF");
        }
        else if ( s1 == 1 && s2 == 1 )
        {
            return point_point<point1_type, point2_type>::apply(range::front(geometry1), range::front(geometry2));
        }
        else if ( s1 == 1 /*&& s2 > 1*/ )
        {
            return point_geometry<point1_type, Geometry2>::apply(range::front(geometry1), geometry2);
        }
        else if ( s2 == 1 /*&& s1 > 1*/  )
        {
            return geometry_point<Geometry2, point2_type>::apply(geometry1, range::front(geometry2));
        }

        // TODO: handle also linestrings with points_num == 2 and equals(front, back) - treat like point?

        result res("FFFFFFFFT");
        static const std::size_t dimension = geometry::dimension<Geometry1>::value;
        if ( dimension < 10 )
            res.template set<exterior, exterior, '0' + dimension>();

        // TODO: implement generic function working also for multilinestrings, also use it in point_in_geometry
        bool has_boundary1 = ! detail::equals::equals_point_point(range::front(geometry1), range::back(geometry1));
        bool has_boundary2 = ! detail::equals::equals_point_point(range::front(geometry2), range::back(geometry2));

        handle_boundaries(res, geometry1, geometry2, has_boundary1, has_boundary2);

        // get and analyse turns

        std::deque<typename turns::get_turns<Geometry1, Geometry2>::turn_info> turns;

        turns::get_turns<Geometry1, Geometry2>::apply(turns, geometry1, geometry2);

        // TODO: turns must be sorted and followed only if it's possible to go out and in on the same point
        // for linear geometries union operation must be detected which I guess would be quite often
        std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<>());

        analyse_turns(res, turns.begin(), turns.end(), geometry1, geometry2, has_boundary1, has_boundary2);

        return res;
    }

    template <typename TurnIt>
    static inline void analyse_turns(result & res,
                                     TurnIt first, TurnIt last,
                                     Geometry1 const& geometry1, Geometry2 const& geometry2,
                                     bool has_boundary1, bool has_boundary2)
    {
        for ( TurnIt it = first ; it != last ; ++it )
        {
            // 'i'
            if ( it->method == overlay::method_crosses )
            {
                res.template update<interior, interior, '0'>();
                res.template update<interior, exterior, '1'>();
                res.template update<exterior, interior, '1'>();
            }
            // 'e' 'c'
            else if ( it->method == overlay::method_equal
                   || it->method == overlay::method_collinear )
            {
                res.template update<interior, interior, '1'>();
            }
            // 't' 'm'
            else if ( it->method == overlay::method_touch
                   || it->method == overlay::method_touch_interior )
            {
                bool b = handle_boundary_point(res, *it, geometry1, geometry2, has_boundary1, has_boundary2);
                
                if ( it->has(overlay::operation_union) )
                {
                    if ( !b )
                        res.template update<interior, interior, '0'>();
                    if ( it->operations[0].operation == overlay::operation_union )
                        res.template update<interior, exterior, '1'>();
                    if ( it->operations[1].operation == overlay::operation_union )
                        res.template update<exterior, interior, '1'>();
                }

                if ( it->has(overlay::operation_intersection) )
                    res.template update<interior, interior, '1'>();

                if ( it->has(overlay::operation_blocked) )
                    if ( !b )
                        res.template update<interior, interior, '0'>();
            }
            
        }
    }

    template <typename Turn>
    static inline bool handle_boundary_point(result & res,
                                             Turn const& turn,
                                             Geometry1 const& geometry1, Geometry2 const& geometry2,
                                             bool has_boundary1, bool has_boundary2)
    {
        bool pt_on_boundary1 = has_boundary1 && equals_terminal_point(turn.point, geometry1);
        bool pt_on_boundary2 = has_boundary2 && equals_terminal_point(turn.point, geometry2);

        if ( pt_on_boundary1 && pt_on_boundary2 )
            res.template update<boundary, boundary, '0'>();
        else if ( pt_on_boundary1 )
            res.template update<boundary, interior, '0'>();
        else if ( pt_on_boundary2 )
            res.template update<interior, boundary, '0'>();
        else
            return false;
        return true;
    }

    // TODO: replace with generic point_in_boundary working also for multilinestrings
    template <typename Point, typename Geometry>
    static inline bool equals_terminal_point(Point const& point, Geometry const& geometry)
    {
        return detail::equals::equals_point_point(point, front(geometry))
            || detail::equals::equals_point_point(point, back(geometry));
    }

    static inline void handle_boundaries(result & res,
                                         Geometry1 const& geometry1, Geometry2 const& geometry2,
                                         bool has_boundary1, bool has_boundary2)
    {
        int pig_front = detail::within::point_in_geometry(front(geometry1), geometry2);

        if ( has_boundary1 )
        {
            int pig_back = detail::within::point_in_geometry(back(geometry1), geometry2);

            if ( pig_front > 0 || pig_back > 0 )
                res.template set<boundary, interior, '0'>();
            if ( pig_front == 0 || pig_back == 0 )
                res.template set<boundary, boundary, '0'>();
            if ( pig_front < 0 || pig_back < 0 )
            {
                res.template set<boundary, exterior, '0'>();
                res.template set<interior, exterior, '1'>();
            }
        }
        else
        {
            if ( pig_front > 0 )
                res.template set<interior, interior, '0'>();
            else if ( pig_front == 0 )
                res.template set<interior, boundary, '0'>();
            else if ( pig_front < 0 )
                res.template set<interior, exterior, '0'>();
        }

        pig_front = detail::within::point_in_geometry(front(geometry2), geometry1);

        if ( has_boundary2 )
        {
            int pig_back = detail::within::point_in_geometry(back(geometry2), geometry1);

            if ( pig_front > 0 || pig_back > 0 )
                res.template set<interior, boundary, '0'>();
            if ( pig_front == 0 || pig_back == 0 )
                res.template set<boundary, boundary, '0'>();
            if ( pig_front < 0 || pig_back < 0 )
            {
                res.template set<exterior, boundary, '0'>();
                res.template set<exterior, interior, '1'>();
            }
        }
        else
        {
            if ( pig_front > 0 )
                res.template set<interior, interior, '0'>();
            else if ( pig_front == 0 )
                res.template set<boundary, interior, '0'>();
            else if ( pig_front < 0 )
                res.template set<exterior, interior, '0'>();
        }
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
