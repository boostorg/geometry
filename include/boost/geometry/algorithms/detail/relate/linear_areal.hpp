// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_AREAL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_AREAL_HPP

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

template <typename Geometry2, typename Result, typename BoundaryChecker, bool TransposeResult>
class no_turns_la_linestring_pred
{
public:
    no_turns_la_linestring_pred(Geometry2 const& geometry2,
                                Result & res,
                                BoundaryChecker & boundary_checker)
        : m_geometry2(geometry2)
        , m_result_ptr(boost::addressof(res))
        , m_boundary_checker_ptr(boost::addressof(boundary_checker))
        , m_interrupt_flags(0)
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

        int pig = detail::within::point_in_geometry(range::front(linestring), m_geometry2);
        BOOST_ASSERT_MSG(pig != 0, "There should be no IPs");

        if ( pig > 0 )
        {
            update<interior, interior, '1', TransposeResult>(*m_result_ptr);
            m_interrupt_flags |= 1;
        }
        else
        {
            update<interior, exterior, '1', TransposeResult>(*m_result_ptr);
            m_interrupt_flags |= 2;
        }

        // check if there is a boundary
        if ( m_boundary_checker_ptr->template
                is_endpoint_boundary<boundary_front>(range::front(linestring))
          || m_boundary_checker_ptr->template
                is_endpoint_boundary<boundary_back>(range::back(linestring)) )
        {
            if ( pig > 0 )
            {
                update<boundary, interior, '0', TransposeResult>(*m_result_ptr);
                m_interrupt_flags |= 4;
            }
            else
            {
                update<boundary, exterior, '0', TransposeResult>(*m_result_ptr);
                m_interrupt_flags |= 8;
            }

            return m_interrupt_flags != 0xF;
        }

        return true;
    }

private:
    Geometry2 const& m_geometry2;
    Result * m_result_ptr;
    BoundaryChecker * m_boundary_checker_ptr;
    unsigned m_interrupt_flags;
};

template <typename Result, bool TransposeResult>
class no_turns_la_areal_pred
{
public:
    no_turns_la_areal_pred(Result & res)
        : m_result_ptr(boost::addressof(res))
    {}

    template <typename Areal>
    bool operator()(Areal const& areal)
    {
        // TODO:
        // add an assertion for empty/invalid geometries

        update<interior, exterior, '2', TransposeResult>(*m_result_ptr);
        update<boundary, exterior, '1', TransposeResult>(*m_result_ptr);
                    
        return false;
    }

private:
    Result * m_result_ptr;
};

template <typename Geometry1, typename Geometry2, bool TransposeResult = false>
struct linear_areal
{
    // check Linear / Areal
    BOOST_STATIC_ASSERT(detail::group_dim<Geometry1>::value == 1
                     && detail::group_dim<Geometry2>::value == 2);

    static const bool interruption_enabled = true;

    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;
    
