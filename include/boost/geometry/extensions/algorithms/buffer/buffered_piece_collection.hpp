// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFERED_PIECE_COLLECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFERED_PIECE_COLLECTION_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/extensions/strategies/buffer_side.hpp>

#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>

#include <boost/geometry/extensions/algorithms/buffer/buffered_ring.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


class backtrack_for_buffer
{
public :
    typedef detail::overlay::backtrack_state state_type;

    template <typename Operation, typename Rings, typename Turns, typename Geometry>
    static inline void apply(std::size_t size_at_start, 
                Rings& rings, typename boost::range_value<Rings>::type& ring,
                Turns& turns, Operation& operation,
                std::string const& reason,
                Geometry const& ,
                Geometry const& ,
                state_type& state
                )
    {
std::cout << "WARNING " << reason << std::endl;

        // TODO this is a copy of dissolve, check this for buffer
        state.m_good = false;
        
        // Make bad output clean
        rings.resize(size_at_start);
        ring.clear();

        // Reject this as a starting point
        operation.visited.set_rejected();

        // And clear all visit info
        clear_visit_info(turns);
    }
};

enum intersection_location_type
{
    location_ok, inside_buffer, inside_original
};


// Shoould follow traversal-turn-concept (enrichment, visit structure)
// and adds index in piece vector to find it back
template <typename Point>
struct buffer_turn_operation : public detail::overlay::traversal_turn_operation<Point>
{
    int piece_index;

    inline buffer_turn_operation()
        : piece_index(-1)
    {}
};

// Add an "intersection_location_type" which gets true for all intersection points lying inside the buffer or original polygon
template <typename Point>
struct buffer_turn_info : public detail::overlay::turn_info<Point, buffer_turn_operation<Point> >
{
    intersection_location_type location;

    inline buffer_turn_info()
        : location(location_ok)
    {}
};

// In the end this will go (if we have a multi-point within/covered_by geometry)
// which is optimized for multi-points and skips linestrings
template <typename tag>
struct check_original
{
};

template <>
struct check_original<polygon_tag>
{
    template <typename Point, typename Geometry>
    static inline bool apply(Point const& point, Geometry const& geometry)
    {
        return geometry::covered_by(point, geometry);
    }
};

template <>
struct check_original<linestring_tag>
{
    template <typename Point, typename Geometry>
    static inline bool apply(Point const& point, Geometry const& geometry)
    {
        return false;
    }
};

template <typename Ring>
struct buffered_piece_collection
{
    typedef typename geometry::point_type<Ring>::type Point;
    typedef typename strategy::side::services::default_strategy<typename cs_tag<Point>::type>::type side_strategy;

    enum piece_type
    {
        buffered_segment, buffered_join
    };

    template <typename Turn>
    struct redundant_turn
    {
        inline bool operator()(Turn const& turn) const
        {
            return turn.location != location_ok;
        }
    };

    struct piece
    {
        piece_type type;
        int index;
        segment_identifier seg_id;

        typedef geometry::model::linestring<Point> buffered_vector_type;

        // These both form a complete clockwise ring for each piece (with one dupped point)
        buffered_vector_type offsetted_segment;
        buffered_vector_type helper_segments; // 3 for segment, 2 for join - might be empty too
    };

    typedef std::vector<piece> piece_vector;
    piece_vector all_pieces;
    buffered_ring_collection<buffered_ring<Ring> > offsetted_rings;
    buffered_ring_collection<Ring> traversed_rings;
    segment_identifier current_segment_id;

    typedef std::vector<buffer_turn_info<Point> > turn_vector_type;
    typedef detail::overlay::get_turn_info
        <
            Point, Point, buffer_turn_info<Point>,
            detail::overlay::calculate_distance_policy
        > turn_policy;
    turn_vector_type turn_vector;



    inline buffered_piece_collection()
    {}


