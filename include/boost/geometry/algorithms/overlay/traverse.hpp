// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_TRAVERSE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_TRAVERSE_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/algorithms/overlay/copy_segments.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#if defined(BOOST_GEOMETRY_DEBUG_INTERSECTION) || defined(BOOST_GEOMETRY_OVERLAY_REPORT_WKT)
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>
#endif


#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>


namespace boost { namespace geometry
{




#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {


template <typename Turns>
inline void clear_visit_info(Turns& turns)
{
    typedef typename boost::range_value<Turns>::type tp_type;

    for (typename boost::range_iterator<Turns>::type
        it = boost::begin(turns);
        it != boost::end(turns);
        ++it)
    {
        for (typename boost::range_iterator
            <
                typename tp_type::container_type
            >::type op_it = boost::begin(it->operations);
            op_it != boost::end(it->operations);
            ++op_it)
        {
            op_it->visited.set_none();
        }
    }
}


template <typename Info, typename Turn>
inline void set_visited_for_contine(Info& info, Turn const& turn)
{
    // On "continue", set "visited" for ALL directions
    if (turn.operation == detail::overlay::operation_continue)
    {
        for (typename boost::range_iterator
            <
                typename Info::container_type
            >::type it = boost::begin(info.operations);
            it != boost::end(info.operations);
            ++it)
        {
            if (it->visited.none())
            {
                it->visited.set_visited();
            }
        }
    }
}


template
<
    typename GeometryOut,
    typename G1,
    typename G2,
    typename Turns,
    typename IntersectionInfo
>
inline void assign_next_ip(G1 const& g1, G2 const& g2,
            Turns& turns,
            typename boost::range_iterator<Turns>::type & ip,
            GeometryOut& current_output,
            IntersectionInfo & info,
            segment_identifier& seg_id)
{
    info.visited.set_visited();
    set_visited_for_contine(*ip, info);

    // If there is no next IP on this segment
    if (info.enriched.next_ip_index < 0)
    {
        if (info.seg_id.source_index == 0)
        {
            geometry::copy_segments(g1, info.seg_id,
                    info.enriched.travels_to_vertex_index,
                    current_output);
        }
        else
        {
            geometry::copy_segments(g2, info.seg_id,
                    info.enriched.travels_to_vertex_index,
                    current_output);
        }
        ip = boost::begin(turns) + info.enriched.travels_to_ip_index;
        seg_id = info.seg_id;
    }
    else
    {
        ip = boost::begin(turns) + info.enriched.next_ip_index;
        seg_id = info.seg_id;
    }
    *(std::back_inserter(current_output)++) = ip->point;
}


inline bool select_source(operation_type operation, int source1, int source2)
{
    return operation == operation_intersection && source1 != source2;
}


template
<
    typename Turn,
    typename Iterator
>
inline bool select_next_ip(operation_type operation,
            Turn& turn,
            segment_identifier const& seg_id,
            Iterator const& start,
            Iterator& selected)
{
    bool has_tp = false;
    selected = boost::end(turn.operations);
    for (Iterator it = boost::begin(turn.operations);
        it != boost::end(turn.operations);
        ++it)
    {
        // In some cases there are two alternatives.
        // For "ii", take the other one (alternate)
        // For "uu", take the same one (see above);
        // For "cc", take either one, but if there is a starting one,
        //           take that one.
        if (   (it->operation == operation_continue
                && (! has_tp
                    || it->visited.started()
                    )
                )
            || (it->operation == operation
                && (! has_tp
                    || select_source(operation,
                            it->seg_id.source_index, seg_id.source_index)
                    )
                )
            )
        {
            selected = it;
            has_tp = true;
        }

        if (it->visited.started())
        {
            selected = it;
            return true;
        }
    }

    return has_tp;
}


template
<
    typename Rings,
    typename Turns,
    typename Geometry1,
    typename Geometry2
>
inline void backtrack(std::size_t size_at_start, bool& fail,
            Rings& rings, typename boost::range_value<Rings>::type& ring,
            Turns& turns, typename boost::range_value<Turns>::type& turn,
            std::string const& reason,
            Geometry1 const& geometry1,
            Geometry2 const& geometry2)
{
    fail = true;

    // Make bad output clean
    rings.resize(size_at_start);
    ring.clear();

    // Reject this as a starting point
    turn.rejected = true;

    // And clear all visit info
    clear_visit_info(turns);


#ifdef BOOST_GEOMETRY_OVERLAY_REPORT_WKT
    std::cout << " BT (" << reason << " )";
    std::cout
        << geometry::wkt(geometry1) << std::endl
        << geometry::wkt(geometry2) << std::endl;
#endif

}

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


/*!
    \brief Traverses through intersection points / geometries
    \ingroup overlay
 */
template
<
    typename Geometry1,
    typename Geometry2,
    typename Turns,
    typename Rings
>
inline void traverse(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            detail::overlay::operation_type operation,
            Turns& turns, Rings& rings)
{
    typedef typename boost::range_iterator<Turns>::type turn_iterator;
    typedef typename boost::range_value<Turns>::type turn_type;
    typedef typename boost::range_iterator
        <
            typename turn_type::container_type
        >::type turn_operation_iterator_type;

    std::size_t size_at_start = boost::size(rings);

    bool fail = false;
    do
    {
        fail = false;
        // Iterate through all unvisited points
        for (turn_iterator it = boost::begin(turns);
            ! fail && it != boost::end(turns);
            ++it)
        {
            // Skip the ones marked ignore (these are: "uu", so self-tangent)
            if (! it->ignore)
            {
                for (turn_operation_iterator_type iit = boost::begin(it->operations);
                    ! fail && ! it->rejected && iit != boost::end(it->operations);
                    ++iit)
                {
                    if (iit->visited.none()
                        && (iit->operation == operation
                            || iit->operation == detail::overlay::operation_continue)
                        )
                    {
                        set_visited_for_contine(*it, *iit);

                        typename boost::range_value<Rings>::type current_output;
                        *(std::back_inserter(current_output)++) = it->point;

                        turn_iterator current = it;
                        turn_operation_iterator_type current_iit = iit;
                        segment_identifier current_seg_id;

                        detail::overlay::assign_next_ip(geometry1, geometry2,
                                    turns,
                                    current, current_output,
                                    *iit, current_seg_id);

                        if (! detail::overlay::select_next_ip(
                                        operation,
                                        *current,
                                        current_seg_id,
                                        boost::end(it->operations),
                                        current_iit))
                        {
                            detail::overlay::backtrack(
                                size_at_start, fail,
                                rings, current_output, turns, *it,
                                "Dead end at start",
                                geometry1, geometry2);
                        }
                        else
                        {

                            iit->visited.set_started();

                            unsigned int i = 0;

                            while (current_iit != iit && ! fail)
                            {
                                if (current_iit->visited.visited())
                                {
                                    // It visits a visited node again, without passing the start node.
                                    // This makes it suspicious for endless loops
                                    detail::overlay::backtrack(
                                        size_at_start, fail,
                                        rings,  current_output, turns, *it,
                                        "Visit again",
                                        geometry1, geometry2);
                                }
                                else
                                {


                                    // We assume clockwise polygons only, non self-intersecting, closed.
                                    // However, the input might be different, and checking validity
                                    // is up to the library user.

                                    // Therefore we make here some sanity checks. If the input
                                    // violates the assumptions, the output polygon will not be correct
                                    // but the routine will stop and output the current polygon, and
                                    // will continue with the next one.

                                    // Below three reasons to stop.
                                    assign_next_ip(geometry1, geometry2,
                                        turns, current, current_output,
                                        *current_iit, current_seg_id);

                                    if (! detail::overlay::select_next_ip(
                                                operation,
                                                *current,
                                                current_seg_id,
                                                iit,
                                                current_iit))
                                    {
                                        // Should not occur in valid (non-self-intersecting) polygons
                                        // Should not occur in self-intersecting polygons without spikes
                                        // Might occur in polygons with spikes
                                        detail::overlay::backtrack(
                                            size_at_start, fail,
                                            rings,  current_output, turns, *it,
                                            "Dead end",
                                            geometry1, geometry2);
                                    }

                                    if (i++ > turns.size())
                                    {
                                        // Sanity check: there may be never more loops
                                        // than intersection points.
                                        detail::overlay::backtrack(
                                            size_at_start, fail,
                                            rings,  current_output, turns, *it,
                                            "Endless loop",
                                            geometry1, geometry2);
                                    }
                                }
                            }

                            if (! fail)
                            {
                                iit->visited.set_finished();
                                rings.push_back(current_output);
                            }
                        }
                    }
                }
            }
        }
    } while (fail);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_TRAVERSE_HPP