    template <typename Result>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2, Result & result)
    {
        // The result should be FFFFFFFFF
        set<exterior, exterior, result_dimension<Geometry2>::value, TransposeResult>(result);// FFFFFFFFd, d in [1,9] or T

        if ( result.interrupt )
            return;

        // get and analyse turns
        typedef typename turns::get_turns<Geometry1, Geometry2>::turn_info turn_type;
        typedef typename std::vector<turn_type>::iterator turn_iterator;
        std::vector<turn_type> turns;

// TODO: INTEGRATE INTERRUPT POLICY WITH THE PASSED RESULT

        turns::get_turns<Geometry1, Geometry2>::apply(turns, geometry1, geometry2);

        boundary_checker<Geometry1> boundary_checker1(geometry1);
        no_turns_la_linestring_pred
            <
                Geometry2,
                Result,
                boundary_checker<Geometry1>,
                TransposeResult
            > pred1(geometry2, result, boundary_checker1);
        for_each_disjoint_geometry_if<0, Geometry1>::apply(turns.begin(), turns.end(), geometry1, pred1);
        if ( result.interrupt )
            return;

        no_turns_la_areal_pred<Result, !TransposeResult> pred2(result);
        for_each_disjoint_geometry_if<1, Geometry2>::apply(turns.begin(), turns.end(), geometry2, pred2);
        if ( result.interrupt )
            return;
        
        if ( turns.empty() )
            return;

        // This is set here because in the case if empty Areal geometry were passed
        // those shouldn't be set
        set<exterior, interior, '2', TransposeResult>(result);// FFFFFF2Fd
// TODO: NEED TO SUPPORT exterior, boundary
//       FOR THIS WE MUST CHECK IF THE BOUNDARY OF THE POLYGON TREATED LIKE LINEAR RING GOES OUT OF THE LINESTRING G1
// THIS WON'T WORK IN ALL CASES!
        set<exterior, boundary, '1', TransposeResult>(result);// FFFFFF21d
        if ( result.interrupt )
            return;

        // x, u, i, c
        std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,2,3,1,4,0,0>());

        turns_analyser<turn_type> analyser;
        analyse_each_turn(result, analyser,
                          turns.begin(), turns.end(),
                          geometry1, geometry2,
                          boundary_checker1);
    }

    // This analyser should be used like Input or SinglePass Iterator
    template <typename TurnInfo>
    class turns_analyser
    {
        typedef typename TurnInfo::point_type turn_point_type;

        static const std::size_t op_id = 0;
        static const std::size_t other_op_id = 1;

    public:
        turns_analyser()
            : m_previous_turn_ptr(0)
            , m_previous_operation(overlay::operation_none)
        {}

        template <typename Result,
                  typename TurnIt,
                  typename Geometry,
                  typename OtherGeometry,
                  typename BoundaryChecker>
        void apply(Result & res,
                   TurnIt first, TurnIt it, TurnIt last,
                   Geometry const& geometry,
                   OtherGeometry const& other_geometry,
                   BoundaryChecker & boundary_checker)
        {
            if ( it != last )
            {
                overlay::operation_type op = it->operations[op_id].operation;

                if ( op != overlay::operation_union
                  && op != overlay::operation_intersection
                  && op != overlay::operation_blocked
                  && op != overlay::operation_continue ) // operation_boundary / operation_boundary_intersection
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
                        update<interior, exterior, '1', TransposeResult>(res);
                    }
                    // fake exit point, reset state
                    else if ( op == overlay::operation_intersection
                           || op == overlay::operation_continue ) // operation_boundary
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
                //        update<boundary, exterior, '0', TransposeResult>(res);
                //    }
                //}

                // i/u, c/u
                if ( op == overlay::operation_intersection
                  || op == overlay::operation_continue ) // operation_boundary/operation_boundary_intersection
                {
                    bool was_outside = m_exit_watcher.is_outside();
                    m_exit_watcher.enter(it->point, other_id);

                    if ( op == overlay::operation_intersection )
                    {
                        // interiors overlaps
                        update<interior, interior, '1', TransposeResult>(res);
                    }
                    else // operation_boundary
                    {
                        update<interior, boundary, '1', TransposeResult>(res);
                    }

                    bool this_b = is_ip_on_boundary<boundary_front>(it->point,
                                                                    it->operations[op_id],
                                                                    boundary_checker,
                                                                    seg_id);
                    // going inside on boundary point
                    if ( this_b )
                    {
                        update<boundary, boundary, '0', TransposeResult>(res);
                    }
                    // going inside on non-boundary point
                    else
                    {
                        update<interior, boundary, '0', TransposeResult>(res);

                        // if we didn't enter in the past, we were outside
                        if ( was_outside && !fake_enter_detected )
                        {
                            update<interior, exterior, '1', TransposeResult>(res);

                            // if it's the first IP then the first point is outside
                            if ( first_in_range )
                            {
                                bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                    range::front(sub_geometry::get(geometry, seg_id)),
                                                    boundary_checker);

                                // if there is a boundary on the first point
                                if ( front_b )
                                {
                                    update<boundary, exterior, '0', TransposeResult>(res);
                                }
                            }
                        }
                    }
                }
                // u/u, x/u
                else if ( op == overlay::operation_union || op == overlay::operation_blocked )
                {
                    bool op_blocked = op == overlay::operation_blocked;
                    bool was_outside = m_exit_watcher.is_outside();
                    m_exit_watcher.exit(it->point, other_id, op);

                    // we're inside, possibly going out right now
                    if ( ! was_outside )
                    {
                        if ( op_blocked )
                        {
                            // check if this is indeed the boundary point
                            // NOTE: is_ip_on_boundary<>() should be called here but the result will be the same
                            if ( is_endpoint_on_boundary<boundary_back>(it->point, boundary_checker) )
                            {
                                update<boundary, boundary, '0', TransposeResult>(res);
                            }
                        }
                    }
                    // we're outside
                    else
                    {
                        update<interior, exterior, '1', TransposeResult>(res);

                        bool this_b = is_ip_on_boundary<boundary_any>(it->point,
                                                                      it->operations[op_id],
                                                                      boundary_checker,
                                                                      seg_id);                        
                        // if current IP is on boundary of the geometry
                        if ( this_b )
                        {
                            update<boundary, boundary, '0', TransposeResult>(res);
                        }
                        // if current IP is not on boundary of the geometry
                        else
                        {
                            update<interior, boundary, '0', TransposeResult>(res);
                        }

                        // first IP on the last segment point - this means that the first point is outside
                        if ( first_in_range && ( !this_b || op_blocked ) )
                        {
                            bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                range::front(sub_geometry::get(geometry, seg_id)),
                                                boundary_checker);

                            // if there is a boundary on the first point
                            if ( front_b )
                            {
                                update<boundary, exterior, '0', TransposeResult>(res);
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
                    update<interior, exterior, '1', TransposeResult>(res);

                    BOOST_ASSERT(first != last);
                    BOOST_ASSERT(m_previous_turn_ptr);

                    segment_identifier const& prev_seg_id = m_previous_turn_ptr->operations[op_id].seg_id;

                    bool prev_back_b = is_endpoint_on_boundary<boundary_back>(
                                            range::back(sub_geometry::get(geometry, prev_seg_id)),
                                            boundary_checker);

                    // if there is a boundary on the last point
                    if ( prev_back_b )
                    {
                        update<boundary, exterior, '0', TransposeResult>(res);
                    }
                }
                // we might enter some Areal and didn't go out,
                else if ( m_previous_operation == overlay::operation_intersection )
                {
                    // just in case
                    update<interior, interior, '1', TransposeResult>(res);

                    BOOST_ASSERT(first != last);
                    BOOST_ASSERT(m_previous_turn_ptr);

                    segment_identifier const& prev_seg_id = m_previous_turn_ptr->operations[op_id].seg_id;

                    bool prev_back_b = is_endpoint_on_boundary<boundary_back>(
                                            range::back(sub_geometry::get(geometry, prev_seg_id)),
                                            boundary_checker);

                    // if there is a boundary on the last point
                    if ( prev_back_b )
                    {
                        update<boundary, interior, '0', TransposeResult>(res);
                    }
                }

                BOOST_ASSERT_MSG(m_previous_operation != overlay::operation_continue,
                                 "Unexpected operation! Probably the error in get_turns(L,A) or relate(L,A)");

                // Reset exit watcher before the analysis of the next Linestring
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
              typename BoundaryChecker>
    static inline void analyse_each_turn(Result & res,
                                         Analyser & analyser,
                                         TurnIt first, TurnIt last,
                                         Geometry const& geometry,
                                         OtherGeometry const& other_geometry,
                                         BoundaryChecker & boundary_checker)
    {
        if ( first == last )
            return;

        for ( TurnIt it = first ; it != last ; ++it )
        {
            analyser.apply(res, first, it, last,
                           geometry, other_geometry,
                           boundary_checker);

            if ( res.interrupt )
                return;
        }

        analyser.apply(res, first, last, last,
                       geometry, other_geometry,
                       boundary_checker);
    }
};

template <typename Geometry1, typename Geometry2>
struct areal_linear
{
    template <typename Result>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2, Result & result)
    {
        linear_areal<Geometry2, Geometry1, true>::apply(geometry2, geometry1, result);
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_AREAL_HPP
