// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/append_no_duplicates.hpp>
#include <boost/geometry/algorithms/detail/overlay/copy_segments.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{



/*!
\brief Follows a linestring from intersection point to intersection point, outputting which
    is inside, or outside, a ring or polygon
\ingroup overlay
 */
template
<
    typename LineStringOut,
    typename LineString,
    typename Polygon
>
class follow
{

    template<typename Turn>
    struct sort_on_segment
    {
        inline bool operator()(Turn const& left, Turn const& right) const
        {
            segment_identifier const& sl = left.operations[0].seg_id;
            segment_identifier const& sr = right.operations[0].seg_id;

            return sl == sr
                ? left.operations[0].enriched.distance < right.operations[0].enriched.distance
                : sl < sr;

        }
    };

    template <typename Turn, typename Operation>
    static inline bool is_entering(Turn const& turn, Operation const& op)
    {
        return op.operation == operation_intersection;
    }

    template <typename Turn, typename Operation>
    static inline bool is_leaving(Turn const& turn, Operation const& op, 
                    bool entered, bool first, 
                    LineString const& linestring, Polygon const& polygon)
    {
        if (op.operation == operation_union)
        {
            if (turn.method == method_crosses)
            {
                return true;
            }

            if (entered)
            {
                return true;
            }
            if (first)
            {
                // Check if first point of line is inside polygon
                return geometry::within(linestring[0], polygon);
            }
        }
        return false;
    }


    template <typename Turn, typename Operation>
    static inline bool is_staying_inside(Turn const& turn, Operation const& op, 
                    bool entered, bool first, 
                    LineString const& linestring, Polygon const& polygon)
    {
        if (turn.method == method_crosses)
        {
            return false;
        }

        if (op.operation == operation_intersection)
        {
            if (entered)
            {
                return true;
            }
            if (first)
            {
                // Check if first point of line is inside polygon
                return geometry::within(linestring[0], polygon);
            }
        }
        return false;
    }


public :
    template<typename Turns, typename OutputIterator>
    static inline OutputIterator apply(LineString const& linestring, Polygon const& polygon,
                detail::overlay::operation_type operation,
                Turns& turns, OutputIterator out)
    {
        typedef typename boost::range_iterator<Turns>::type turn_iterator;
        typedef typename boost::range_value<Turns>::type turn_type;
        typedef typename boost::range_iterator
            <
                typename turn_type::container_type
            >::type turn_operation_iterator_type;


        // Sort intersection points on segments-along-linestring, and distance
        // (like in enrich is done for poly/poly)
        std::sort(boost::begin(turns), boost::end(turns), sort_on_segment<turn_type>());

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        // Iterate through all intersection points (they are ordered along the line)
        bool entered = false;
        bool first = true;
        for (turn_iterator it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            turn_operation_iterator_type iit = boost::begin(it->operations);

            if (is_staying_inside(*it, *iit, entered, first, linestring, polygon))
            {
                debug_traverse(*it, *iit, "-> Staying inside");

                entered = true;
            }
            else if (is_entering(*it, *iit))
            {
                debug_traverse(*it, *iit, "-> Entering");

                entered = true;
                detail::overlay::append_no_duplicates(current_piece, it->point);
                current_segment_id = iit->seg_id;
            }
            else if (is_leaving(*it, *iit, entered, first, linestring, polygon))
            {
                debug_traverse(*it, *iit, "-> Leaving");

                entered = false;
                geometry::copy_segments<false>(linestring, current_segment_id,
                        iit->seg_id.segment_index,
                        current_piece);
                detail::overlay::append_no_duplicates(current_piece, it->point);

                if (! current_piece.empty())
                {
                    *out++ = current_piece;
                    current_piece.clear();
                }
            }
            first = false;
        }

        if (entered)
        {
            geometry::copy_segments<false>(linestring, current_segment_id, 
                    boost::size(linestring) - 1,
                    current_piece);
        }

        // Add the last one, if applicable
        if (! current_piece.empty())
        {
            *out++ = current_piece;
        }
        return out;
    }
};


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_HPP
