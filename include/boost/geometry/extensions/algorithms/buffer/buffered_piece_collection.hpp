// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFERED_PIECE_COLLECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFERED_PIECE_COLLECTION_HPP


#include <algorithm>
#include <cstddef>
#include <set>
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
#include <boost/geometry/algorithms/detail/occupation_info.hpp>
#include <boost/geometry/algorithms/detail/partition.hpp>


#include <boost/geometry/extensions/algorithms/buffer/buffered_ring.hpp>
#include <boost/geometry/extensions/algorithms/buffer/buffer_policies.hpp>
#include <boost/geometry/extensions/algorithms/buffer/side_on_convex_range.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

enum piece_type
{
    buffered_segment, buffered_join, buffered_flat_end
};

enum segment_relation_code
{
    segment_relation_on_left, 
    segment_relation_on_right, 
    segment_relation_within,
    segment_relation_disjoint
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
    static inline int apply(Point const& point, Geometry const& geometry)
    {
        return geometry::covered_by(point, geometry) ? 1 : -1;
    }
};

template <>
struct check_original<linestring_tag>
{
    template <typename Point, typename Geometry>
    static inline int apply(Point const& point, Geometry const& geometry)
    {
        return 0;
    }
};


template <typename Ring>
struct buffered_piece_collection
{
    typedef typename geometry::point_type<Ring>::type point_type;
    typedef typename geometry::coordinate_type<Ring>::type coordinate_type;

    struct piece
    {
        piece_type type;
        int index;

        // These both form a complete clockwise ring for each piece (with one dupped point)

		// 1: half, part of offsetted_rings
        segment_identifier first_seg_id;
        int last_segment_index; // no segment-identifier - it is always the same

		// 2: half, not part (will be indexed in one vector too)
        std::vector<point_type> helper_segments; // 3 points for segment, 2 points for join - 0 points for flat-end
    };


    typedef typename strategy::side::services::default_strategy
		<
			typename cs_tag<point_type>::type
		>::type side_strategy;
    typedef std::vector<piece> piece_vector;

    piece_vector m_pieces;
    buffered_ring_collection<buffered_ring<Ring> > offsetted_rings; // indexed by multi_index
    buffered_ring_collection<Ring> traversed_rings;
    segment_identifier current_segment_id;

    typedef std::vector<buffer_turn_info<point_type> > turn_vector_type;
    typedef detail::overlay::get_turn_info
        <
            point_type, point_type, buffer_turn_info<point_type>,
            turn_assign_for_buffer
        > turn_policy;
    turn_vector_type m_turns;


	// To check clustered locations we keep track of segments being opposite somewhere
	std::set<segment_identifier> m_in_opposite_segments;

	struct buffer_occupation_info : public occupation_info<angle_info<coordinate_type> >
	{
		std::set<segment_identifier> seg_ids;
		std::set<int> turn_indices;
	};

    typedef occupation_map<point_type, buffer_occupation_info> occupation_map_type;
    occupation_map_type m_occupation_map;


	struct redundant_turn
	{
		inline bool operator()(buffer_turn_info<point_type> const& turn) const
		{
			// Erase discarded turns (location not OK) and the turns
			// only used to detect oppositeness.
			return turn.location != location_ok 
				|| turn.opposite();
		}
	};


    inline bool is_neighbor(piece const& piece1, piece const& piece2) const
    {
        if (piece1.first_seg_id.multi_index != piece2.first_seg_id.multi_index)
        {
            return false;
        }

        if (std::abs(piece1.index - piece2.index) == 1)
        {
            return true;
        }

        int const last = boost::size(m_pieces) - 1;
        return (piece1.index == 0 && piece2.index == last)
            || (piece1.index == last && piece2.index == 0)
            ;
    }

    inline bool skip_neighbor(piece const& piece1, piece const& piece2) const
    {
        return piece1.type != piece2.type && is_neighbor(piece1, piece2);
    }

    template <typename Range, typename Iterator>
    inline void move_to_next_point(Range const& range, Iterator& next) const
    {
        ++next;
		if (next == boost::end(range))
		{
			next = boost::begin(range) + 1;
		}
    }

    template <typename Range, typename Iterator>
    inline Iterator next_point(Range const& range, Iterator it) const
    {
        Iterator result = it;
		move_to_next_point(range, result);
		while(geometry::equals(*it, *result))
		{
			move_to_next_point(range, result);
		}
		return result;
    }

