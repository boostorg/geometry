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

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>
#endif


#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {


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
    typename TurnPoints,
    typename IntersectionInfo
>
inline void assign_next_ip(G1 const& g1, G2 const& g2,
            TurnPoints& turn_points,
            typename boost::range_iterator<TurnPoints>::type & ip,
            GeometryOut& current_output,
            IntersectionInfo & info,
            segment_identifier& seg_id)
{
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    //std::cout << "traverse: take: " << info << std::endl;
#endif
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
        ip = boost::begin(turn_points) + info.enriched.travels_to_ip_index;
        seg_id = info.seg_id;
    }
    else
    {
        ip = boost::begin(turn_points) + info.enriched.next_ip_index;
        seg_id = info.seg_id;
    }
    *(std::back_inserter(current_output)++) = ip->point;
}


inline bool select_source(operation_type operation, int source1, int source2)
{
    return
        (operation == operation_intersection && source1 != source2)
        || (operation == operation_union && source1 == source2);
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
        // For "uu", take the same one
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
            has_tp = true;
            return true;
        }

    }

    return has_tp;
}


template <typename Container>
inline void stop_gracefully(Container& container, bool& stop,
            std::string const& reason)
{
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    std::cout << "STOPPING: " << reason << std::endl;
#endif

    stop = true;
    if (container.size() > 0)
    {
        // Two-step adding first value, without assignment
        // and without (erroneous) push_back of reference
        typename boost::range_value<Container>::type p;
        geometry::copy_coordinates(container.front(), p);
        *(std::back_inserter(container)++) = p;

    }
}

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


/*!
    \brief Traverses through intersection points / geometries
    \ingroup overlay
 */
template
<
    typename SideStrategy,
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename TurnPoints,
    typename OutputIterator
>
inline void traverse(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            detail::overlay::operation_type operation,
            TurnPoints& turn_points,
            OutputIterator out)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    typedef typename boost::range_iterator<TurnPoints>::type turn_iterator;
    typedef typename boost::range_value<TurnPoints>::type turn_type;
    typedef typename boost::range_iterator
        <
            typename turn_type::container_type
        >::type turn_operation_iterator_type;


    // Iterate through all unvisited points
    for (turn_iterator it = boost::begin(turn_points);
        it != boost::end(turn_points);
        ++it)
    {
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
        //std::cout << "traversal: try: " << *it;
#endif

        for (turn_operation_iterator_type iit = boost::begin(it->operations);
            iit != boost::end(it->operations);
            ++iit)
        {
            if (iit->visited.none()
                && (iit->operation == operation
                    || iit->operation == detail::overlay::operation_continue)
                )
            {
                set_visited_for_contine(*it, *iit);

                GeometryOut current_output;
                *(std::back_inserter(current_output)++) = it->point;

                turn_iterator current = it;
                turn_operation_iterator_type current_iit = iit;
                segment_identifier current_seg_id;

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
                //std::cout << "traversal: start: " << *current;
#endif

                detail::overlay::assign_next_ip(geometry1, geometry2,
                            turn_points,
                            current, current_output,
                            *iit, current_seg_id);
                detail::overlay::select_next_ip(
                                operation,
                                *current,
                                current_seg_id,
                                boost::end(it->operations),
                                current_iit);

                iit->visited.set_started();


                unsigned int i = 0;
                bool stop = false;

                while (current_iit != iit && ! stop)
                {

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
                    //std::cout << "traverse: " << *current;
#endif

                    if (current_iit->visited.visited())
                    {
                        // It visits a visited node again, without passing the start node.
                        // This makes it suspicious for endless loops
                        // Check if it is really same node
                        detail::overlay::stop_gracefully(
                            current_output, stop, "Visit again");
                    }


                    // We assume clockwise polygons only, non self-intersecting, closed.
                    // However, the input might be different, and checking validity
                    // is up to the library user.

                    // Therefore we make here some sanity checks. If the input
                    // violates the assumptions, the output polygon will not be correct
                    // but the routine will stop and output the current polygon, and
                    // will continue with the next one.

                    // Below three reasons to stop.
                    assign_next_ip(geometry1, geometry2,
                        turn_points, current, current_output,
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
                        detail::overlay::stop_gracefully(
                            current_output, stop, "Dead end");
                    }

                    if (i++ > turn_points.size())
                    {
                        // Sanity check: there may be never more loops
                        // than intersection points.
                        detail::overlay::stop_gracefully(
                            current_output, stop, "Endless loop");
                    }
                }

                iit->visited.set_finished();

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
                //std::cout << "traversal: finish: " << *current;
                std::cout << geometry::wkt(current_output) << std::endl;
#endif

                *out++ = current_output;
            }
        }
    }
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    std::cout << "traversal: all IP's handled" << std::endl;
#endif
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_TRAVERSE_HPP
