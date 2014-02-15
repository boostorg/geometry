// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP

#include <boost/geometry/algorithms/detail/sub_geometry.hpp>
#include <boost/geometry/algorithms/detail/range_helpers.hpp>

#include <boost/geometry/algorithms/detail/relate/result.hpp>
#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/turns.hpp>
#include <boost/geometry/algorithms/detail/relate/boundary_checker.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

template <std::size_t OpId,
          typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type>
struct has_disjoint_sub_geometries {};

template <std::size_t OpId, typename Geometry>
struct has_disjoint_sub_geometries<OpId, Geometry, linestring_tag>
{
    template <typename TurnIt>
    static inline bool apply(TurnIt first, TurnIt last, Geometry const& /*geometry*/)
    {
        BOOST_ASSERT(first != last);
        return false;
    }
};

template <std::size_t OpId, typename Geometry>
struct has_disjoint_sub_geometries<OpId, Geometry, multi_linestring_tag>
{
    template <typename TurnIt>
    static inline bool apply(TurnIt first, TurnIt last, Geometry const& geometry)
    {
        BOOST_ASSERT(first != last);

        std::size_t count = boost::size(geometry);
        std::vector<bool> detected_intersections(count, false);
        for ( TurnIt it = first ; it != last ; ++it )
        {
            int multi_index = it->operations[OpId].seg_id.multi_index;
            BOOST_ASSERT(multi_index >= 0);
            std::size_t index = static_cast<std::size_t>(multi_index);
            BOOST_ASSERT(index < count);
            detected_intersections[index] = true;
        }

        for ( std::vector<bool>::iterator it = detected_intersections.begin() ;
              it != detected_intersections.end() ; ++it )
        {
            if ( *it == false )
                return true;
        }
        
        return false;
    }
};

// update_result

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
// TODO : not working for MultiLinestrings
//            return point_point<point1_type, point2_type>::apply(range::front(geometry1), range::front(geometry2));
            BOOST_ASSERT(false);
        }
        else if ( s1 == 1 /*&& s2 > 1*/ )
        {
// TODO : not working for MultiLinestrings
//            return point_geometry<point1_type, Geometry2>::apply(range::front(geometry1), geometry2);
            BOOST_ASSERT(false);
        }
        else if ( s2 == 1 /*&& s1 > 1*/  )
        {
// TODO : not working for MultiLinestrings
//            return geometry_point<Geometry2, point2_type>::apply(geometry1, range::front(geometry2));
            BOOST_ASSERT(false);
        }

        // TODO: handle also linestrings with points_num == 2 and equals(front, back) - treat like point?

        result res("FFFFFFFFT");
        static const std::size_t dimension = geometry::dimension<Geometry1>::value;
        if ( dimension < 10 )
            res.template set<exterior, exterior, '0' + dimension>();

        // TEMP
        //bool has_boundary1 = ! detail::equals::equals_point_point(range::front(geometry1), range::back(geometry1));
        //bool has_boundary2 = ! detail::equals::equals_point_point(range::front(geometry2), range::back(geometry2));
        //handle_boundaries(res, geometry1, geometry2, has_boundary1, has_boundary2);

        // get and analyse turns
        typedef typename turns::get_turns<Geometry1, Geometry2>::turn_info turn_type;
        typedef typename std::vector<turn_type>::iterator turn_iterator;
        std::vector<turn_type> turns;

        turns::get_turns<Geometry1, Geometry2>::apply(turns, geometry1, geometry2);

        if ( turns.empty() )
        {
            return res;
        }