    inline void calculate_turns(piece const& piece1, piece const& piece2)
    {
        typedef typename boost::range_iterator<buffered_ring<Ring> const>::type iterator;

		segment_identifier seg_id1 = piece1.first_seg_id;
		segment_identifier seg_id2 = piece2.first_seg_id;

		if (seg_id1.segment_index < 0 || seg_id2.segment_index < 0)
		{
			return;
		}

		buffered_ring<Ring> const& ring1 = offsetted_rings[seg_id1.multi_index];
		iterator it1_first = boost::begin(ring1) + seg_id1.segment_index;
		iterator it1_last = boost::begin(ring1) + piece1.last_segment_index;

		buffered_ring<Ring> const& ring2 = offsetted_rings[seg_id2.multi_index];
		iterator it2_first = boost::begin(ring2) + seg_id2.segment_index;
		iterator it2_last = boost::begin(ring2) + piece2.last_segment_index;

        buffer_turn_info<point_type> the_model;
        the_model.operations[0].piece_index = piece1.index;
        the_model.operations[0].seg_id = piece1.first_seg_id;

        iterator it1 = it1_first;
        for (iterator prev1 = it1++; 
                it1 != it1_last; 
                prev1 = it1++, the_model.operations[0].seg_id.segment_index++)
        {
            the_model.operations[1].piece_index = piece2.index;
            the_model.operations[1].seg_id = piece2.first_seg_id;

			iterator next1 = next_point(ring1, it1);

            iterator it2 = it2_first;
            for (iterator prev2 = it2++; 
                    it2 != it2_last;
                    prev2 = it2++, the_model.operations[1].seg_id.segment_index++)
            {
                // Revert (this is used more often - should be common function TODO)
                the_model.operations[0].other_id = the_model.operations[1].seg_id;
                the_model.operations[1].other_id = the_model.operations[0].seg_id;

				iterator next2 = next_point(ring2, it2);

				if (piece1.index == 21 && piece2.index == 32)
				{
					int kkk = 0;
				}

                turn_policy::apply(*prev1, *it1, *next1,
                                    *prev2, *it2, *next2,
                                    the_model, std::back_inserter(m_turns));
            }
        }
    }

	inline void fill_opposite_segments()
	{
        for (typename boost::range_iterator<turn_vector_type const>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it)
        {
			if (it->is_opposite)
			{
				m_in_opposite_segments.insert(it->operations[0].seg_id);
				m_in_opposite_segments.insert(it->operations[1].seg_id);
//std::cout << " " << it->operations[0].seg_id.segment_index;
			}
		}
	}

	inline segment_relation_code get_segment_relation(point_type const& point,
                segment_identifier const& seg_id) const
	{
		typedef typename boost::range_iterator<std::vector<point_type> const>::type iterator_type;
		iterator_type it = boost::begin(offsetted_rings[seg_id.multi_index]) + seg_id.segment_index;
		iterator_type prev = it++;
		int side = side_strategy::apply(point, *prev, *it);
        if (side == 0)
        {
            if (geometry::equals(point, *prev))
            {
                return segment_relation_on_left;
            }
            else if (geometry::equals(point, *it))
            {
                return segment_relation_on_right;
            }
            else if (collinear_point_on_segment(point, *prev, *it))
            {
                return segment_relation_within;
            }
        }
        return segment_relation_disjoint;
	}

    inline void add_angles(int index, point_type const& point, buffer_turn_operation<point_type> const& operation)
    {
        buffer_occupation_info& info = m_occupation_map.find_or_insert(point);
        info.turn_indices.insert(index);
        if (info.seg_ids.count(operation.seg_id) <= 0)
        {
            info.seg_ids.insert(operation.seg_id);
            add_incoming_and_outgoing_angles(point, 
						offsetted_rings[operation.seg_id.multi_index], 
						operation.seg_id.segment_index, 
						info);
        }
    }

