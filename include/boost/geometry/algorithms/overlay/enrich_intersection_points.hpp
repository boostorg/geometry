// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ENRICH_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ENRICH_HPP


#include <cstddef>
#include <algorithm>
#include <map>
#include <vector>

//#ifdef BOOST_GEOMETRY_DEBUG_OVERLAY
#include <iostream>
#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
//#endif

#include <boost/assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/overlay/copy_segment_point.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_relative_order.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename Operation>
struct indexed_operation
{
    typedef Operation type;

    int index;
    int operation_index;
    Operation subject;

    inline indexed_operation(int i, int oi, Operation const& s)
        : index(i)
        , operation_index(oi)
        , subject(s)
    {}
};


template
<
    typename Indexed,
    typename Geometry1,
    typename Geometry2,
    typename Strategy
>
struct sort_on_distance
{
private :
    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Strategy const& m_strategy;

    typedef typename Indexed::type operation_type;
    typedef typename point_type<Geometry1>::type point_type;


    inline bool normal_compare_distances(operation_type const& left,
                    operation_type const& right) const
    {
        return left.enriched.distance < right.enriched.distance;
    }

protected :


    // Note that left/right do NOT correspond to m_geometry1/m_geometry2
    inline bool compare_distances(operation_type const& left,
                    operation_type const& right) const
    {
        // TODO: change this, it gives non-consequent behaviour resulting in
        // an assertion within sort.
        // SOLUTION: first sort on distance, then check if the sorted vector
        // has distances close to zero, swap them mutually if there is a reason
        // (reason by sides)
        //if (fuzzy_equals(left.enriched.distance, right.enriched.distance))
        if (false)
        {

            // Distances are the same, or quite close.
            // We sort now using sides instead of using distance
            // In most cases, this is possible. Else we fallback to distance
            // (--> very rare cases, when both methods fail)

#ifdef BOOST_GEOMETRY_DEBUG_OVERLAY
            std::cout << "Equal Distance"
                << " : "    << left.seg_id  << " / "  << left.other_id
                << " and "  << right.seg_id << " / " << right.other_id
                << std::endl;
#endif

            point_type pi, pj, ri, rj, si, sj;
            if (left.seg_id == right.seg_id)
            {
                geometry::copy_segment_points(m_geometry1, m_geometry2,
                    left.seg_id,
                    pi, pj);
                geometry::copy_segment_points(m_geometry1, m_geometry2,
                    left.other_id,
                    ri, rj);
                geometry::copy_segment_points(m_geometry1, m_geometry2,
                    right.other_id,
                    si, sj);
            }
            else if (left.other_id == right.other_id)
            {
                geometry::copy_segment_points(m_geometry1, m_geometry2,
                    left.other_id,
                    pi, pj);
                geometry::copy_segment_points(m_geometry1, m_geometry2,
                    left.seg_id,
                    ri, rj);
                geometry::copy_segment_points(m_geometry1, m_geometry2,
                    right.seg_id,
                    si, sj);
            }
            else
            {
                return normal_compare_distances(left, right);
            }

            int const order = get_relative_order<point_type>::apply(pi, pj,
                            ri, rj, si, sj);
            if (order != 0)
            {
                // If order == -1, r is the first segment along p
                // So then return true;
                return order == -1;
            }
        }

        return normal_compare_distances(left, right);
    }

public :
    sort_on_distance(Geometry1 const& geometry1, Geometry2 const& geometry2,
                Strategy const& strategy)
        : m_geometry1(geometry1)
        , m_geometry2(geometry2)
        , m_strategy(strategy)
    {}

    inline bool operator()(Indexed const& left, Indexed const& right) const
    {
        return compare_distances(left.subject, right.subject);
    }
};


template
<
    typename Indexed,
    typename Geometry1,
    typename Geometry2,
    typename Strategy
>
struct sort_on_segment_and_distance
        : public sort_on_distance
            <
                Indexed,
                Geometry1, Geometry2, Strategy
            >
{
    sort_on_segment_and_distance(Geometry1 const& geometry1,
                Geometry2 const& geometry2,
                Strategy const& strategy)
        : sort_on_distance
            <
                Indexed,
                Geometry1, Geometry2, Strategy
            >(geometry1, geometry2, strategy)
    {}

    inline bool operator()(Indexed const& left, Indexed const& right) const
    {
        segment_identifier const& sl = left.subject.seg_id;
        segment_identifier const& sr = right.subject.seg_id;

        return sl == sr
            ? compare_distances(left.subject, right.subject)
            : sl < sr;
    }
};


template <typename Point, typename Operation, typename Geometry1, typename Geometry2>
inline bool swap_operations(Operation const& left, Operation const& right,
        Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    Point pi, pj, ri, rj, si, sj;
    if (left.seg_id == right.seg_id)
    {
        geometry::copy_segment_points(geometry1, geometry2,
            left.seg_id,
            pi, pj);
        geometry::copy_segment_points(geometry1, geometry2,
            left.other_id,
            ri, rj);
        geometry::copy_segment_points(geometry1, geometry2,
            right.other_id,
            si, sj);
        std::cout << "Considering seg"  << std::endl;
    }
    else if (left.other_id == right.other_id)
    {
        geometry::copy_segment_points(geometry1, geometry2,
            left.other_id,
            pi, pj);
        geometry::copy_segment_points(geometry1, geometry2,
            left.seg_id,
            ri, rj);
        geometry::copy_segment_points(geometry1, geometry2,
            right.seg_id,
            si, sj);
        std::cout << "Considering other"  << std::endl;
    }
    else
    {
        return false;
    }

    int const order = get_relative_order<Point>::apply(pi, pj,
                    ri, rj, si, sj);
    std::cout << "Order: "  << order << std::endl;
    return order == 1;
}


