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
        // TODO: This is O(N)
        // Run in a loop O(NM) - optimize!
        int pig = detail::within::point_in_geometry(pt, *m_other_areal_ptr);
        //BOOST_ASSERT( pig != 0 );
        
        // inside
        if ( pig > 0 )
        {
            update<interior, interior, '2', TransposeResult>(*m_result_ptr);
            update<boundary, interior, '1', TransposeResult>(*m_result_ptr);
            update<exterior, interior, '2', TransposeResult>(*m_result_ptr);
            m_flags |= 1;
        }
        // outside
        else
        {
            update<interior, exterior, '2', TransposeResult>(*m_result_ptr);
            update<boundary, exterior, '1', TransposeResult>(*m_result_ptr);
            m_flags |= 2;

            // If the exterior ring is outside, interior rings must be checked
            ring_identifier ring_id(0, -1, 0);
            for ( ; ring_id.ring_index < boost::numeric_cast<int>(geometry::num_interior_rings(areal)) ;
                    ++ring_id.ring_index )
            {
                typename detail::sub_range_return_type<Areal const>::type
                    range_ref = detail::sub_range(areal, ring_id);

                if ( boost::empty(range_ref) )
                {
                    // TODO: throw exception?
                    continue; // ignore
                }

                // TODO: O(N)
                // Optimize!
                int pig = detail::within::point_in_geometry(range::front(range_ref), *m_other_areal_ptr);

                // hole inside
                if ( pig > 0 )
                {
                    update<interior, interior, '2', TransposeResult>(*m_result_ptr);
                    update<boundary, interior, '1', TransposeResult>(*m_result_ptr);
                    update<exterior, interior, '2', TransposeResult>(*m_result_ptr);
                    m_flags |= 1;
                }
            }
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

            uncertain_rings_analyser<0, Result, Geometry1, Geometry2> rings_analyser(result, geometry1, geometry2);
            analyse_uncertain_rings<0>::apply(rings_analyser, turns.begin(), turns.end());

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

            uncertain_rings_analyser<1, Result, Geometry2, Geometry1> rings_analyser(result, geometry2, geometry1);
            analyse_uncertain_rings<1>::apply(rings_analyser, turns.begin(), turns.end());

            //if ( result.interrupt )
            //    return;
        }
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
            static const std::size_t other_op_id = (OpId + 1) % 2;
            static const bool transpose_result = OpId != 0;

            overlay::operation_type op = turn.operations[OpId].operation;

            if ( op == overlay::operation_union )
            {
                // ignore u/u
                /*if ( turn.operations[other_op_id].operation != overlay::operation_union )
                {
                    update<interior, exterior, '2', transpose_result>(m_result);
                    update<boundary, exterior, '1', transpose_result>(m_result);
                }*/

                update<boundary, boundary, '0', transpose_result>(m_result);
            }
            else if ( op == overlay::operation_intersection )
            {
                // ignore i/i
                if ( turn.operations[other_op_id].operation != overlay::operation_intersection )
                {
                    update<interior, interior, '2', transpose_result>(m_result);
                    //update<boundary, interior, '1', transpose_result>(m_result);
                }

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
            //segment_identifier const& other_id = it->operations[other_op_id].seg_id;

            const bool first_in_range = m_seg_watcher.update(seg_id);

            if ( m_previous_turn_ptr )
            {
                if ( m_exit_detected /*m_previous_operation == overlay::operation_union*/ )
                {
                    // real exit point - may be multiple
                    if ( first_in_range
                      || ! turn_on_the_same_ip<op_id>(*m_previous_turn_ptr, *it) )
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
                /*else*/
                if ( m_enter_detected /*m_previous_operation == overlay::operation_intersection*/ )
                {
                    // real entry point
                    if ( first_in_range
                      || op == overlay::operation_union // optimization
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
                // already set in interrupt policy
                //update<boundary, boundary, '0', transpose_result>(m_result);

                // ignore u/u
                //if ( it->operations[other_op_id].operation != overlay::operation_union )
                {
                    m_exit_detected = true;
                }
            }
            else if ( op == overlay::operation_intersection )
            {
                // ignore i/i
                if ( it->operations[other_op_id].operation != overlay::operation_intersection )
                {
                    // already set in interrupt policy
                    //update<interior, interior, '2', transpose_result>(result);
                    //update<boundary, boundary, '0', transpose_result>(result);
                    m_enter_detected = true;
                }
            }
            else if ( op == overlay::operation_blocked )
            {
                // already set in interrupt policy
            }
            else // if ( op == overlay::operation_continue )
            {
                // already set in interrupt policy
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
        segment_watcher<same_ring> m_seg_watcher;
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

    template <std::size_t OpId, typename Result, typename Geometry, typename OtherGeometry>
    class uncertain_rings_analyser
    {
        static const bool transpose_result = OpId != 0;
        static const int other_id = (OpId + 1) % 2;

        static const iterate_direction direction = order_as_direction<geometry::point_order<Geometry const>::value>::value;
        static const closure_selector closure = geometry::closure<Geometry const>::value;

        typedef typename ring_type<Geometry const>::type ring_type;
        typedef typename reversible_view<ring_type const, direction>::type reversible_type;
        typedef typename closeable_view<reversible_type const, closure>::type closeable_type;

    public:
        inline uncertain_rings_analyser(Result & result,
                                        Geometry const& geom,
                                        OtherGeometry const& other_geom)
            : geometry(geom), other_geometry(other_geom)
            , interrupt(result.interrupt) // just in case, could be false as well
            , m_result(result)
            , m_flags(0)
        {
            // TODO: initialize flags with the result

// TODO: WOULD IT BE POSSIBLE TO DISABLE SOME PARTS OF ANALYSIS DEPENDING ON WHAT IS THE STATE OF THE RESULT?
// E.G. IF I^I IS SET TO SOME MAX IT'S NOT REQUIRED TO ANALYSE THINGS THAT MAY SET THE I^I
        }

        inline void no_turns(segment_identifier const& seg_id)
        {
            // if those flags are set nothing will change
            if ( (m_flags & 3) == 3 )
            {
                return;
            }

            typename detail::sub_range_return_type<Geometry const>::type
                range_ref = detail::sub_range(geometry, seg_id);

            if ( boost::empty(range_ref) )
            {
                // TODO: throw an exception?
                return; // ignore
            }
                
            // TODO: possible optimization
            // if the range is an interior ring we may use other IPs generated for this single geometry
            // to know which other single geometries should be checked

            // TODO: optimize! e.g. use spatial index
            // O(N) - running it in a loop would gives O(NM)
            int pig = detail::within::point_in_geometry(range::front(range_ref), other_geometry);

            //BOOST_ASSERT(pig != 0);
            if ( pig > 0 )
            {
                update<boundary, interior, '1', transpose_result>(m_result);
                update<interior, interior, '2', transpose_result>(m_result);
                m_flags |= 1;
            }
            else
            {
                update<boundary, exterior, '1', transpose_result>(m_result);
                update<interior, exterior, '2', transpose_result>(m_result);
                m_flags |= 2;
            }

// TODO: break if all things are set
// also some of them could be checked outside, before the analysis
// In this case we shouldn't relay just on dummy flags
// Flags should be initialized with proper values
// or the result should be checked directly
// THIS IS ALSO TRUE FOR OTHER ANALYSERS! in L/L and L/A

            interrupt = m_flags == 7 || m_result.interrupt;
        }

        template <typename TurnIt>
        inline void turns(TurnIt first, TurnIt last)
        {
            // if those flags are set nothing will change
            if ( (m_flags & 6) == 6 )
            {
                return;
            }

            bool found_ii = false;
            bool found_uu = false;

            for ( TurnIt it = first ; it != last ; ++it )
            {
                if ( it->operations[0].operation == overlay::operation_intersection 
                  && it->operations[1].operation == overlay::operation_intersection )
                {
                    // ignore exterior ring
                    if ( it->operations[OpId].seg_id.ring_index >= 0 )
                    {
                        found_ii = true;
                    }
                }
                else if ( it->operations[0].operation == overlay::operation_union 
                       && it->operations[1].operation == overlay::operation_union )
                {
                    // ignore if u/u is for holes
                    //if ( it->operations[OpId].seg_id.ring_index >= 0
                    //  && it->operations[other_id].seg_id.ring_index >= 0 )
                    {
                        found_uu = true;
                    }
                }
                else // ignore
                {
                    return; // don't interrupt
                }
            }

            // only i/i was generated for this ring
            if ( found_ii )
            {
                //update<interior, interior, '0', transpose_result>(m_result);
                //update<boundary, boundary, '0', transpose_result>(m_result);
                update<boundary, interior, '1', transpose_result>(m_result);
                update<exterior, interior, '2', transpose_result>(m_result);
                m_flags |= 4;
            }

            // only u/u was generated for this ring
            if ( found_uu )
            {
                update<boundary, exterior, '1', transpose_result>(m_result);
                update<interior, exterior, '2', transpose_result>(m_result);
                m_flags |= 2;

                // not necessary since this will be checked in the next iteration
                // but increases the pruning strength
                // WARNING: this is not reflected in flags
                update<exterior, boundary, '1', transpose_result>(m_result);
                update<exterior, interior, '2', transpose_result>(m_result);
            }

            interrupt = m_flags == 7 || m_result.interrupt; // interrupt if the result won't be changed in the future
        }

//        template <typename TurnIt>
//        inline bool turns(TurnIt first, TurnIt last)
//        {
//            std::set<int> other_multi_indexes_ii;
//            bool found_uu = false;
//
//            for ( TurnIt it = first ; it != last ; ++it )
//            {
//                if ( it->operations[0].operation == overlay::operation_intersection 
//                  && it->operations[1].operation == overlay::operation_intersection )
//                {
//                    // ignore exterior ring
//                    if ( it->operations[OpId].seg_id.ring_index >= 0 )
//                    {
//                        other_multi_indexes_ii.insert(it->operations[other_id].seg_id.multi_index);
//                    }
//                }
//                else if ( it->operations[0].operation == overlay::operation_union 
//                       && it->operations[1].operation == overlay::operation_union )
//                {
//                    // ignore if u/u is for holes
//                    //if ( it->operations[OpId].seg_id.ring_index >= 0
//                    //  && it->operations[other_id].seg_id.ring_index >= 0 )
//                    {
//                        found_uu = true;
//                    }
//                }
//                else // ignore
//                {
//                    return true; // don't interrupt
//                }
//            }
//
//            // If we're here there was no other than i/i or u/u turns generated
//            // i/i generated for holes of current geometry are stored only
//
//            // for each i/i other index test if there is no hole inside current ring
//            if ( !other_multi_indexes_ii.empty() )
//            {
//                reversible_type rev_view(detail::sub_range(geometry, first->operations[OpId].seg_id));
//                closeable_type ring_view(rev_view);
//
//                // NOTE that it doesn't matter if the tested hole is partially contained
//                for ( std::set<int>::iterator it = other_multi_indexes_ii.begin() ;
//                      it != other_multi_indexes_ii.end() ; ++it )
//                {
//                    ring_identifier other_ring_id(other_id, multi_index, 0);
//
//                    typename detail::single_geometry_return_type<OtherGeometry const>::type
//                        other_single_ref = detail::single_geometry(other_geometry, other_ring_id);
//
//                    bool found_inside = false;
//
//                    // for each interior ring of other geometry
//                    for ( ; other_ring_id.ring_index < geometry::num_interior_rings(other_single_ref) ;
//                            ++other_ring_id.ring_index )
//                    {
//// TODO: not very optimal since single geometry must be indexed for each ring
//                        typename detail::sub_range_return_type<OtherGeometry const>::type
//                            other_range_ref = detail::sub_range(other_geometry, other_ring_id);
//
//                        if ( boost::empty(other_range_ref) )
//                        {
//                            continue;
//                        }
//
//                        typename point_type<OtherGeometry>::type const&
//                            pt = range::front(other_range_ref);
//
//                        int pig = detail::within::point_in_range<Geometry>(pt, ring_view);
//
//                        if ( pig > 0 )
//                        {
//                            found_inside = true;
//                            break;
//                        }
//                    }
//
//                    // nothing is inside the hole, ... WRONG!
//                    if ( !found_inside )
//                    {
//                        update<boundary, interior, '1', transpose_result>(m_result);
//                        update<exterior, interior, '2', transpose_result>(m_result);
//                    }
//                }
//            }
//
//            // if u/u were found this means that only u/u were generated for this ring
//            if ( found_uu )
//            {
//                update<boundary, exterior, '1', transpose_result>(m_result);
//                update<interior, exterior, '2', transpose_result>(m_result);
//                m_flags |= 2;
//
//                // not necessary since this will be checked in the next iteration
//                // but increases the pruning strength
//                // NOTE that this is not reflected in flags on purpose
//                update<exterior, boundary, '1', transpose_result>(m_result);
//                update<exterior, interior, '2', transpose_result>(m_result);
//            }
//
//            return m_flags != 3; // interrupt if the result won't be changed in the future
//        }

        Geometry const& geometry;
        OtherGeometry const& other_geometry;
        bool interrupt;

    private:
        Result & m_result;
        int m_flags;
    };

    template <std::size_t OpId>
    class analyse_uncertain_rings
    {
    public:
        template <typename Analyser, typename TurnIt>
        static inline void apply(Analyser & analyser, TurnIt first, TurnIt last)
        {
            if ( first == last )
                return;

            for_preceding_rings(analyser, *first);
            //analyser.per_turn(*first);

            TurnIt prev = first;
            for ( ++first ; first != last ; ++first, ++prev )
            {
                // same multi
                if ( prev->operations[OpId].seg_id.multi_index
                  == first->operations[OpId].seg_id.multi_index )
                {
                    // same ring
                    if ( prev->operations[OpId].seg_id.ring_index
                      == first->operations[OpId].seg_id.ring_index )
                    {
                        //analyser.per_turn(*first);
                    }
                    // same multi, next ring
                    else
                    {
                        //analyser.end_ring(*prev);
                        analyser.turns(prev, first);

                        //if ( prev->operations[OpId].seg_id.ring_index + 1
                        //   < first->operations[OpId].seg_id.ring_index)
                        {
                            for_no_turns_rings(analyser,
                                               *first,
                                               prev->operations[OpId].seg_id.ring_index + 1,
                                               first->operations[OpId].seg_id.ring_index);
                        }

                        //analyser.per_turn(*first);
                    }
                }
                // next multi
                else
                {
                    //analyser.end_ring(*prev);
                    analyser.turns(prev, first);
                    for_following_rings(analyser, *prev);
                    for_preceding_rings(analyser, *first);
                    //analyser.per_turn(*first);
                }

                if ( analyser.interrupt )
                {
                    return;
                }
            }

            //analyser.end_ring(*prev);
            analyser.turns(prev, first); // first == last
            for_following_rings(analyser, *prev);
        }

    private:
        template <typename Analyser, typename Turn>
        static inline void for_preceding_rings(Analyser & analyser, Turn const& turn)
        {
            for_no_turns_rings(analyser, turn, -1, turn.operations[OpId].seg_id.ring_index);
        }

        template <typename Analyser, typename Turn>
        static inline void for_following_rings(Analyser & analyser, Turn const& turn)
        {
            std::size_t count = geometry::num_interior_rings(
                                    detail::single_geometry(analyser.geometry,
                                                            turn.operations[OpId].seg_id));
            for_no_turns_rings(analyser, turn, turn.operations[OpId].seg_id.ring_index + 1, count);
        }

        template <typename Analyser, typename Turn>
        static inline void for_no_turns_rings(Analyser & analyser, Turn const& turn, int first, int last)
        {
            segment_identifier seg_id = turn.operations[OpId].seg_id;
            static const bool transpose_result = OpId != 0;

            for ( seg_id.ring_index = first ; seg_id.ring_index < last ; ++seg_id.ring_index )
            {
                analyser.no_turns(seg_id);
            }
        }
    };
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_AREAL_AREAL_HPP