    inline void classify_turn(buffer_turn_info<point_type>& turn, piece const& pc) const
    {
        if (pc.type == buffered_flat_end)
        {
            return;
        }

		int flat_ends_involved = 0;
        for (int i = 0; i < boost::size(turn.operations); i++)
        {
			// Don't check any turn against a piece of which is itself the result
			if (turn.operations[i].piece_index == pc.index)
			{
				return;
			}

            piece const& piece_from_intersection = m_pieces[turn.operations[i].piece_index];
            if (piece_from_intersection.type == buffered_flat_end)
			{
				flat_ends_involved++;
			}
		}

        int side_helper = side_on_convex_range<side_strategy>(turn.point, pc.helper_segments);
        if (side_helper == 1)
        {
			// Left or outside
            return;
        }

		segment_identifier seg_id = pc.first_seg_id;
		if (seg_id.segment_index < 0)
		{
			// Should not occur
			std::cout << "Warning: negative segment_index" << std::endl;
			return;
		}

		segment_identifier on_segment_seg_id;

		buffered_ring<Ring> const& ring = offsetted_rings[seg_id.multi_index];

        int const side_offsetted = side_on_convex_range<side_strategy>(turn.point, 
						boost::begin(ring) + seg_id.segment_index, 
						boost::begin(ring) + pc.last_segment_index,
						seg_id, on_segment_seg_id);
        if (side_offsetted == 1)
        {
            return;
        }

        if (side_offsetted == -1 && side_helper == -1)
        {
            // It is within (assumed that both halves form a closed convex clockwise ring)
            turn.count_within++;
        }
        if (side_offsetted == 0)
        {
	        turn.count_on_offsetted++;
        }
        if (side_helper == 0)
        {
			//BOOST_AUTO(d1, geometry::comparable_distance(turn.point, pc.helper_segments.back()));
			//BOOST_AUTO(d2, geometry::comparable_distance(turn.point, pc.helper_segments.front()));
            //if (d1 < 0.1 || d2 < 0.1)
			if (geometry::equals(turn.point, pc.helper_segments.back())
				|| geometry::equals(turn.point, pc.helper_segments.front()))
            {
                turn.count_on_corner++;
            }
            else
            {
				if (flat_ends_involved == 0)
				{
					turn.count_on_helper++;
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
					std::ostringstream out;
					out << "HLP " << pc.index;
					turn.debug_string += out.str();
#endif
				}
				else
				{
					turn.count_on_corner++;
				}
            }
        }
    }

	inline void classify_occupied_locations()
	{
        for (typename boost::range_iterator<typename occupation_map_type::map_type>::type it =
	            boost::begin(m_occupation_map.map);
			it != boost::end(m_occupation_map.map); ++it)
        {
	        buffer_occupation_info& info = it->second;
			if (info.occupied())
			{
				for (std::set<int>::const_iterator sit = info.turn_indices.begin();
					sit != info.turn_indices.end();
					++sit)
				{
					m_turns[*sit].count_on_occupied++;
				}
			}
//std::cout << geometry::wkt(it->first) << " " << int(info.occupied()) << std::endl;
        }
	}

	inline void get_occupation()
    {
		fill_opposite_segments();

        int index = 0;
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it, ++index)
        {
			buffer_turn_info<point_type>& turn = *it;
			if (m_in_opposite_segments.count(turn.operations[0].seg_id) > 0
				|| m_in_opposite_segments.count(turn.operations[1].seg_id) > 0)
			{
				add_angles(index, turn.point, turn.operations[0]);
				add_angles(index, turn.point, turn.operations[1]);
			}
		}

