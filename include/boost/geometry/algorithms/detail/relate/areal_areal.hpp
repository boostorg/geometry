// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_AREAL_AREAL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_AREAL_AREAL_HPP

#include <boost/geometry/core/topological_dimension.hpp>
#include <boost/geometry/util/range.hpp>

#include <boost/geometry/algorithms/detail/sub_range.hpp>
#include <boost/geometry/algorithms/detail/single_geometry.hpp>

#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/turns.hpp>
#include <boost/geometry/algorithms/detail/relate/boundary_checker.hpp>
#include <boost/geometry/algorithms/detail/relate/follow_helpers.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {
    
// WARNING!
// TODO: In the worst case calling this Pred in a loop for MultiPolygon/MultiPolygon may take O(NM)
// Use the rtree in this case!

// may be used to set EI and EB for an Areal geometry for which no turns were generated
template <typename OtherAreal, typename Result, bool TransposeResult>
class no_turns_aa_pred
{
public:
    no_turns_aa_pred(OtherAreal const& other_areal, Result & res)
        : m_result_ptr(boost::addressof(res))
        , m_other_areal_ptr(boost::addressof(other_areal))
        , m_flags(0)
    {}

    template <typename Areal>
    bool operator()(Areal const& areal)
    {
        typedef typename geometry::point_type<Areal>::type point_type;
        point_type pt;
        bool ok = boost::geometry::point_on_border(pt, areal);

        // TODO: for now ignore, later throw an exception?
        if ( !ok )
            return true;

        // check if the areal is inside the other_areal
        int pig = detail::within::point_in_geometry(pt, *m_other_areal_ptr);
        //BOOST_ASSERT( pig != 0 );
        
        // inside
        if ( pig > 0 )
        {
            update<interior, interior, '2', TransposeResult>(*m_result_ptr);
            update<boundary, interior, '1', TransposeResult>(*m_result_ptr);
            m_flags |= 1;
        }
        // outside
        else
        {
            update<interior, exterior, '2', TransposeResult>(*m_result_ptr);
            update<boundary, exterior, '1', TransposeResult>(*m_result_ptr);
            m_flags |= 2;
        }
                    
        return m_flags != 3;
    }

private:
    Result * m_result_ptr;
    const OtherAreal * m_other_areal_ptr;
    int m_flags;
};

// The implementation of an algorithm calculating relate() for A/A
template <typename Geometry1, typename Geometry2>
struct areal_areal
{
    // check Linear / Areal
    BOOST_STATIC_ASSERT(topological_dimension<Geometry1>::value == 2
                     && topological_dimension<Geometry2>::value == 2);

    static const bool interruption_enabled = true;

    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;
    