// TODO: this works bout we don't know what should be set exactly
//       replace it with something like: for_each_disjoint_subgeometry
        if ( has_disjoint_sub_geometries<0, Geometry1>::apply(turns.begin(), turns.end(), geometry1) )
        {
// TODO:
// check if there is an interior and/or boundary
            res.template update<interior, exterior, '1'>();
            res.template update<boundary, exterior, '0'>();
        }

        if ( has_disjoint_sub_geometries<1, Geometry2>::apply(turns.begin(), turns.end(), geometry2) )
        {
// TODO:
// check if there is an interior and/or boundary
            res.template update<exterior, interior, '1'>();
            res.template update<exterior, boundary, '0'>();
        }

        boundary_checker<Geometry1> boundary_checker1(geometry1);
        boundary_checker<Geometry2> boundary_checker2(geometry2);

        // TODO: turns must be sorted and followed only if it's possible to go out and in on the same point
        // for linear geometries union operation must be detected which I guess would be quite often

        {
            std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,2,3,1,4,0,0>());

            turns_analyser<0, point1_type> analyser;
            analyse_each_turn(res, analyser,
                              turns.begin(), turns.end(),
                              geometry1, geometry2,
                              boundary_checker1, boundary_checker2);
        }
        
        {
            std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,2,3,1,4,0,1>());

            turns_analyser<1, point1_type> analyser;
            analyse_each_turn(res, analyser,
                              turns.begin(), turns.end(),
                              geometry2, geometry1,
                              boundary_checker2, boundary_checker1);
        }

        return res;
    }

    // TODO: rename to point_id_ref?
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

        bool operator()(segment_identifier const& sid) const
        {
            return sid.multi_index == sid_ptr->multi_index
                && sid.ring_index == sid_ptr->ring_index;                
        }

        template <typename Point>
        bool operator()(point_identifier<Point> const& pid) const
        {
            return operator()(pid.seg_id());
        }

    private:
        const segment_identifier * sid_ptr;
    };

    class segment_watcher
    {
    public:
        segment_watcher()
            : m_seg_id_ptr(0)
        {}

        bool update(segment_identifier const& seg_id)
        {
            bool result = m_seg_id_ptr == 0 || !same_ranges(*m_seg_id_ptr)(seg_id);
            m_seg_id_ptr = boost::addressof(seg_id);
            return result;
        }

    private:
        const segment_identifier * m_seg_id_ptr;
    };

    template <typename Point>
    class exit_watcher
    {
        typedef point_identifier<Point> point_identifier;

    public:
        exit_watcher()
            : exit_operation(overlay::operation_none)
        {}

        // returns true if before the call we were outside
        bool enter(Point const& point, segment_identifier const& other_id)
        {
            bool result = other_entry_points.empty();
            other_entry_points.push_back(point_identifier(other_id, point));
            return result;
        }

        // returns true if before the call we were outside
        bool exit(Point const& point,
                  segment_identifier const& other_id,
                  overlay::operation_type exit_op)
        {
            // if we didn't entered anything in the past, we're outside
            if ( other_entry_points.empty() )
                return true;

            typedef typename std::vector<point_identifier>::iterator point_iterator;
            // search for the entry point in the same range of other geometry
            point_iterator entry_it = std::find_if(other_entry_points.begin(),
                                                   other_entry_points.end(),
                                                   same_ranges(other_id));

            // this end point has corresponding entry point
            if ( entry_it != other_entry_points.end() )
            {
                // here we know that we possibly left LS
                // we must still check if we didn't get back on the same point
                exit_operation = exit_op;
                exit_id = point_identifier(other_id, point);

                // erase the corresponding entry point
                other_entry_points.erase(entry_it);
            }

            return false;
        }

        overlay::operation_type get_exit_operation() const
        {
            return exit_operation;
        }

        Point const& get_exit_point() const
        {
            BOOST_ASSERT(exit_operation != overlay::operation_none);
            return exit_id.point();
        }

        void reset_detected_exit()
        {
            exit_operation = overlay::operation_none;
        }

    private:
        overlay::operation_type exit_operation;
        point_identifier exit_id;
        std::vector<point_identifier> other_entry_points; // TODO: use map here or sorted vector?
    };

    template <std::size_t OpId, typename TurnPoint>
    class turns_analyser
    {
        static const std::size_t op_id = OpId;
        static const std::size_t other_op_id = (OpId + 1) % 2;
        static const bool reverse_result = OpId != 0;

    public:
        turns_analyser()
            : m_last_union(false)
        {}

        template <typename TurnIt,
                  typename Geometry,
                  typename OtherGeometry,
                  typename BoundaryChecker,
                  typename OtherBoundaryChecker>
        void apply(result & res,
                   TurnIt first, TurnIt it, TurnIt last,
                   Geometry const& geometry,
                   OtherGeometry const& other_geometry,
                   BoundaryChecker & boundary_checker,
                   OtherBoundaryChecker & other_boundary_checker)
        {
            if ( it != last )
            {
                overlay::operation_type op = it->operations[op_id].operation;

                if ( op != overlay::operation_union
                  && op != overlay::operation_intersection
                  && op != overlay::operation_blocked )
                {
                    return;
                }

                segment_identifier const& seg_id = it->operations[op_id].seg_id;
                segment_identifier const& other_id = it->operations[other_op_id].seg_id;

                const bool first_in_range = m_seg_watcher.update(seg_id);

                // handle possible exit
                bool fake_enter_detected = false;
                if ( m_exit_watcher.get_exit_operation() == overlay::operation_union )
                {
                    // real exit point - may be multiple
                    // we know that we entered and now we exit
                    if ( !detail::equals::equals_point_point(it->point, m_exit_watcher.get_exit_point()) )
                    {
                        m_exit_watcher.reset_detected_exit();
                    
                        // not the last IP
                        update_result<interior, exterior, '1', reverse_result>(res);
                    }
                    // fake exit point, reset state
                    // in reality this will be op == overlay::operation_intersection
                    else if ( op == overlay::operation_intersection )
                    {
                        m_exit_watcher.reset_detected_exit();
                        fake_enter_detected = true;
                    }
                }

                if ( first_in_range && !fake_enter_detected && m_last_union )
                {
                    BOOST_ASSERT(it != first);
// TODO: ERROR!
// PREV MAY NOT BE VALID!
                    TurnIt prev = it;
                    --prev;
                    segment_identifier const& prev_seg_id = prev->operations[op_id].seg_id;

                    // if there is a boundary on the last point
                    if ( boost::size(sub_geometry::get(geometry, prev_seg_id)) > 1
                      && boundary_checker.template is_boundary<boundary_back_explicit>
                            (range::back(sub_geometry::get(geometry, prev_seg_id)), prev_seg_id) )
                    {
                        update_result<boundary, exterior, '0', reverse_result>(res);
                    }
                }

                // reset state
                m_last_union = (op == overlay::operation_union);

                // i/i, i/x, i/u
                if ( op == overlay::operation_intersection )
                {
                    bool was_outside = m_exit_watcher.enter(it->point, other_id);

                    // interiors overlaps
                    update_result<interior, interior, '1', reverse_result>(res);
                
                    // going inside on boundary point
                    if ( boundary_checker.template is_boundary<boundary_front>(it->point, seg_id) )
                    {
                        bool other_b =
                            it->operations[other_op_id].operation == overlay::operation_blocked ?
                                other_boundary_checker.template is_boundary<boundary_back_explicit>(it->point, other_id) :
                                other_boundary_checker.template is_boundary<boundary_any>(it->point, other_id);                        
                        // it's also the boundary of the other geometry
                        if ( other_b )
                        {
                            update_result<boundary, boundary, '0', reverse_result>(res);
                        }
                        else
                        {
                            update_result<boundary, interior, '0', reverse_result>(res);
                        }
                    }
                    // going inside on non-boundary point
                    else
                    {
                        // if we didn't enter in the past, we were outside
                        if ( was_outside && !fake_enter_detected )
                        {
                            update_result<interior, exterior, '1', reverse_result>(res);

                            // if it's the first IP then the first point is outside
                            if ( first_in_range )
                            {
                                segment_identifier first_seg_id = seg_id;
                                first_seg_id.segment_index = 0;

                                // if there is a boundary on the first point
                                if ( boundary_checker.template is_boundary<boundary_front_explicit>
                                        (range::front(sub_geometry::get(geometry, first_seg_id)), first_seg_id) )
                                {
                                    update_result<boundary, exterior, '0', reverse_result>(res);
                                }
                            }
                        }
                    }
                }
                // u/i, u/u, u/x, x/i, x/u, x/x
                else if ( op == overlay::operation_union || op == overlay::operation_blocked )
                {
                    bool op_blocked = op == overlay::operation_blocked;
                    bool was_outside = m_exit_watcher.exit(it->point, other_id, op);

                    // we're inside, possibly going out right now
                    if ( ! was_outside )
                    {
                        if ( op_blocked )
                        {
                            // check if this is indeed the boundary point
                            if ( boundary_checker.template is_boundary<boundary_back_explicit>(it->point, seg_id) )
                            {
                                bool other_b =
                                    it->operations[other_op_id].operation == overlay::operation_blocked ?
                                        other_boundary_checker.template is_boundary<boundary_back_explicit>(it->point, other_id) :
                                        other_boundary_checker.template is_boundary<boundary_any>(it->point, other_id);
                                // it's also the boundary of the other geometry
                                if ( other_b )
                                {
                                    update_result<boundary, boundary, '0', reverse_result>(res);
                                }
                                else
                                {
                                    update_result<boundary, interior, '0', reverse_result>(res);
                                }
                            }
                        }
                    }
                    // we're outside
                    else
                    {
                        update_result<interior, exterior, '1', reverse_result>(res);

                        // boundaries don't overlap - just an optimization
                        if ( it->method == overlay::method_crosses )
                        {
                            update_result<interior, interior, '0', reverse_result>(res);

                            // it's the first point in range
                            if ( first_in_range )
                            {
                                // if there is a boundary on the first point
                                if ( boundary_checker.template is_boundary<boundary_front_explicit>
                                        (range::front(sub_geometry::get(geometry, seg_id)), seg_id) )
                                {
                                    update_result<boundary, exterior, '0', reverse_result>(res);
                                }
                            }
                        }
                        // method other than crosses, check more conditions
                        else
                        {
                            bool this_b =
                                    op_blocked ?
                                        boundary_checker.template is_boundary<boundary_back_explicit>(it->point, seg_id) :
                                        boundary_checker.template is_boundary<boundary_front>(it->point, seg_id);
                        
                            // if current IP is on boundary of the geometry
                            if ( this_b )
                            {
                                bool other_b =
                                    it->operations[other_op_id].operation == overlay::operation_blocked ?
                                        other_boundary_checker.template is_boundary<boundary_back_explicit>(it->point, other_id) :
                                        other_boundary_checker.template is_boundary<boundary_any>(it->point, other_id);
                                // it's also the boundary of the other geometry
                                if ( other_b )
                                {
                                    update_result<boundary, boundary, '0', reverse_result>(res);
                                }
                                else
                                {
                                    update_result<boundary, interior, '0', reverse_result>(res);
                                }

                                // first IP on the last segment point - this means that the first point is outside
                                if ( first_in_range && op_blocked )
                                {
                                    // if there is a boundary on the first point
                                    if ( boundary_checker.template is_boundary<boundary_front_explicit>
                                            (range::front(sub_geometry::get(geometry, seg_id)), seg_id) )
                                    {
                                        update_result<boundary, exterior, '0', reverse_result>(res);
                                    }
                                }
                            }
                            // boundaries don't overlap
                            else
                            {
                                update_result<interior, interior, '0', reverse_result>(res);

                                if ( first_in_range )
                                {
                                    // if there is a boundary on the first point
                                    if ( boundary_checker.template is_boundary<boundary_front_explicit>
                                            (range::front(sub_geometry::get(geometry, seg_id)), seg_id) )
                                    {
                                        update_result<boundary, exterior, '0', reverse_result>(res);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // it == last
            else
            {
                // here, the possible exit is the real one
                // we know that we entered and now we exit
                if ( m_exit_watcher.get_exit_operation() == overlay::operation_union // THIS CHECK IS REDUNDANT
                  || m_last_union )
                {
                    // for sure
                    update_result<interior, exterior, '1', reverse_result>(res);

                    BOOST_ASSERT(first != last);
// TODO: ERROR!
// PREV MAY NOT BE VALID!
                    TurnIt prev = last;
                    --prev;
                    segment_identifier prev_seg_id = prev->operations[OpId].seg_id;
                    std::size_t points_count = boost::size(sub_geometry::get(geometry, prev_seg_id));
                    
                    if ( points_count > 1 )
                    {
                        prev_seg_id.segment_index = points_count - 2;

                        // if there is a boundary on the last point
                        if ( boundary_checker.template is_boundary<boundary_back_explicit>
                                (range::back(sub_geometry::get(geometry, prev_seg_id)), prev_seg_id) )
                        {
                            update_result<boundary, exterior, '0', reverse_result>(res);
                        }
                    }
                }
            }
        }

    private:
        exit_watcher<TurnPoint> m_exit_watcher;
        segment_watcher m_seg_watcher;
        bool m_last_union;
    };

    template <typename TurnIt,
              typename Analyser,
              typename Geometry,
              typename OtherGeometry,
              typename BoundaryChecker,
              typename OtherBoundaryChecker>
    static inline void analyse_each_turn(result & res,
                                         Analyser & analyser,
                                         TurnIt first, TurnIt last,
                                         Geometry const& geometry,
                                         OtherGeometry const& other_geometry,
                                         BoundaryChecker & boundary_checker,
                                         OtherBoundaryChecker & other_boundary_checker)
    {
        if ( first == last )
            return;

        for ( TurnIt it = first ; it != last ; ++it )
        {
            analyser.apply(res, first, it, last,
                           geometry, other_geometry,
                           boundary_checker, other_boundary_checker);
        }

        analyser.apply(res, first, last, last,
                       geometry, other_geometry,
                       boundary_checker, other_boundary_checker);
    }
    
    //template <typename TurnIt>
    //static inline void analyse_turns_simple(result & res,
    //                                        TurnIt first, TurnIt last,
    //                                        Geometry1 const& geometry1,
    //                                        Geometry2 const& geometry2,
    //                                        bool has_boundary1, bool has_boundary2)
    //{
    //    for ( TurnIt it = first ; it != last ; ++it )
    //    {
    //        // 'i'
    //        if ( it->method == overlay::method_crosses )
    //        {
    //            res.template update<interior, interior, '0'>(); // always true
    //            res.template update<interior, exterior, '1'>(); // not always true
    //            res.template update<exterior, interior, '1'>(); // not always true
    //        }
    //        // 'e' 'c'
    //        else if ( it->method == overlay::method_equal
    //               || it->method == overlay::method_collinear )
    //        {
    //            res.template update<interior, interior, '1'>(); // always true
    //        }
    //        // 't' 'm'
    //        else if ( it->method == overlay::method_touch
    //               || it->method == overlay::method_touch_interior )
    //        {
    //            bool b = handle_boundary_point(res, *it, geometry1, geometry2, has_boundary1, has_boundary2);
    //            
    //            if ( it->has(overlay::operation_union) )
    //            {
    //                if ( !b )
    //                    res.template update<interior, interior, '0'>();
    //                if ( it->operations[0].operation == overlay::operation_union )
    //                    res.template update<interior, exterior, '1'>();
    //                if ( it->operations[1].operation == overlay::operation_union )
    //                    res.template update<exterior, interior, '1'>();
    //            }

    //            if ( it->has(overlay::operation_intersection) )
    //                res.template update<interior, interior, '1'>();

    //            if ( it->has(overlay::operation_blocked) )
    //                if ( !b )
    //                    res.template update<interior, interior, '0'>();
    //        }
    //        
    //    }
    //}

    //template <typename Turn>
    //static inline bool handle_boundary_point(result & res,
    //                                         Turn const& turn,
    //                                         Geometry1 const& geometry1, Geometry2 const& geometry2,
    //                                         bool has_boundary1, bool has_boundary2)
    //{
    //    bool pt_on_boundary1 = has_boundary1 && equals_terminal_point(turn.point, geometry1);
    //    bool pt_on_boundary2 = has_boundary2 && equals_terminal_point(turn.point, geometry2);

    //    if ( pt_on_boundary1 && pt_on_boundary2 )
    //        res.template update<boundary, boundary, '0'>();
    //    else if ( pt_on_boundary1 )
    //        res.template update<boundary, interior, '0'>();
    //    else if ( pt_on_boundary2 )
    //        res.template update<interior, boundary, '0'>();
    //    else
    //        return false;
    //    return true;
    //}

    //// TODO: replace with generic point_in_boundary working also for multilinestrings
    //template <typename Point, typename Geometry>
    //static inline bool equals_terminal_point(Point const& point, Geometry const& geometry)
    //{
    //    return detail::equals::equals_point_point(point, range::front(geometry))
    //        || detail::equals::equals_point_point(point, range::back(geometry));
    //}

    //static inline void handle_boundaries(result & res,
    //                                     Geometry1 const& geometry1, Geometry2 const& geometry2,
    //                                     bool has_boundary1, bool has_boundary2)
    //{
    //    int pig_front = detail::within::point_in_geometry(range::front(geometry1), geometry2);

    //    if ( has_boundary1 )
    //    {
    //        int pig_back = detail::within::point_in_geometry(range::back(geometry1), geometry2);

    //        if ( pig_front > 0 || pig_back > 0 )
    //            res.template set<boundary, interior, '0'>();
    //        if ( pig_front == 0 || pig_back == 0 )
    //            res.template set<boundary, boundary, '0'>();
    //        if ( pig_front < 0 || pig_back < 0 )
    //        {
    //            res.template set<boundary, exterior, '0'>();
    //            res.template set<interior, exterior, '1'>();
    //        }
    //    }
    //    else
    //    {
    //        if ( pig_front > 0 )
    //            res.template set<interior, interior, '0'>();
    //        else if ( pig_front == 0 )
    //            res.template set<interior, boundary, '0'>();
    //        else if ( pig_front < 0 )
    //            res.template set<interior, exterior, '0'>();
    //    }

    //    pig_front = detail::within::point_in_geometry(range::front(geometry2), geometry1);

    //    if ( has_boundary2 )
    //    {
    //        int pig_back = detail::within::point_in_geometry(range::back(geometry2), geometry1);

    //        if ( pig_front > 0 || pig_back > 0 )
    //            res.template set<interior, boundary, '0'>();
    //        if ( pig_front == 0 || pig_back == 0 )
    //            res.template set<boundary, boundary, '0'>();
    //        if ( pig_front < 0 || pig_back < 0 )
    //        {
    //            res.template set<exterior, boundary, '0'>();
    //            res.template set<exterior, interior, '1'>();
    //        }
    //    }
    //    else
    //    {
    //        if ( pig_front > 0 )
    //            res.template set<interior, interior, '0'>();
    //        else if ( pig_front == 0 )
    //            res.template set<boundary, interior, '0'>();
    //        else if ( pig_front < 0 )
    //            res.template set<exterior, interior, '0'>();
    //    }
    //}
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