        index = 0;
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it, ++index)
        {
			buffer_turn_info<point_type>& turn = *it;
			if (m_in_opposite_segments.count(turn.operations[0].seg_id) == 0
				&& m_in_opposite_segments.count(turn.operations[1].seg_id) ==  0)
			{
                // See if it is in the map
                if (m_occupation_map.contains(turn.point))
                {
//std::cout << "Adding point " << geometry::wkt(turn.point) << std::endl;
				    add_angles(index, turn.point, turn.operations[0]);
				    add_angles(index, turn.point, turn.operations[1]);
                }
			}
		}
	}

    inline void classify_turns()
    {

        // Check if it is inside any of the pieces
        // Now: quadratic
        // TODO: in partition.
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it)
		{
            if (it->count_on_occupied == 0)
            {
                typename std::vector<piece>::const_iterator pit;
                for (pit = boost::begin(m_pieces);
                    pit != boost::end(m_pieces);
                    ++pit)
                {
                    classify_turn(*it, *pit);
                }
            }
        }

		// Set results:
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it)
        {
            if (it->count_within > 0 
                || it->count_on_helper > 0
				|| it->count_on_occupied > 0
				)
            {
                it->location = inside_buffer;
            }
        }
    }

    template <typename Geometry>
    inline void check_remaining_points(Geometry const& input_geometry, int factor)
    {
        // TODO: this should be done as a collection-of-points, for performance
        for (typename boost::range_iterator<turn_vector_type>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it)
        {
            if (it->location == location_ok)
			{
				int code = check_original
                        <
                            typename geometry::tag<Geometry>::type
                        >::apply(it->point, input_geometry);
				if (code * factor == 1)
				{
					it->location = inside_original;
				}
			}
        }
    }

    template <typename Geometry>
    inline void get_turns(Geometry const& input_geometry, int factor)
    {
		// Now: quadratic
        // TODO use partition

        for(typename piece_vector::const_iterator it1 = boost::begin(m_pieces);
            it1 != boost::end(m_pieces);
            ++it1)
        {
            for(typename piece_vector::const_iterator it2 = it1 + 1;
                it2 != boost::end(m_pieces);
                ++it2)
            {
                if (! skip_neighbor(*it1, *it2))
                {
                    calculate_turns(*it1, *it2);
                }
            }
        }

		get_occupation();
        classify_occupied_locations();
        classify_turns();

        if (boost::is_same<typename tag_cast<typename tag<Geometry>::type, areal_tag>::type, areal_tag>())
        {
            check_remaining_points(input_geometry, factor);
        }
    }

    inline void start_new_ring()
    {
        int const n = offsetted_rings.size();
        current_segment_id.source_index = 0;
        current_segment_id.multi_index = n;
        current_segment_id.ring_index = -1;
        current_segment_id.segment_index = 0;

        offsetted_rings.resize(n + 1);
    }

    inline int add_point(point_type const& p)
    {
        BOOST_ASSERT
            (
                boost::size(offsetted_rings) > 0
            );

        current_segment_id.segment_index++;
        offsetted_rings.back().push_back(p);
		return offsetted_rings.back().size();
    }

    inline piece& add_piece(piece_type type, bool decrease_by_one)
    {
        piece pc;
        pc.type = type;
        pc.index = boost::size(m_pieces);
        pc.first_seg_id = current_segment_id;

        std::size_t const n = boost::size(offsetted_rings.back());
        pc.first_seg_id.segment_index = decrease_by_one ? n - 1 : n;

        m_pieces.push_back(pc);
        return m_pieces.back();
    }

    inline void add_piece(piece_type type, point_type const& p1, point_type const& p2, 
            point_type const& b1, point_type const& b2)
    {
        bool const last_type_join = ! m_pieces.empty() 
				&& m_pieces.back().first_seg_id.multi_index == current_segment_id.multi_index
                && m_pieces.back().type == buffered_join;

        piece& pc = add_piece(type, last_type_join);

        // If it follows the same piece-type point both should be added.
        // There should be two intersections later and it should be discarded.
        // But for need it to calculate intersections
        if (! last_type_join)
        {
            add_point(b1);
        }
        pc.last_segment_index = add_point(b2);

        pc.helper_segments.push_back(b2);
        pc.helper_segments.push_back(p2);
        pc.helper_segments.push_back(p1);
        pc.helper_segments.push_back(b1);
    }

    template <typename Range>
    inline piece& add_piece(piece_type type, Range const& range)
    {
        piece& pc = add_piece(type, true);

        bool first = true;
		int last = offsetted_rings.back().size() + 1;
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
                last = add_point(*it);
            }

        }

        pc.last_segment_index = last;

        return pc;
    }

    template <typename Range>
    inline void add_piece(piece_type type, point_type const& p, Range const& range)
    {
        piece& pc = add_piece(type, range);

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

        enrich_intersection_points<false, false>(m_turns,
                    detail::overlay::operation_union,
                    offsetted_rings, offsetted_rings,
                    side_strategy_type());
    }

    // Discards all rings which do have not-OK intersection points only.
    // Those can never be traversed and should not be part of the output.
    inline void discard_rings()
    {
        for (typename boost::range_iterator<turn_vector_type const>::type it =
            boost::begin(m_turns); it != boost::end(m_turns); ++it)
        {
            if (it->location != location_ok)
            {
                offsetted_rings[it->operations[0].seg_id.multi_index].has_discarded_intersections = true;
                offsetted_rings[it->operations[1].seg_id.multi_index].has_discarded_intersections = true;
            }
            else if (! it->both(detail::overlay::operation_union))
            {
                offsetted_rings[it->operations[0].seg_id.multi_index].has_accepted_intersections = true;
                offsetted_rings[it->operations[1].seg_id.multi_index].has_accepted_intersections = true;
            }
        }
    }
                    
    inline void discard_turns()
    {
        m_turns.erase
			(
				std::remove_if(boost::begin(m_turns), boost::end(m_turns),
								redundant_turn()),
				boost::end(m_turns)
			);

    }

    inline void traverse()
    {
        typedef detail::overlay::traverse
            <
                false, false, 
                buffered_ring_collection<buffered_ring<Ring> >, buffered_ring_collection<buffered_ring<Ring > >,
                backtrack_for_buffer
            > traverser;

        traversed_rings.clear();
        traverser::apply(offsetted_rings, offsetted_rings,
                        detail::overlay::operation_union,
                        m_turns, traversed_rings);
    }

    template <typename GeometryOutput, typename OutputIterator>
    inline OutputIterator assign(OutputIterator out)
    {
        typedef detail::overlay::ring_properties<point_type> properties;

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

//std::cout << geometry::wkt(*it) << std::endl;
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

        detail::overlay::assign_parents(offsetted_rings, traversed_rings, selected, false);
        return detail::overlay::add_rings<GeometryOutput>(selected, offsetted_rings, traversed_rings, out);
    }

};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFERED_PIECE_COLLECTION_HPP
