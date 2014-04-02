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
                    
        return m_flags != 3 && !m_result_ptr->interrupt;
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

        interrupt_policy_areal_areal<Result> interrupt_policy(geometry1, geometry2, result);

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

        {
            typedef turns::less_seg_dist_op<0,1,2,3,4,0, 0,
                                turns::less_ignore_other> less;

            // u, i, x, c
            std::sort(turns.begin(), turns.end(), less());

            turns_analyser<turn_type, 0> analyser;
            analyse_each_turn(result, analyser,
                              turns.begin(), turns.end(),
                              geometry1, geometry2);

            if ( result.interrupt )
                return;
        }

        {
            typedef turns::less_seg_dist_op<0,1,2,3,4,0, 1,
                                turns::less_ignore_other> less;

            // u, i, x, c
            std::sort(turns.begin(), turns.end(), less());

            turns_analyser<turn_type, 1> analyser;
            analyse_each_turn(result, analyser,
                              turns.begin(), turns.end(),
                              geometry2, geometry1);

            if ( result.interrupt )
                return;
        }

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
    template <typename Result>
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
                per_turn<0>(*it);
                per_turn<1>(*it);
            }

            return m_result.interrupt;
        }

    private:
        template <std::size_t OpId, typename Turn>
        inline void per_turn(Turn const& turn)
        {
            static const bool transpose_result = OpId != 0;

            overlay::operation_type op = turn.operations[OpId].operation;

            if ( op == overlay::operation_union )
            {
                //update<interior, exterior, '2', transpose_result>(m_result);
                //update<boundary, exterior, '1', transpose_result>(m_result);
                update<boundary, boundary, '0', transpose_result>(m_result);
            }
            else if ( op == overlay::operation_intersection )
            {
                update<interior, interior, '2', transpose_result>(m_result);
                //update<boundary, interior, '1', transpose_result>(m_result);
                update<boundary, boundary, '0', transpose_result>(m_result);
            }
            else if ( op == overlay::operation_continue )
            {
                update<boundary, boundary, '1', transpose_result>(m_result);
                update<interior, interior, '2', transpose_result>(m_result);
            }
            else if ( op == overlay::operation_blocked )
            {
                update<boundary, boundary, '1', transpose_result>(m_result);
                update<interior, exterior, '2', transpose_result>(m_result);
            }
        }

        Result & m_result;
        Geometry1 const& m_geometry1;
        Geometry2 const& m_geometry2;
    };

    // This analyser should be used like Input or SinglePass Iterator
    // IMPORTANT! It should be called also for the end iterator - last
    template <typename TurnInfo, std::size_t OpId>
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
            , m_enter_detected(false)
            , m_exit_detected(false)
        {}

        template <typename Result,
                  typename TurnIt,
                  typename Geometry,
                  typename OtherGeometry>
        void apply(Result & result,
                   TurnIt first, TurnIt it, TurnIt last,
                   Geometry const& geometry,
                   OtherGeometry const& other_geometry)
        {
            //BOOST_ASSERT( it != last );

            overlay::operation_type op = it->operations[op_id].operation;

            if ( op != overlay::operation_union
              && op != overlay::operation_intersection
              && op != overlay::operation_blocked
              && op != overlay::operation_continue )
            {
                return;
            }

            segment_identifier const& seg_id = it->operations[op_id].seg_id;
            segment_identifier const& other_id = it->operations[other_op_id].seg_id;

            const bool first_in_range = m_seg_watcher.update(seg_id);

            // TODO

            if ( m_previous_turn_ptr )
            {
                if ( m_exit_detected /*m_previous_operation == overlay::operation_union*/ )
                {
                    // real exit point - may be multiple
                    if ( ! turn_on_the_same_ip<op_id>(*m_previous_turn_ptr, *it) )
                    {
                        update_exit(result);
                        m_exit_detected = false;
                    }
                    // fake exit point, reset state
                    else if ( op != overlay::operation_union )
                    {
                        m_exit_detected = false;
                    }
                }
                else if ( m_enter_detected /*m_previous_operation == overlay::operation_intersection*/ )
                {
                    // real entry point
                    if ( op == overlay::operation_union // optimization
                      || ! turn_on_the_same_ip<op_id>(*m_previous_turn_ptr, *it) )
                    {
                        update_enter(result);
                        m_enter_detected = false;
                    }
                    // fake exit point, reset state
                    else if ( op == overlay::operation_continue
                           || op == overlay::operation_blocked )
                    {
                        m_enter_detected = false;
                    }
                }
            }

            if ( op == overlay::operation_union )
            {
                //update<boundary, boundary, '0', transpose_result>(m_result);
                m_exit_detected = true;
            }
            else if ( op == overlay::operation_intersection )
            {
                //update<interior, interior, '2', transpose_result>(result);
                //update<boundary, boundary, '0', transpose_result>(result);
                m_enter_detected = true;
            }
            else if ( op == overlay::operation_blocked )
            {
                // TODO
            }
            else // if ( op == overlay::operation_continue )
            {
                // TODO
            }

            // store ref to previously analysed (valid) turn
            m_previous_turn_ptr = boost::addressof(*it);
            // and previously analysed (valid) operation
            m_previous_operation = op;
        }

        // it == last
        template <typename Result,
                  typename TurnIt,
                  typename Geometry,
                  typename OtherGeometry>
        void apply(Result & result,
                   TurnIt first, TurnIt last,
                   Geometry const& geometry,
                   OtherGeometry const& other_geometry)
        {
            //BOOST_ASSERT( first != last );

            if ( m_exit_detected /*m_previous_operation == overlay::operation_union*/ )
            {
                update_exit(result);
                m_exit_detected = false;
            }

            if ( m_enter_detected /*m_previous_operation == overlay::operation_intersection*/ )
            {
                update_enter(result);
                m_enter_detected = false;
            }
        }

        template <typename Result>
        static inline void update_exit(Result & result)
        {
            update<interior, exterior, '2', transpose_result>(result);
            update<boundary, exterior, '1', transpose_result>(result);
        }

        template <typename Result>
        static inline void update_enter(Result & result)
        {
            update<boundary, interior, '1', transpose_result>(result);
            update<exterior, interior, '2', transpose_result>(result);
        }

    private:
        segment_watcher m_seg_watcher;
        TurnInfo * m_previous_turn_ptr;
        overlay::operation_type m_previous_operation;
        bool m_enter_detected;
        bool m_exit_detected;
    };

    // call analyser.apply() for each turn in range
    // IMPORTANT! The analyser is also called for the end iterator - last
    template <typename Result,
              typename TurnIt,
              typename Analyser,
              typename Geometry,
              typename OtherGeometry>
    static inline void analyse_each_turn(Result & res,
                                         Analyser & analyser,
                                         TurnIt first, TurnIt last,
                                         Geometry const& geometry,
                                         OtherGeometry const& other_geometry)
    {
        if ( first == last )
            return;

        for ( TurnIt it = first ; it != last ; ++it )
        {
            analyser.apply(res, first, it, last,
                           geometry, other_geometry);

            if ( res.interrupt )
                return;
        }

        analyser.apply(res, first, last,
                       geometry, other_geometry);
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