// Sorts IP-s of this ring on segment-identifier, and if on same segment,
//  on distance.
// Then assigns for each IP which is the next IP on this segment,
// plus the vertex-index to travel to, plus the next IP
// (might be on another segment)
template
<
    typename IndexType,
    typename Container,
    typename TurnPoints,
    typename Geometry1,
    typename Geometry2,
    typename Strategy
>
static inline bool enrich(Container& operations,
            TurnPoints& turn_points,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            Strategy const& strategy)
{
    std::sort(boost::begin(operations),
                boost::end(operations),
                sort_on_segment_and_distance
                    <
                        IndexType,
                        Geometry1, Geometry2,
                        Strategy
                    >(geometry1, geometry2, strategy));


    typedef typename IndexType::type operation_type;
    typedef typename boost::range_iterator<Container const>::type iterator_type;

//#ifdef BOOST_GEOMETRY_DEBUG_OVERLAY
    /***
    // Check if it is really sorted.
    if (operations.size() > 1)
    {
        typedef typename boost::range_iterator<Container>::type nc_iterator;
        nc_iterator it = boost::begin(operations);
        for (nc_iterator prev = it++;
            it != boost::end(operations);
            prev = it++)
        {
            operation_type& prev_op = turn_points[prev->index]
                .operations[prev->operation_index];
            operation_type& op = turn_points[it->index]
                .operations[it->operation_index];
           if ((prev_op.seg_id == op.seg_id || prev_op.other_id == op.other_id)
               && geometry::math::equals(prev_op.enriched.distance,
                        op.enriched.distance))
           {
                std::cout << "Equal Distance on "
                    << " : "    << prev_op.seg_id  << " / "  << prev_op.other_id
                    << " and "  << op.seg_id << " / " << op.other_id
                    << std::endl;
                std::cout << "\tType of intersections: "
                    << operation_char(prev_op.operation)
                    << " , " << operation_char(op.operation)
                    << std::endl;

                if (swap_operations
                    <
                        typename point_type<Geometry1>::type
                    >(prev_op, op, geometry1, geometry2))
                {
                    std::cout << "Should be swapped" << std::endl;

                    std::swap(*prev, *it);
                }
           }
        }
    }
    ***/
//#endif


    // Assign travel-to-vertex/ip index for each turning point.
    // Because IP's are circular, PREV starts at the very last one,
    // being assigned from the first one.
    // For "next ip on same segment" it should not be considered circular.
    bool first = true;
    iterator_type it = boost::begin(operations);
    for (iterator_type prev = it + (boost::size(operations) - 1);
         it != boost::end(operations);
         prev = it++)
    {
        operation_type& prev_op = turn_points[prev->index]
            .operations[prev->operation_index];

        prev_op.enriched.travels_to_ip_index = it->index;
        prev_op.enriched.travels_to_vertex_index
                    = it->subject.seg_id.segment_index;

        operation_type& op = turn_points[it->index]
            .operations[it->operation_index];

        if (! first && prev_op.seg_id.segment_index == op.seg_id.segment_index)
        {
            prev_op.enriched.next_ip_index = it->index;
        }
        first = false;
    }

    return true;
}


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


/*!
    \brief All intersection points are enriched with successor information
    \ingroup overlay
    \tparam TurnPoints type of intersection container
                (e.g. vector of "intersection/turn point"'s)
    \param turn_points container containing intersectionpoints
 */
template
<
    typename TurnPoints,
    typename Geometry1,
    typename Geometry2,
    typename Strategy
>
inline void enrich_intersection_points(TurnPoints& turn_points,
    Geometry1 const& geometry1, Geometry2 const& geometry2,
    Strategy const& strategy)
{
    typedef typename boost::range_value<TurnPoints>::type turn_point_type;
    typedef typename turn_point_type::container_type container_type;
    typedef typename boost::range_value<container_type>::type operation_type;
    typedef detail::overlay::indexed_operation<operation_type> indexed_type;
    typedef std::map
        <
            ring_identifier,
            std::vector<indexed_type>
        > mapped_vector_type;

    // Create a map of vectors of indexed operation-types to be able
    // to sort per ring, later on.
    mapped_vector_type mapped_vector;

    int index = 0;
    for (typename boost::range_iterator<TurnPoints const>::type
            it = boost::begin(turn_points);
         it != boost::end(turn_points);
         ++it, ++index)
    {
        if (! it->ignore)
        {
            int op_index = 0;
            for (typename boost::range_iterator<container_type const>::type
                    op_it = boost::begin(it->operations);
                op_it != boost::end(it->operations);
                ++op_it, ++op_index)
            {
                ring_identifier ring_id
                    (
                        op_it->seg_id.source_index,
                        op_it->seg_id.multi_index,
                        op_it->seg_id.ring_index
                    );
                mapped_vector[ring_id].push_back
                    (
                        indexed_type(index, op_index, *op_it)
                    );
            }
        }
    }

    // Note: no const-operator because contents of mapped copy is temporary,
    // and changed by enrich)
    for (typename mapped_vector_type::iterator mit
        = mapped_vector.begin();
        mit != mapped_vector.end();
        ++mit)
    {
        detail::overlay::enrich<indexed_type>(mit->second, turn_points,
                    geometry1, geometry2, strategy);
    }
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ENRICH_HPP