    inline bool within_piece(buffer_turn_info<Point> const& turn, piece const& pc) const
    {
        bool const collinear =
            turn.operations[0].operation == detail::overlay::operation_continue
            && turn.operations[0].operation == detail::overlay::operation_continue;

        // TODO factor out the two loops

        typedef typename boost::range_iterator
            <
                typename piece::buffered_vector_type const
            >::type iterator_type;

        if (boost::size(pc.helper_segments) > 0)
        {
            iterator_type it = boost::begin(pc.helper_segments);
            for (iterator_type prev = it++;
                it != boost::end(pc.helper_segments);
                prev = it++)
            {
                int side = side_strategy::apply(turn.point, *prev, *it);
                switch(side)
                {
                    case 1 : return false;
                    case 0 : return true;
                }
            }
        }

        if (boost::size(pc.offsetted_segment) > 0)
        {
            iterator_type it = boost::begin(pc.offsetted_segment);
            for (iterator_type prev = it++;
                it != boost::end(pc.offsetted_segment);
                prev = it++)
            {
                int side = side_strategy::apply(turn.point, *prev, *it);
                switch(side)
                {
                    case 1 : return false;
                    case 0 : return !collinear;
                }
            }
        }

        return true;
    }

    // Checks if an intersection point is within one of all pieces
    // (but skip the pieces of which this intersection point is the result
    inline bool is_wrong(buffer_turn_info<Point> const& turn, piece const& piece1, piece const& piece2) const
    {
        // We might do this using partition.
        typename std::vector<piece>::const_iterator it;
        for (it = boost::begin(all_pieces);
            it != boost::end(all_pieces);
            ++it)
        {
            if (it->index != piece1.index && it->index != piece2.index)
            {
                if (within_piece(turn, *it))
                {
                    return true;
                }
            }
        }

        return false;

    }

    inline bool is_neighbor(piece const& piece1, piece const& piece2) const
    {
        if (std::abs(piece1.index - piece2.index) == 1)
        {
            return true;
        }

        int const b = boost::size(all_pieces) - 1; // back
        return (piece1.index == 0 && piece2.index == b)
            || (piece1.index == b && piece2.index == 0)
            ;
    }

    inline bool skip_neighbor(piece const& piece1, piece const& piece2) const
    {
        return piece1.type != piece2.type && is_neighbor(piece1, piece2);
    }

    template <typename Iterator>
    inline Point next_point(piece const& piece, Iterator it) const
    {
        // Next point in current offseted:
        Iterator next = it;
        ++next;
        if (next != boost::end(piece.offsetted_segment))
        {
            return *next;
        }

        // TODO: check if there is a second point (there should be one)
        // Second point from next piece:
        int next_index = piece.index + 1;
        if (next_index >= boost::size(all_pieces))
        {
            next_index = 0;
        }
        return piece.offsetted_segment[1];
    }

    inline void calculate_turns(piece const& piece1, piece const& piece2)
    {
        buffer_turn_info<Point> the_model;
        the_model.operations[0].piece_index = piece1.index;
        the_model.operations[0].seg_id = piece1.seg_id;

        // TODO use partition
        typedef typename boost::range_iterator<typename piece::buffered_vector_type const>::type iterator;
        iterator it1 = boost::begin(piece1.offsetted_segment);
        for (iterator prev1 = it1++; 
                it1 != boost::end(piece1.offsetted_segment); 
                prev1 = it1++, the_model.operations[0].seg_id.segment_index++)
        {
            the_model.operations[1].piece_index = piece2.index;
            the_model.operations[1].seg_id = piece2.seg_id;

            iterator it2 = boost::begin(piece2.offsetted_segment);
            for (iterator prev2 = it2++; 
                    it2 != boost::end(piece2.offsetted_segment); 
                    prev2 = it2++, the_model.operations[1].seg_id.segment_index++)
            {
                // Revert (this is used more often - should be common function TODO)
                the_model.operations[0].other_id = the_model.operations[1].seg_id;
                the_model.operations[1].other_id = the_model.operations[0].seg_id;


                turn_vector_type turns; // TEMPORARY in the end we can add directly to turn_vector
                turn_policy::apply(*prev1, *it1, next_point(piece1, it1),
                                    *prev2, *it2, next_point(piece2, it2),
                                    the_model, std::back_inserter(turns));

                // Check if it is inside any of the pieces
                for (typename boost::range_iterator<turn_vector_type>::type it =
                    boost::begin(turns); it != boost::end(turns); ++it)
                {
                    // TODO: much later in partition.
                    if (is_wrong(*it, piece1, piece2))
                    {
                        it->location = inside_buffer;
                    }
                    turn_vector.push_back(*it);
                }
            }
        }
    }