    template <typename Result>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2, Result & result)
    {
// TODO: If Areal geometry may have infinite size, change the following line:

        // The result should be FFFFFFFFF
        set<exterior, exterior, result_dimension<Geometry2>::value>(result);// FFFFFFFFd, d in [1,9] or T

        if ( result.interrupt )
            return;

        // get and analyse turns
        typedef typename turns::get_turns<Geometry1, Geometry2>::turn_info turn_type;
        typedef typename std::vector<turn_type>::iterator turn_iterator;
        std::vector<turn_type> turns;

        interrupt_policy_areal_areal<Geometry1, Geometry2, Result> interrupt_policy(geometry1, geometry2, result);

        turns::get_turns<Geometry1, Geometry2>::apply(turns, geometry1, geometry2, interrupt_policy);
        if ( result.interrupt )
            return;

        no_turns_aa_pred<Geometry2, Result, false> pred1(geometry2, result);
        for_each_disjoint_geometry_if<0, Geometry1>::apply(turns.begin(), turns.end(), geometry1, pred1);
        if ( result.interrupt )
            return;

        no_turns_aa_pred<Geometry1, Result, true> pred2(geometry1, result);
        for_each_disjoint_geometry_if<1, Geometry2>::apply(turns.begin(), turns.end(), geometry2, pred2);
        if ( result.interrupt )
            return;
        
        if ( turns.empty() )
            return;

//
//        {
//            // for different multi or same ring id: x, u, i, c
//            // for same multi and different ring id: c, i, u, x
//            std::sort(turns.begin(), turns.end(), turns::less_seg_dist_op<0,2,3,1,4,0,0>());
//
//            turns_analyser<turn_type> analyser;
//            analyse_each_turn(result, analyser,
//                              turns.begin(), turns.end(),
//                              geometry1, geometry2,
//                              boundary_checker1);
//
//            if ( result.interrupt )
//                return;
//        }
//
//// TODO: CALCULATE THE FOLLOWING ONLY IF IT'S REQUIRED BY THE RESULT!
////       AND ONLY IF IT WAS NOT SET BY THE no_turns_la_areal_pred
//
//        if ( !interrupt_policy.is_boundary_found )
//        {
//            set<exterior, boundary, '1', TransposeResult>(result);
//        }
//        else
//        {
//            // sort by multi_index and rind_index
//            std::sort(turns.begin(), turns.end(), less_ring());
//
//            typedef typename std::vector<turn_type>::iterator turn_iterator;
//
//            turn_iterator it = turns.begin();
//            segment_identifier * prev_seg_id_ptr = NULL;
//            // for each ring
//            for ( ; it != turns.end() ; )
//            {
//                // it's the next single geometry
//                if ( prev_seg_id_ptr == NULL
//                  || prev_seg_id_ptr->multi_index != it->operations[1].seg_id.multi_index )
//                {
//                    // if the first ring has no IPs
//                    if ( it->operations[1].seg_id.ring_index > -1 )
//                    {
//                        // we can be sure that the exterior overlaps the boundary
//                        set<exterior, boundary, '1', TransposeResult>(result);                    
//                        break;
//                    }
//                    // if there was some previous ring
//                    if ( prev_seg_id_ptr != NULL )
//                    {
//                        int const next_ring_index = prev_seg_id_ptr->ring_index + 1;
//                        BOOST_ASSERT(next_ring_index >= 0);
//                        
//                        // if one of the last rings of previous single geometry was ommited
//                        if ( static_cast<std::size_t>(next_ring_index)
//                                < geometry::num_interior_rings(
//                                    single_geometry(geometry2, *prev_seg_id_ptr)) )
//                        {
//                            // we can be sure that the exterior overlaps the boundary
//                            set<exterior, boundary, '1', TransposeResult>(result);                    
//                        }
//                    }
//                }
//                // if it's the same single geometry
//                else /*if ( previous_multi_index == it->operations[1].seg_id.multi_index )*/
//                {
//                    // and we jumped over one of the rings
//                    if ( prev_seg_id_ptr != NULL // just in case
//                      && prev_seg_id_ptr->ring_index + 1 < it->operations[1].seg_id.ring_index )
//                    {
//                        // we can be sure that the exterior overlaps the boundary
//                        set<exterior, boundary, '1', TransposeResult>(result);                    
//                        break;
//                    }
//                }
//
//                prev_seg_id_ptr = boost::addressof(it->operations[1].seg_id);
//
//                // find the next ring first iterator and check if the analysis should be performed
//                has_boundary_intersection has_boundary_inters;
//                turn_iterator next = find_next_ring(it, turns.end(), has_boundary_inters);
//
//                // if there is no 1d overlap with the boundary
//                if ( !has_boundary_inters.result )
//                {
//                    // we can be sure that the exterior overlaps the boundary
//                    set<exterior, boundary, '1', TransposeResult>(result);                    
//                    break;
//                }
//                // else there is 1d overlap with the boundary so we must analyse the boundary
//                else
//                {
//                    // u, c
//                    std::sort(it, next, turns::less_seg_dist_op<0,1,0,0,2,0,1>());
//
//                    // analyse
//                    areal_boundary_analyser<turn_type> analyser;
//                    for ( turn_iterator rit = it ; rit != next ; ++rit )
//                    {
//                        // if the analyser requests, break the search
//                        if ( !analyser.apply(it, rit, next) )
//                            break;
//                    }
//
//                    // if the boundary of Areal goes out of the Linear
//                    if ( analyser.is_union_detected )
//                    {
//                        // we can be sure that the boundary of Areal overlaps the exterior of Linear
//                        set<exterior, boundary, '1', TransposeResult>(result);
//                        break;
//                    }
//                }
//
//                it = next;
//            }
//
//            // if there was some previous ring
//            if ( prev_seg_id_ptr != NULL )
//            {
//                int const next_ring_index = prev_seg_id_ptr->ring_index + 1;
//                BOOST_ASSERT(next_ring_index >= 0);
//
//                // if one of the last rings of previous single geometry was ommited
//                if ( static_cast<std::size_t>(next_ring_index)
//                        < geometry::num_interior_rings(
//                            single_geometry(geometry2, *prev_seg_id_ptr)) )
//                {
//                    // we can be sure that the exterior overlaps the boundary
//                    set<exterior, boundary, '1', TransposeResult>(result);                    
//                }
//            }
//        }
    }

    // interrupt policy which may be passed to get_turns to interrupt the analysis
    // based on the info in the passed result/mask
    template <typename Geometry1, typename Geometry2, typename Result>
    class interrupt_policy_areal_areal
    {
    public:
        static bool const enabled = true;

        interrupt_policy_areal_areal(Geometry1 const& geometry1,
                                     Geometry1 const& geometry2,
                                     Result & result)
            : m_result(result)
            , m_geometry1(geometry1)
            , m_geometry2(geometry2)
        {}

// TODO: since we update result for some operations here, we may not do it in the analyser!

        template <typename Range>
        inline bool apply(Range const& turns)
        {
            typedef typename boost::range_iterator<Range const>::type iterator;
            
            for (iterator it = boost::begin(turns) ; it != boost::end(turns) ; ++it)
            {
                per_turn<0, false>(*it);
                per_turn<1, true>(*it);
            }

            return m_result.interrupt;
        }

    private:
        template <std::size_t Id, bool TransposeResult, typename Turn>
        inline void per_turn(Turn const& turn)
        {
// THIS WON'T WORK FOR NON-SIMPLE GEOMETRIES!

            overlay::operation_type op = turn.operations[Id].operation;

            if ( op == overlay::operation_intersection )
            {
                update<interior, interior, '2', TransposeResult>(m_result);
                update<boundary, interior, '1', TransposeResult>(m_result);
                update<boundary, boundary, '0', TransposeResult>(m_result);
            }
            else if ( op == overlay::operation_continue ||
                      op == overlay::operation_blocked )
            {
                update<boundary, boundary, '1', TransposeResult>(m_result);
            }
            else if ( op == overlay::operation_union )
            {
                update<boundary, boundary, '0', TransposeResult>(m_result);
                update<boundary, exterior, '1', TransposeResult>(m_result);
                update<interior, exterior, '2', TransposeResult>(m_result);
            }
        }

        Result & m_result;
        Geometry1 const& m_geometry1;
        Geometry2 const& m_geometry2;
    };

    // This analyser should be used like Input or SinglePass Iterator
    // IMPORTANT! It should be called also for the end iterator - last
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
            , m_boundary_counter(0)
            , m_interior_detected(false)
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
                   BoundaryChecker const& boundary_checker)
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

