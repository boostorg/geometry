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

template <field F1, field F2, char D, bool Reverse>
struct update_result_dispatch
{
    static inline void apply(result & res)
    {
        res.template update<F1, F2, D>();
    }
};

template <field F1, field F2, char D>
struct update_result_dispatch<F1, F2, D, true>
{
    static inline void apply(result & res)
    {
        res.template update<F2, F1, D>();
    }
};

template <field F1, field F2, char D, bool Reverse>
inline void update_result(result & res)
{
    update_result_dispatch<F1, F2, D, Reverse>::apply(res);
}

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

        if ( turns.empty() )
        {
            return res;
        }

        // TODO: turns must be sorted and followed only if it's possible to go out and in on the same point
        // for linear geometries union operation must be detected which I guess would be quite often
        std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,1,2,4,3,0,0>());

        analyse_turns<0, 1>(res, turns.begin(), turns.end(), geometry1, geometry2, has_boundary1, has_boundary2);

        std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,1,2,4,3,0,1>());

        analyse_turns<1, 0>(res, turns.begin(), turns.end(), geometry2, geometry1, has_boundary2, has_boundary1);

        return res;
    }

    template <typename Point>
    class point_identifier
    {
    public:
        point_identifier() : sid_ptr(0), pt_ptr(0) {}
        point_identifier(segment_identifier const& sid, Point const& pt)
            : sid_ptr(boost::addressof(sid))
            , pt_ptr(boost::addressof(pt))
        {}
        segment_identifier const& seg_id() const
        {
            BOOST_ASSERT(sid_ptr);
            return *sid_ptr;
        }
        Point const& point() const
        {
            BOOST_ASSERT(pt_ptr);
            return *pt_ptr;
        }

        //friend bool operator==(point_identifier const& l, point_identifier const& r)
        //{
        //    return l.seg_id() == r.seg_id()
        //        && detail::equals::equals_point_point(l.point(), r.point());
        //}

    private:
        const segment_identifier * sid_ptr;
        const Point * pt_ptr;
    };

    class same_ranges
    {
    public:
        same_ranges(segment_identifier const& sid)
            : sid_ptr(boost::addressof(sid))
        {}

        template <typename Point>
        bool operator()(point_identifier<Point> const& pid)
        {
            return pid.seg_id().multi_index == sid_ptr->multi_index
                && pid.seg_id().ring_index == sid_ptr->ring_index;                
        }
    private:
        const segment_identifier * sid_ptr;
    };

    template <std::size_t OpId,
              std::size_t OtherOpId,
              typename TurnIt,
              typename Geometry,
              typename OtherGeometry>
    static inline void analyse_turns(result & res,
                                     TurnIt first, TurnIt last,
                                     Geometry const& geometry,
                                     OtherGeometry const& other_geometry,
                                     bool has_boundary,
                                     bool other_has_boundary)
    {
        // should be the same as the one stored in Turn
        typedef typename point_type<Geometry1>::type point_type;
        typedef point_identifier<point_type> point_identifier;
        static const bool reverse_result = OpId != 0;

        if ( first == last )
            return;

        typedef typename std::vector<point_identifier>::iterator point_iterator;
        std::vector<point_identifier> entry_points; // TODO: use map here or sorted vector?
        
        bool possible_exit_detected = false;
        point_identifier possible_exit_point;

        for ( TurnIt it = first ; it != last ; ++it )
        {
            overlay::operation_type op = it->operations[OpId].operation;
            
            // handle possible exit
            if ( possible_exit_detected && 
                 ( op == overlay::operation_union || op == overlay::operation_intersection
                || op == overlay::operation_continue || op == overlay::operation_blocked ) )
            {
                // real exit point - may be multiple
                // we know that we entered and now we exit
                if ( !detail::equals::equals_point_point(it->point, possible_exit_point.point()) )
                {
                    possible_exit_detected = false;
                    
                    // not the last IP
                    update_result<interior, exterior, '1', reverse_result>(res);
                }
                // fake exit point, reset state
                // in reality this will be op == overlay::operation_intersection
                else if ( op != overlay::operation_union )
                {
                    possible_exit_detected = false;
                }
            }

            if ( op == overlay::operation_intersection )
            {
                // here we know that we entered the range
                entry_points.push_back(point_identifier(it->operations[OpId].seg_id, it->point));

                update_result<interior, interior, '1', reverse_result>(res);
                // if the first point of P and/or Q is boundary then also
                //update_result<boundary, interior, '0', reverse_result>(res);
                // or
                //update_result<boundary, boundary, '0', reverse_result>(res);
            }
            else if ( op == overlay::operation_blocked
                   || op == overlay::operation_union )
            {
                if ( !entry_points.empty() )
                {
                    segment_identifier const& seg_id = it->operations[OpId].seg_id;
                    point_iterator entry_it = std::find_if(entry_points.begin(),
                                                           entry_points.end(),
                                                           same_ranges(seg_id));
                    if ( entry_it != entry_points.end() )
                    {
                        if ( op == overlay::operation_union )
                        {
                            // here we know that we possibly left LS
                            // we must still check if we didn't get back on the same point
                            possible_exit_detected = true;
                            possible_exit_point = point_identifier(seg_id, it->point);
                        }
                        else // op == overlay::operation_blocked
                        {
                            // here we know that we're on the last point of the range

                            // depending on the other operation
                            //update_result<boundary, interior, '0', reverse_result>(res);
                            // or
                            //update_result<boundary, boundary, '0', reverse_result>(res);
                        }

                        // erase the corresponding entry point,
                        // don't postpone the erasure decision because
                        // there may be multiple exit IPs on the same point
                        // and we'd be forced to store them all just like the entry points
                        entry_points.erase(entry_it);
                    }
                }
                else
                {
                    update_result<interior, exterior, '1', reverse_result>(res);

                    if ( it->method == overlay::method_crosses )
                        update_result<interior, interior, '0', reverse_result>(res);

                    // or depending on the IP
                    //update_result<boundary, interior, '0', reverse_result>(res);
                    // or
                    //update_result<boundary, boundary, '0', reverse_result>(res);
                }
            }
            else if ( op == overlay::operation_continue )
            {
                update_result<interior, interior, '1', reverse_result>(res);
            }
        }

        // here, the possible exit is the real one
        // we know that we entered and now we exit
        if ( possible_exit_detected )
        {
            update_result<interior, exterior, '1', reverse_result>(res);
            // if has boundary on the last point of the current range
            //update_result<boundary, exterior, '0', reverse_result>(res);
        }
    }
    
    template <typename TurnIt>
    static inline void analyse_turns_simple(result & res,
                                            TurnIt first, TurnIt last,
                                            Geometry1 const& geometry1,
                                            Geometry2 const& geometry2,
                                            bool has_boundary1, bool has_boundary2)
    {
        for ( TurnIt it = first ; it != last ; ++it )
        {
            // 'i'
            if ( it->method == overlay::method_crosses )
            {
                res.template update<interior, interior, '0'>(); // always true
                res.template update<interior, exterior, '1'>(); // not always true
                res.template update<exterior, interior, '1'>(); // not always true
            }
            // 'e' 'c'
            else if ( it->method == overlay::method_equal
                   || it->method == overlay::method_collinear )
            {
                res.template update<interior, interior, '1'>(); // always true
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
        return detail::equals::equals_point_point(point, range::front(geometry))
            || detail::equals::equals_point_point(point, range::back(geometry));
    }

    static inline void handle_boundaries(result & res,
                                         Geometry1 const& geometry1, Geometry2 const& geometry2,
                                         bool has_boundary1, bool has_boundary2)
    {
        int pig_front = detail::within::point_in_geometry(range::front(geometry1), geometry2);

        if ( has_boundary1 )
        {
            int pig_back = detail::within::point_in_geometry(range::back(geometry1), geometry2);

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

        pig_front = detail::within::point_in_geometry(range::front(geometry2), geometry1);

        if ( has_boundary2 )
        {
            int pig_back = detail::within::point_in_geometry(range::back(geometry2), geometry1);

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