    template <typename Geometry>
    inline void check_remaining_points(Geometry const& input_geometry)
    {
        // TODO: this should be done as a collection-of-points, for performance
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(turn_vector); it != boost::end(turn_vector); ++it)
        {
            if (it->location == location_ok
                && check_original
                        <
                            typename geometry::tag<Geometry>::type
                        >::apply(it->point, input_geometry))
            {
                it->location = inside_original;
            }
        }
    }


    template <typename Geometry>
    inline void get_turns(Geometry const& input_geometry)
    {
        for(typename piece_vector::const_iterator it1 = boost::begin(all_pieces);
            it1 != boost::end(all_pieces);
            ++it1)
        {
            for(typename piece_vector::const_iterator it2 = it1 + 1;
                it2 != boost::end(all_pieces);
                ++it2)
            {
                if (! skip_neighbor(*it1, *it2))
                {
                    calculate_turns(*it1, *it2);
                }
            }
        }
        if (boost::is_same<typename tag_cast<typename tag<Geometry>::type, areal_tag>::type, areal_tag>())
        {
            check_remaining_points(input_geometry);
        }

        calculate_discarded();
    }

    inline void add_input()
    {
        int const n = offsetted_rings.size();
        current_segment_id.source_index = 0;
        current_segment_id.multi_index = n;
        current_segment_id.ring_index = -1;
        current_segment_id.segment_index = 0;

        offsetted_rings.resize(n + 1);
    }

    inline void add_point(Point const& p)
    {
        BOOST_ASSERT
            (
                boost::size(offsetted_rings) > 0
            );

        current_segment_id.segment_index++;
        offsetted_rings.back().push_back(p);
    }


    inline piece& add_piece(piece_type type, bool decrease_by_one)
    {
        piece pc;
        pc.type = type;
        pc.index = boost::size(all_pieces);
        pc.seg_id = current_segment_id;

        std::size_t const n = boost::size(offsetted_rings.back());

        pc.seg_id.segment_index = decrease_by_one ? n - 1 : n;

        all_pieces.push_back(pc);
        return all_pieces.back();
    }


    inline void add_piece(Point const& p1, Point const& p2, 
            Point const& b1, Point const& b2)
    {
        bool const last_type_join = ! all_pieces.empty() 
                && all_pieces.back().type != buffered_segment;

        piece& pc = add_piece(buffered_segment, last_type_join);

        // If it follows the same piece-type point both should be added.
        // There should be two intersections later and it should be discarded.
        // But for need it to calculate intersections
        if (! last_type_join)
        {
            add_point(b1);
        }
        add_point(b2);

        pc.offsetted_segment.push_back(b1);
        pc.offsetted_segment.push_back(b2);
        pc.helper_segments.push_back(b2);
        pc.helper_segments.push_back(p2);
        pc.helper_segments.push_back(p1);
        pc.helper_segments.push_back(b1);
    }

    template <typename Range>
    inline piece& add_piece(Range const& range)
    {
        piece& pc = add_piece(buffered_join, true);

        bool first = true;
        for (typename Range::const_iterator it = boost::begin(range);
            it != boost::end(range);
            ++it)
        {
            bool add = true;
            if (first)
            {
                // Only for very first one, add first. In all other cases it is shared with previous.
                add = offsetted_rings.back().empty();
                first = false;
            }
            if (add)
            {
                add_point(*it);
            }

            pc.offsetted_segment.push_back(*it);
        }
        return pc;
    }

    template <typename Range>
    inline void add_piece(Point const& p, Range const& range)
    {
        piece& pc = add_piece(range);

        if (boost::size(range) > 0)
        {
            pc.helper_segments.push_back(range.back());
            pc.helper_segments.push_back(p);
            pc.helper_segments.push_back(range.front());
        }
    }

    inline void enrich()
    {
        typedef typename strategy::side::services::default_strategy
        <
            typename cs_tag<Ring>::type
        >::type side_strategy_type;

        enrich_intersection_points<false, false>(turn_vector,
                    detail::overlay::operation_union,
                    offsetted_rings, offsetted_rings,
                    side_strategy_type());
    }

    // Discards all rings which do have not-OK intersection points only.
    // Those can never be traversed and should not be part of the output.
    inline void calculate_discarded()
    {
        for (typename boost::range_iterator<turn_vector_type const>::type it =
            boost::begin(turn_vector); it != boost::end(turn_vector); ++it)
        {
            if (it->location != location_ok)
            {
                offsetted_rings[it->operations[0].seg_id.multi_index].has_discarded_intersections = true;
                offsetted_rings[it->operations[1].seg_id.multi_index].has_discarded_intersections = true;
            }
            else
            {
                offsetted_rings[it->operations[0].seg_id.multi_index].has_accepted_intersections = true;
                offsetted_rings[it->operations[1].seg_id.multi_index].has_accepted_intersections = true;
            }
        }
    }
                    
    inline void discard_points()
    {
        // Erase all points being inside
        turn_vector.erase(
            std::remove_if(boost::begin(turn_vector), boost::end(turn_vector),
            redundant_turn<buffer_turn_info<Point> >()),
            boost::end(turn_vector));

    }

    inline void traverse()
    {
        typedef detail::overlay::traverse
            <
                false, false, 
                buffered_ring_collection<buffered_ring<Ring > >, buffered_ring_collection<buffered_ring<Ring > >,
                backtrack_for_buffer
            > traverser;

        traversed_rings.clear();
        traverser::apply(offsetted_rings, offsetted_rings,
                        detail::overlay::operation_union,
                        turn_vector, traversed_rings);
    }

    template <typename GeometryOutput, typename OutputIterator>
    inline OutputIterator assign(OutputIterator out)
    {
        typedef detail::overlay::ring_properties<Point> properties;

        std::map<ring_identifier, properties> selected;

        // Select all rings which do not have any self-intersection (other ones should be traversed)
        int index = 0;
        for(typename buffered_ring_collection<buffered_ring<Ring> >::const_iterator it = boost::begin(offsetted_rings);
            it != boost::end(offsetted_rings);
            ++it, ++index)
        {
            if (! it->has_intersections())
            {
                ring_identifier id(0, index, -1);
                selected[id] = properties(*it, true);
            }
        }

        // Select all created rings
        index = 0;
        for (typename boost::range_iterator<buffered_ring_collection<Ring> const>::type
                it = boost::begin(traversed_rings);
                it != boost::end(traversed_rings);
                ++it, ++index)
        {
            ring_identifier id(2, index, -1);
            selected[id] = properties(*it, true);
        }

        detail::overlay::assign_parents(offsetted_rings, traversed_rings, selected);
        return detail::overlay::add_rings<GeometryOutput>(selected, offsetted_rings, traversed_rings, out);
    }

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER

    template <typename Mapper>
    inline void map_turns(Mapper& mapper)
    {
        int index = 0;
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(turn_vector); it != boost::end(turn_vector); ++it)
        {
            std::string fill = "fill:rgb(0,255,0);";
            switch(it->location)
            {
                case inside_buffer : fill = "fill:rgb(255,0,0);"; break;
                case inside_original : fill = "fill:rgb(0,0,255);"; break;
            }
            mapper.map(it->point, fill, 6);
            std::ostringstream out;
            //out << it->operations[0].piece_index << "/" << it->operations[1].piece_index;
            out << " " <<  all_pieces[it->operations[0].piece_index].seg_id.segment_index
                 << "+" << all_pieces[it->operations[1].piece_index].seg_id.segment_index;
            //out << " " <<  all_pieces[it->operations[0].piece_index].index_in_all_points
            //     << "," << all_pieces[it->operations[1].piece_index].index_in_all_points;
            //out << " " <<  it->operations[0].seg_id.segment_index
            //     << "|" << it->operations[1].seg_id.segment_index;
            out << ":" << operation_char(it->operations[0].operation);
            mapper.text(it->point, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
        }
    }

    template <typename Tag, typename Mapper>
    inline void map(Mapper& mapper)
    {
        for(typename piece_vector::const_iterator it = boost::begin(all_pieces);
            it != boost::end(all_pieces);
            ++it)
        {
            Ring corner;
            std::copy(boost::begin(it->offsetted_segment), 
                    boost::end(it->offsetted_segment), 
                    std::back_inserter(corner));
            std::copy(boost::begin(it->helper_segments), 
                    boost::end(it->helper_segments), 
                    std::back_inserter(corner));

            if (it->type == buffered_segment)
            {
                if(boost::is_same<Tag, ring_tag>::value || boost::is_same<Tag, polygon_tag>::value)
                {
                    mapper.map(corner, "opacity:0.3;fill:rgb(255,128,0);stroke:rgb(0,0,0);stroke-width:1");
                }
                else if(boost::is_same<Tag, linestring_tag>::value)
                {
                    mapper.map(corner, "opacity:0.3;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");
                }
            }
            else
            {
                mapper.map(corner, "opacity:0.3;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:1");
            }


            // Put starting segment_index in centroid
            Point centroid;
            geometry::centroid(corner, centroid);
            std::ostringstream out;
            out << it->seg_id.segment_index;
            mapper.text(centroid, out.str(), "fill:rgb(255,0,0);font-family='Arial';", 5, 5);

        }

        int index = 0;
        for (typename boost::range_iterator<std::vector<Point> >::type it =
            boost::begin(offsetted_rings.front()); it != boost::end(offsetted_rings.front()); ++it)
        {
            mapper.map(*it, "fill:rgb(0,0,0);", 2);
            std::ostringstream out;
            out << index++;
            mapper.text(*it, out.str(), "fill:rgb(0,0,255);font-family='Arial';", -5, -5);
        }

    }

    template <typename Mapper>
    inline void map_traverse(Mapper& mapper)
    {
        for(typename buffered_ring_collection<Ring>::const_iterator it = boost::begin(traversed_rings);
            it != boost::end(traversed_rings);
            ++it)
        {
            mapper.map(*it, "opacity:0.4;fill:none;stroke:rgb(0,255,0);stroke-width:8");
        }
    }

    template <typename Mapper>
    inline void map_offsetted(Mapper& mapper)
    {
        for(typename buffered_ring_collection<buffered_ring<Ring> >::const_iterator it = boost::begin(offsetted_rings);
            it != boost::end(offsetted_rings);
            ++it)
        {
            if (it->discarded())
            {
                mapper.map(*it, "opacity:0.4;fill:none;stroke:rgb(255,0,0);stroke-width:8");
            }
            else
            {
                mapper.map(*it, "opacity:0.4;fill:none;stroke:rgb(0,0,255);stroke-width:8");
            }
        }
    }
#endif
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFERED_PIECE_COLLECTION_HPP
