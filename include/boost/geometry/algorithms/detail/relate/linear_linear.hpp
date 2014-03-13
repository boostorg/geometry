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

#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/turns.hpp>
#include <boost/geometry/algorithms/detail/relate/boundary_checker.hpp>
#include <boost/geometry/algorithms/detail/relate/follow_helpers.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

template <typename Result, typename BoundaryChecker, bool TransposeResult>
class disjoint_linestring_pred
{
public:
    disjoint_linestring_pred(Result & res,
                             BoundaryChecker & boundary_checker)
        : m_result_ptr(boost::addressof(res))
        , m_boundary_checker_ptr(boost::addressof(boundary_checker))
    {}

    template <typename Linestring>
    bool operator()(Linestring const& linestring)
    {
        std::size_t count = boost::size(linestring);
        
        // invalid input
        if ( count < 2 )
        {
            // ignore
            // TODO: throw an exception?
            return true;
        }

        update<interior, exterior, '1', TransposeResult>(*m_result_ptr);

        // check if there is a boundary
        if ( m_boundary_checker_ptr->template
                is_endpoint_boundary<boundary_front>(range::front(linestring))
          || m_boundary_checker_ptr->template
                is_endpoint_boundary<boundary_back>(range::back(linestring)) )
        {
            update<boundary, exterior, '0', TransposeResult>(*m_result_ptr);
                    
            return false;
        }

        return true;
    }

private:
    Result * m_result_ptr;
    BoundaryChecker * m_boundary_checker_ptr;
};

//enum linestring_kind { linestring_exterior, linestring_point, linestring_closed, linestring_open };
//
//template <typename Linestring>
//linestring_kind check_linestring_kind(Linestring const& ls)
//{
//    std::size_t count = boost::size(ls);
//    if ( count == 0 )
//        return linestring_exterior;
//    else if ( count == 1 )
//        return linestring_point;
//    else
//    {
//        bool equal_fb = equals::equals_point_point(range::front(ls), range::back(ls));
//        if ( equal_fb )
//        {
//            typedef typename boost::range_iterator<Linestring const>::type iterator;
//            iterator first = boost::begin(ls);
//            ++first;
//            iterator last = boost::end(ls);
//            --last;
//            for ( iterator it = first ; it != last ; ++it )
//            {
//                if ( !equals::equals_point_point(range::front(ls), *it) )
//                    return linestring_closed;
//            }
//
//            return linestring_point;
//        }
//        else
//            return linestring_open;
//    }
//}

// Called in a loop for:
// Ls/Ls - worst O(N) - 1x point_in_geometry(MLs)
// Ls/MLs - worst O(N) - 1x point_in_geometry(MLs)
// MLs/Ls - worst O(N^2) - Bx point_in_geometry(Ls)
// MLs/MLs - worst O(N^2) - Bx point_in_geometry(Ls)
// TODO: later use spatial index
//template <std::size_t OpId, typename Result, typename BoundaryChecker, typename OtherGeometry>
//class disjoint_linestring_pred_with_point_size_handling
//{
//    static const bool transpose_result = OpId != 0;
//
//public:
//    disjoint_linestring_pred_with_point_size_handling(Result & res,
//                                                      BoundaryChecker & boundary_checker,
//                                                      OtherGeometry const& other_geometry)
//        : m_result_ptr(boost::addressof(res))
//        , m_boundary_checker_ptr(boost::addressof(boundary_checker))
//        , m_other_geometry(boost::addressof(other_geometry))
//        , m_detected_mask_point(0)
//        , m_detected_open_boundary(false)
//    {}
//
//    template <typename Linestring>
//    bool operator()(Linestring const& linestring)
//    {
//        linestring_kind lk = check_linestring_kind(linestring);
//
//        if ( lk == linestring_point ) // just an optimization
//        {
//            if ( m_detected_mask_point != 7 )
//            {
//                // check the relation
//                int pig = within::point_in_geometry(range::front(linestring), *m_other_geometry);
//
//                // point inside
//                if ( pig > 0 )
//                {
//                    update<interior, interior, '0', transpose_result>(*m_result_ptr);
//                    m_detected_mask_point |= 1;
//                }
//                // point on boundary
//                else if ( pig == 0 )
//                {
//                    update<interior, boundary, '0', transpose_result>(*m_result_ptr);
//                    m_detected_mask_point |= 2;
//                }
//                // point outside
//                else
//                {
//                    update<interior, exterior, '0', transpose_result>(*m_result_ptr);
//                    m_detected_mask_point |= 4;
//                }
//            }
//        }
//        // NOTE: For closed Linestrings I/I=1 could be set automatically
//        // but for MultiLinestrings endpoints of closed Linestrings must also be checked for boundary
//        else if ( lk == linestring_open || lk == linestring_closed )
//        {
//            if ( !m_detected_open_boundary ) // just an optimization
//            {
//                update<interior, exterior, '1', transpose_result>(*m_result_ptr);
//
//                // check if there is a boundary
//                if ( m_boundary_checker_ptr->template
//                        is_endpoint_boundary<boundary_front>(range::front(linestring))
//                    || m_boundary_checker_ptr->template
//                        is_endpoint_boundary<boundary_back>(range::back(linestring)) )
//                {
//                    update<boundary, exterior, '0', transpose_result>(*m_result_ptr);
//                    
//                    m_detected_open_boundary = true;
//                }
//            }
//        }
//
//        bool all_detected = m_detected_mask_point == 7 && m_detected_open_boundary;
//        return !all_detected && !m_result_ptr->interrupt;
//    }
//
//private:
//    Result * m_result_ptr;
//    BoundaryChecker * m_boundary_checker_ptr;
//    const OtherGeometry * m_other_geometry;
//    char m_detected_mask_point;
//    bool m_detected_open_boundary;
//};