// NOTE: THE WHOLE m_interior_detected HANDLING IS HERE BECAUSE WE CAN'T EFFICIENTLY SORT TURNS (CORRECTLY)
// BECAUSE THE SAME IP MAY BE REPRESENTED BY TWO SEGMENTS WITH DIFFERENT DISTANCES
// IT WOULD REQUIRE THE CALCULATION OF MAX DISTANCE
// TODO: WE COULD GET RID OF THE TEST IF THE DISTANCES WERE NORMALIZED

// TODO: THIS IS POTENTIALLY ERROREOUS!
// THIS ALGORITHM DEPENDS ON SOME SPECIFIC SEQUENCE OF OPERATIONS
// IT WOULD GIVE WRONG RESULTS E.G.
// IN THE CASE OF SELF-TOUCHING POINT WHEN 'i' WOULD BE BEFORE 'u' 

                // handle the interior overlap
                if ( m_interior_detected )
                {
                    // real interior overlap
                    if ( !detail::equals::equals_point_point(it->point, m_previous_turn_ptr->point) )
                    {
                        update<interior, interior, '1', TransposeResult>(res);
                        m_interior_detected = false;
                    }
                    // fake interior overlap
                    else if ( op == overlay::operation_continue )
                    {
                        m_interior_detected = false;
                    }
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
                    bool no_enters_detected = m_exit_watcher.is_outside();
                    m_exit_watcher.enter(it->point, other_id);

                    if ( op == overlay::operation_intersection )
                    {
                        if ( m_boundary_counter > 0 && it->operations[op_id].is_collinear )
                            --m_boundary_counter;

                        if ( m_boundary_counter == 0 )
                        {
                            // interiors overlaps
                            //update<interior, interior, '1', TransposeResult>(res);

                            // don't update now
                            // we might enter a boundary of some other ring on the same IP
                            m_interior_detected = true;
                        }
                    }
                    else // operation_boundary
                    {
                        // don't add to the count for all met boundaries
                        // only if this is the "new" boundary
                        if ( first_in_range || !it->operations[op_id].is_collinear )
                            ++m_boundary_counter;

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
                        if ( no_enters_detected
                          && !fake_enter_detected
                          && it->operations[op_id].position != overlay::position_front )
                        {
// TODO: calculate_from_inside() is only needed if the current Linestring is not closed
                            bool from_inside = first_in_range
                                               && calculate_from_inside(geometry,
                                                                        other_geometry,
                                                                        *it);

                            if ( from_inside )
                                update<interior, interior, '1', TransposeResult>(res);
                            else
                                update<interior, exterior, '1', TransposeResult>(res);

                            // if it's the first IP then the first point is outside
                            if ( first_in_range )
                            {
                                bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                    range::front(sub_range(geometry, seg_id)),
                                                    boundary_checker);

                                // if there is a boundary on the first point
                                if ( front_b )
                                {
                                    if ( from_inside )
                                        update<boundary, interior, '0', TransposeResult>(res);
                                    else
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
                    bool no_enters_detected = m_exit_watcher.is_outside();
                    
                    if ( op == overlay::operation_union )
                    {
                        if ( m_boundary_counter > 0 && it->operations[op_id].is_collinear )
                            --m_boundary_counter;
                    }
                    else // overlay::operation_blocked
                    {
                        m_boundary_counter = 0;
                    }

                    // we're inside, possibly going out right now
                    if ( ! no_enters_detected )
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
                        // union, inside, but no exit -> collinear on self-intersection point
                        // not needed since we're already inside the boundary
                        /*else if ( !exit_detected )
                        {
                            update<interior, boundary, '0', TransposeResult>(res);
                        }*/
                    }
                    // we're outside or inside and this is the first turn
                    else
                    {
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

                        // TODO: very similar code is used in the handling of intersection
                        if ( it->operations[op_id].position != overlay::position_front )
                        {
// TODO: calculate_from_inside() is only needed if the current Linestring is not closed
                            bool first_from_inside = first_in_range
                                                  && calculate_from_inside(geometry,
                                                                           other_geometry,
                                                                           *it);
                            if ( first_from_inside )
                            {
                                update<interior, interior, '1', TransposeResult>(res);

                                // notify the exit_watcher that we started inside
                                m_exit_watcher.enter(it->point, other_id);
                            }
                            else
                            {
                                update<interior, exterior, '1', TransposeResult>(res);
                            }

                            // first IP on the last segment point - this means that the first point is outside or inside
                            if ( first_in_range && ( !this_b || op_blocked ) )
                            {
                                bool front_b = is_endpoint_on_boundary<boundary_front>(
                                                    range::front(sub_range(geometry, seg_id)),
                                                    boundary_checker);

                                // if there is a boundary on the first point
                                if ( front_b )
                                {
                                    if ( first_from_inside )
                                        update<boundary, interior, '0', TransposeResult>(res);
                                    else
                                        update<boundary, exterior, '0', TransposeResult>(res);
                                }
                            }
                        }
                    }

                    // if we're going along a boundary, we exit only if the linestring was collinear
                    if ( m_boundary_counter == 0
                      || it->operations[op_id].is_collinear )
                    {
                        // notify the exit watcher about the possible exit
                        m_exit_watcher.exit(it->point, other_id, op);
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
                                            range::back(sub_range(geometry, prev_seg_id)),
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
                                            range::back(sub_range(geometry, prev_seg_id)),
                                            boundary_checker);

                    // if there is a boundary on the last point
                    if ( prev_back_b )
                    {
                        update<boundary, interior, '0', TransposeResult>(res);
                    }
                }

                // handle the interior overlap
                if ( m_interior_detected )
                {
                    // just in case
                    update<interior, interior, '1', TransposeResult>(res);
                    m_interior_detected = false;
                }

                BOOST_ASSERT_MSG(m_previous_operation != overlay::operation_continue,
                                 "Unexpected operation! Probably the error in get_turns(L,A) or relate(L,A)");

                // Reset exit watcher before the analysis of the next Linestring
                m_exit_watcher.reset();
                m_boundary_counter = 0;
            }
        }

        // check if the passed turn's segment of Linear geometry arrived
        // from the inside or the outside of the Areal geometry
        template <typename Turn>
        static inline bool calculate_from_inside(Geometry1 const& geometry1,
                                                 Geometry2 const& geometry2,
                                                 Turn const& turn)
        {
            if ( turn.operations[op_id].position == overlay::position_front )
                return false;

            static const bool reverse2 = detail::overlay::do_reverse<
                                            geometry::point_order<Geometry2>::value
                                         >::value;

            typedef typename closeable_view
                <
                    typename range_type<Geometry2>::type const,
                    closure<Geometry2>::value
                >::type range2_cview;

            typedef typename reversible_view
                <
                    range2_cview const,
                    reverse2 ? iterate_reverse : iterate_forward
                >::type range2_view;

            typedef typename sub_range_return_type<Geometry1 const>::type range1_ref;

            range1_ref range1 = sub_range(geometry1, turn.operations[op_id].seg_id);
            range2_cview const cview(sub_range(geometry2, turn.operations[other_op_id].seg_id));
            range2_view const range2(cview);

            std::size_t s1 = boost::size(range1);
            std::size_t s2 = boost::size(range2);
            BOOST_ASSERT(s1 > 1 && s2 > 2);
            std::size_t seg_count2 = s2 - 1;

            std::size_t p_seg_ij = turn.operations[op_id].seg_id.segment_index;
            std::size_t q_seg_ij = turn.operations[other_op_id].seg_id.segment_index;

            BOOST_ASSERT(p_seg_ij + 1 < s1 && q_seg_ij + 1 < s2);

            point1_type const& pi = range::at(range1, p_seg_ij);
            point2_type const& qi = range::at(range2, q_seg_ij);
            point2_type const& qj = range::at(range2, q_seg_ij + 1);
            point1_type qi_conv;
            geometry::convert(qi, qi_conv);
            bool is_ip_qj = equals::equals_point_point(turn.point, qj);
// TODO: test this!
//            BOOST_ASSERT(!equals::equals_point_point(turn.point, pi));
//            BOOST_ASSERT(!equals::equals_point_point(turn.point, qi));
            point1_type new_pj;
            geometry::convert(turn.point, new_pj);

            if ( is_ip_qj )
            {
                std::size_t q_seg_jk = (q_seg_ij + 1) % seg_count2;
// TODO: the following function should return immediately, however the worst case complexity is O(N)
// It would be good to replace it with some O(1) mechanism
                typename boost::range_iterator<range2_view>::type
                    qk_it = find_next_non_duplicated(boost::begin(range2),
                                                     boost::begin(range2) + q_seg_jk,
                                                     boost::end(range2));

                // Will this sequence of points be always correct?
                overlay::side_calculator<point1_type, point2_type> side_calc(qi_conv, new_pj, pi, qi, qj, *qk_it);

                return calculate_from_inside_sides(side_calc);
            }
            else
            {
                point1_type new_qj;
                geometry::convert(turn.point, new_qj);

                overlay::side_calculator<point1_type, point2_type> side_calc(qi_conv, new_pj, pi, qi, new_qj, qj);

                return calculate_from_inside_sides(side_calc);
            }
        }

        template <typename It>
        static inline It find_next_non_duplicated(It first, It current, It last)
        {
            BOOST_ASSERT( current != last );

            It it = current;

            for ( ++it ; it != last ; ++it )
            {
                if ( !equals::equals_point_point(*current, *it) )
                    return it;
            }

            // if not found start from the beginning
            for ( it = first ; it != current ; ++it )
            {
                if ( !equals::equals_point_point(*current, *it) )
                    return it;
            }

            return current;
        }

        // calculate inside or outside based on side_calc
        // this is simplified version of a check from equal<>
        template <typename SideCalc>
        static inline bool calculate_from_inside_sides(SideCalc const& side_calc)
        {
            int const side_pk_p = side_calc.pk_wrt_p1();
            int const side_qk_p = side_calc.qk_wrt_p1();
            // If they turn to same side (not opposite sides)
            if (! overlay::base_turn_handler::opposite(side_pk_p, side_qk_p))
            {
                int const side_pk_q2 = side_calc.pk_wrt_q2();
                return side_pk_q2 == -1;
            }
            else
            {
                return side_pk_p == -1;
            }
        }

    private:
        exit_watcher<turn_point_type> m_exit_watcher;
        segment_watcher m_seg_watcher;
        TurnInfo * m_previous_turn_ptr;
        overlay::operation_type m_previous_operation;
        unsigned m_boundary_counter;
        bool m_interior_detected;
    };

    // call analyser.apply() for each turn in range
    // IMPORTANT! The analyser is also called for the end iterator - last
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
                                         BoundaryChecker const& boundary_checker)
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

    // less comparator comparing multi_index then ring_index
    // may be used to sort turns by ring
    struct less_ring
    {
        template <typename Turn>
        inline bool operator()(Turn const& left, Turn const& right) const
        {
            return left.operations[1].seg_id.multi_index < right.operations[1].seg_id.multi_index
                || ( left.operations[1].seg_id.multi_index == right.operations[1].seg_id.multi_index
                  && left.operations[1].seg_id.ring_index < right.operations[1].seg_id.ring_index );
        }
    };

    // policy/functor checking if a turn's operation is operation_continue
    struct has_boundary_intersection
    {
        has_boundary_intersection()
            : result(false) {}

        template <typename Turn>
        inline void operator()(Turn const& turn)
        {
            if ( turn.operations[1].operation == overlay::operation_continue )
                result = true;
        }

        bool result;
    };

    // iterate through the range and search for the different multi_index or ring_index
    // also call fun for each turn in the current range
    template <typename It, typename Fun>
    static inline It find_next_ring(It first, It last, Fun & fun)
    {
        if ( first == last )
            return last;

        int const multi_index = first->operations[1].seg_id.multi_index;
        int const ring_index = first->operations[1].seg_id.ring_index;

        fun(*first);
        ++first;

        for ( ; first != last ; ++first )
        {
            if ( multi_index != first->operations[1].seg_id.multi_index
              || ring_index != first->operations[1].seg_id.ring_index )
            {
                return first;
            }

            fun(*first);
        }

        return last;
    }

    // analyser which called for turns sorted by seg/distance/operation
    // checks if the boundary of Areal geometry really got out
    // into the exterior of Linear geometry
    template <typename TurnInfo>
    class areal_boundary_analyser
    {
    public:
        areal_boundary_analyser()
            : is_union_detected(false)
            , m_previous_turn_ptr(NULL)
        {}

        template <typename TurnIt>
        bool apply(TurnIt /*first*/, TurnIt it, TurnIt last)
        {
            overlay::operation_type op = it->operations[1].operation;

            if ( it != last )
            {
                if ( op != overlay::operation_union
                  && op != overlay::operation_continue )
                {
                    return true;
                }

                if ( is_union_detected )
                {
                    BOOST_ASSERT(m_previous_turn_ptr != NULL);
                    if ( !detail::equals::equals_point_point(it->point, m_previous_turn_ptr->point) )
                    {
                        // break
                        return false;
                    }
                    else if ( op == overlay::operation_continue ) // operation_boundary
                    {
                        is_union_detected = false;
                    }
                }

                if ( op == overlay::operation_union )
                {
                    is_union_detected = true;
                    m_previous_turn_ptr = boost::addressof(*it);
                }

                return true;
            }
            else
            {
                return false;
            }            
        }

        bool is_union_detected;

    private:
        const TurnInfo * m_previous_turn_ptr;
    };
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_AREAL_AREAL_HPP