template <typename Geometry1, typename Geometry2>
struct linear_linear
{
    static const bool interruption_enabled = true;

    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;

    template <typename Result>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2, Result & result)
    {
        // The result should be FFFFFFFFF
        set<exterior, exterior, result_dimension<Geometry1>::value>(result);// FFFFFFFFd, d in [1,9] or T
        if ( result.interrupt )
            return;

        // get and analyse turns
        typedef typename turns::get_turns<Geometry1, Geometry2>::turn_info turn_type;
        typedef typename std::vector<turn_type>::iterator turn_iterator;
        std::vector<turn_type> turns;

// TODO: INTEGRATE INTERRUPT POLICY WITH THE PASSED RESULT

        turns::get_turns<Geometry1, Geometry2>::apply(turns, geometry1, geometry2);

        boundary_checker<Geometry1> boundary_checker1(geometry1);
        disjoint_linestring_pred<Result, boundary_checker<Geometry1>, false> pred1(result, boundary_checker1);
        for_each_disjoint_geometry_if<0, Geometry1>::apply(turns.begin(), turns.end(), geometry1, pred1);
        if ( result.interrupt )
            return;

        boundary_checker<Geometry2> boundary_checker2(geometry2);
        disjoint_linestring_pred<Result, boundary_checker<Geometry2>, true> pred2(result, boundary_checker2);
        for_each_disjoint_geometry_if<1, Geometry2>::apply(turns.begin(), turns.end(), geometry2, pred2);
        if ( result.interrupt )
            return;
        
        if ( turns.empty() )
            return;

        // TODO: turns must be sorted and followed only if it's possible to go out and in on the same point
        // for linear geometries union operation must be detected which I guess would be quite often

// TODO: ADD A CHECK TO THE RESULT INDICATING IF THE FIRST AND/OR SECOND GEOMETRY MUST BE ANALYSED

        {
            // x, u, i, c
            std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,2,3,1,4,0,0>());

            turns_analyser<0, turn_type> analyser;
            analyse_each_turn(result, analyser,
                              turns.begin(), turns.end(),
                              geometry1, geometry2,
                              boundary_checker1, boundary_checker2);
        }

        if ( result.interrupt )
            return;
        
        {
            // x, u, i, c
            std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,2,3,1,4,0,1>());

            turns_analyser<1, turn_type> analyser;
            analyse_each_turn(result, analyser,
                              turns.begin(), turns.end(),
                              geometry2, geometry1,
                              boundary_checker2, boundary_checker1);
        }
    }

    // This analyser should be used like Input or SinglePass Iterator
    template <std::size_t OpId, typename TurnInfo>
    class turns_analyser
    {
        typedef typename TurnInfo::point_type turn_point_type;

        static const std::size_t op_id = OpId;
        static const std::size_t other_op_id = (OpId + 1) % 2;
        static const bool transpose_result = OpId != 0;

    public:
        turns_analyser()
            : m_previous_turn_ptr(0)
            , m_previous_operation(overlay::operation_none)
        {}

        template <typename Result,
                  typename TurnIt,
                  typename Geometry,
                  typename OtherGeometry,
                  typename BoundaryChecker,
                  typename OtherBoundaryChecker>
        void apply(Result & res,
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
                        update<interior, exterior, '1', transpose_result>(res);
                    }
                    // fake exit point, reset state
                    else if ( op == overlay::operation_intersection )
                    {
                        m_exit_watcher.reset_detected_exit();
                        fake_enter_detected = true;
                    }
                }
                else if ( m_exit_watcher.get_exit_operation() == overlay::operation_blocked )
                {
                    // ignore multiple BLOCKs
                    if ( op == overlay::operation_blocked )
                        return;

                    m_exit_watcher.reset_detected_exit();
                }

                // if the new linestring started just now,
                // but the previous one went out on the previous point,
                // we must check if the boundary of the previous segment is outside
                // NOTE: couldn't it be integrated with the handling of the union above?
                // THIS IS REDUNDANT WITH THE HANDLING OF THE END OF THE RANGE
                //if ( first_in_range
                //  && ! fake_enter_detected
                //  && m_previous_operation == overlay::operation_union )
                //{
                //    BOOST_ASSERT(it != first);
                //    BOOST_ASSERT(m_previous_turn_ptr);

                //    segment_identifier const& prev_seg_id = m_previous_turn_ptr->operations[op_id].seg_id;

                //    bool prev_back_b = is_endpoint_on_boundary<boundary_back>(
                //                            range::back(sub_geometry::get(geometry, prev_seg_id)),
                //                            boundary_checker);

                //    // if there is a boundary on the last point
                //    if ( prev_back_b )
                //    {
                //        update<boundary, exterior, '0', transpose_result>(res);
                //    }
                //}

                // i/i, i/x, i/u
                if ( op == overlay::operation_intersection )
                {
                    bool was_outside = m_exit_watcher.is_outside();
                    m_exit_watcher.enter(it->point, other_id);

                    // interiors overlaps
                    update<interior, interior, '1', transpose_result>(res);
                
                    bool this_b = is_ip_on_boundary<boundary_front>(it->point,
                                                                    it->operations[op_id],
                                                                    boundary_checker,
                                                                    seg_id);

                    // going inside on boundary point
                    // may be front only
                    if ( this_b )
                    {
                        // may be front and back
                        bool other_b = is_ip_on_boundary<boundary_any>(it->point,
                                                                       it->operations[other_op_id],
                                                                       other_boundary_checker,
                                                                       other_id);

                        // it's also the boundary of the other geometry
                        if ( other_b )
                        {
                            update<boundary, boundary, '0', transpose_result>(res);
                        }
                        else
                        {
                            update<boundary, interior, '0', transpose_result>(res);
                        }
                    }
                    // going inside on non-boundary point
                    else
                    {
                        // if we didn't enter in the past, we were outside
                        if ( was_outside && !fake_enter_detected )
                        {
                            update<interior, exterior, '1', transpose_result>(res);

                            // if it's the first IP then the first point is outside
                            if ( first_in_range )
                            {
                                bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                    range::front(sub_geometry::get(geometry, seg_id)),
                                                    boundary_checker);

                                // if there is a boundary on the first point
                                if ( front_b )
                                {
                                    update<boundary, exterior, '0', transpose_result>(res);
                                }
                            }
                        }
                    }
                }
                // u/i, u/u, u/x, x/i, x/u, x/x
                else if ( op == overlay::operation_union || op == overlay::operation_blocked )
                {
                    // TODO: is exit watcher still needed?
                    // couldn't is_collinear and some going inside counter be used instead?

                    bool is_collinear = it->operations[op_id].is_collinear;
                    bool was_outside = m_exit_watcher.is_outside();

                    // to exit we must be currently inside and the current segment must be collinear
                    if ( !was_outside && is_collinear )
                    {
                        m_exit_watcher.exit(it->point, other_id, op);
                    }

                    bool op_blocked = op == overlay::operation_blocked;

                    // we're inside and going out from inside
                    // possibly going out right now
                    if ( ! was_outside && is_collinear )
                    {
                        if ( op_blocked )
                        {
                            // check if this is indeed the boundary point
                            // NOTE: is_ip_on_boundary<>() should be called here but the result will be the same
                            if ( is_endpoint_on_boundary<boundary_back>(it->point, boundary_checker) )
                            {
                                // may be front and back
                                bool other_b = is_ip_on_boundary<boundary_any>(it->point,
                                                                               it->operations[other_op_id],
                                                                               other_boundary_checker,
                                                                               other_id);
                                // it's also the boundary of the other geometry
                                if ( other_b )
                                {
                                    update<boundary, boundary, '0', transpose_result>(res);
                                }
                                else
                                {
                                    update<boundary, interior, '0', transpose_result>(res);
                                }
                            }
                        }
                    }
                    // we're outside or intersects some segment from the outside
                    else
                    {
                        // if we are truly outside
                        if ( was_outside /*&& !is_collinear*/ )
                        {
                            update<interior, exterior, '1', transpose_result>(res);
                        }

                        // boundaries don't overlap - just an optimization
                        if ( it->method == overlay::method_crosses )
                        {
                            // the L1 is going from one side of the L2 to the other through the point
                            update<interior, interior, '0', transpose_result>(res);

                            // it's the first point in range
                            if ( first_in_range )
                            {
                                bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                    range::front(sub_geometry::get(geometry, seg_id)),
                                                    boundary_checker);

                                // if there is a boundary on the first point
                                if ( front_b )
                                {
                                    update<boundary, exterior, '0', transpose_result>(res);
                                }
                            }
                        }
                        // method other than crosses, check more conditions
                        else
                        {
                            bool this_b = is_ip_on_boundary<boundary_any>(it->point,
                                                                          it->operations[op_id],
                                                                          boundary_checker,
                                                                          seg_id);

                            bool other_b = is_ip_on_boundary<boundary_any>(it->point,
                                                                           it->operations[other_op_id],
                                                                           other_boundary_checker,
                                                                           other_id);
                        
                            // if current IP is on boundary of the geometry
                            if ( this_b )
                            {
                                // it's also the boundary of the other geometry
                                if ( other_b )
                                    update<boundary, boundary, '0', transpose_result>(res);
                                else
                                    update<boundary, interior, '0', transpose_result>(res);
                            }
                            // if current IP is not on boundary of the geometry
                            else
                            {
                                // it's also the boundary of the other geometry
                                if ( other_b )
                                    update<interior, boundary, '0', transpose_result>(res);
                                else
                                    update<interior, interior, '0', transpose_result>(res);
                            }

                            // first IP on the last segment point - this means that the first point is outside
                            if ( first_in_range && ( !this_b || op_blocked ) && was_outside /*&& !is_collinear*/ )
                            {
                                bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                    range::front(sub_geometry::get(geometry, seg_id)),
                                                    boundary_checker);

                                // if there is a boundary on the first point
                                if ( front_b )
                                {
                                    update<boundary, exterior, '0', transpose_result>(res);
                                }
                            }
                            
                        }
                    }
                }

                // store ref to previously analysed (valid) turn
                m_previous_turn_ptr = boost::addressof(*it);
                // and previously analysed (valid) operation
                m_previous_operation = op;
            }
            // it == last
            else
            {
                // here, the possible exit is the real one
                // we know that we entered and now we exit
                if ( /*m_exit_watcher.get_exit_operation() == overlay::operation_union // THIS CHECK IS REDUNDANT
                  ||*/ m_previous_operation == overlay::operation_union )
                {
                    // for sure
                    update<interior, exterior, '1', transpose_result>(res);

                    BOOST_ASSERT(first != last);
                    BOOST_ASSERT(m_previous_turn_ptr);

                    segment_identifier const& prev_seg_id = m_previous_turn_ptr->operations[op_id].seg_id;

                    bool prev_back_b = is_endpoint_on_boundary<boundary_back>(
                                            range::back(sub_geometry::get(geometry, prev_seg_id)),
                                            boundary_checker);

                    // if there is a boundary on the last point
                    if ( prev_back_b )
                    {
                        update<boundary, exterior, '0', transpose_result>(res);
                    }
                }

                // Just in case,
                // reset exit watcher before the analysis of the next Linestring
                // note that if there are some enters stored there may be some error above
                m_exit_watcher.reset();
            }
        }

    private:
        exit_watcher<turn_point_type> m_exit_watcher;
        segment_watcher m_seg_watcher;
        TurnInfo * m_previous_turn_ptr;
        overlay::operation_type m_previous_operation;
    };

    template <typename Result,
              typename TurnIt,
              typename Analyser,
              typename Geometry,
              typename OtherGeometry,
              typename BoundaryChecker,
              typename OtherBoundaryChecker>
    static inline void analyse_each_turn(Result & res,
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

            if ( res.interrupt )
                return;
        }

        analyser.apply(res, first, last, last,
                       geometry, other_geometry,
                       boundary_checker, other_boundary_checker);
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
