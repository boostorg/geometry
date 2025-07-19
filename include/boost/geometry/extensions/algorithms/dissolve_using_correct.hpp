// Boost.Geometry
//
// Copyright (c) 2021 Wouter van Kleunen, the Netherlands.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Official repository: https://github.com/boostorg/geometry
// Documentation: http://www.boost.org/libs/geometry

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_USING_CORRECT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_USING_CORRECT_HPP

// Originally licensed as:
// Used by permission of the author
// https://github.com/boostorg/geometry/issues/868
//  * ----------------------------------------------------------------------------
//  * "THE BEER-WARE LICENSE" (Revision 42):
//  * As long as you retain this notice you
//  * can do whatever you want with this stuff. If we meet some day, and you think
//  * this stuff is worth it, you can buy me a beer in return. 
//  * ----------------------------------------------------------------------------

#include <boost/geometry.hpp>

#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <map>
#include <set>
#include <vector>

namespace boost { namespace geometry {

namespace impl {

template<typename C, typename T>
inline void result_combine(C &result, T &&new_element)
{
    result.push_back(new_element);

   	for(std::size_t i = 0; i < result.size() - 1; ) {
        if(!geometry::intersects(result[i], result.back())) {
            ++i;
            continue;
        }

        std::vector<T> union_result;
        geometry::union_(result[i], result.back(), union_result);

        if(union_result.size() != 1) {
			++i;
			continue;
		}

       	result.back() = std::move(union_result[0]);
       	result.erase(result.begin() + i);
    } 
}

struct result_combine_multiple
{
	template<typename GeometryOut, typename GeometryIn>
	static inline void apply(GeometryOut &result, GeometryIn &new_elements)
	{
		for(auto &element: new_elements)
		{
			result_combine(result, std::move(element));
		}
	}
};

struct pseudo_vertice_key
{
    std::size_t index_1;
    double scale;
    std::size_t index_2;
    bool reroute;
    
    pseudo_vertice_key(std::size_t index_1 = 0, std::size_t index_2 = 0, double scale = 0.0, bool reroute = false)
        : index_1(index_1), scale(scale), index_2(index_2), reroute(reroute)
    { } 
};

struct compare_pseudo_vertice_key
{
    bool operator()(pseudo_vertice_key const &a, pseudo_vertice_key const &b) const {
        if(a.index_1 < b.index_1) return true;
        if(a.index_1 > b.index_1) return false;
        if(a.scale < b.scale) return true;
        if(a.scale > b.scale) return false;
        if(a.index_2 > b.index_2) return true;
        if(a.index_2 < b.index_2) return false;
        if(a.reroute && !b.reroute) return true;
        if(!a.reroute && b.reroute) return false;
		return false;
    }
};

template<typename Point>
struct pseudo_vertice
{
    Point p;
    pseudo_vertice_key link;
    
    pseudo_vertice(Point p, pseudo_vertice_key link = pseudo_vertice_key())   
        : p(p), link(link)
    { }        
};

struct assign_policy    {
	static bool const include_no_turn = true;
	static bool const include_degenerate = true;
	static bool const include_opposite = true;
	static bool const include_start_turn = true;
};

template <typename Ring, typename PseudoVertices>
inline void dissolve_find_intersections(
			Ring const &ring,
			PseudoVertices &pseudo_vertices,
    		std::set<pseudo_vertice_key, compare_pseudo_vertice_key> &start_keys)
{
	if(ring.empty()) return;
   
    for(std::size_t i = 0; i < ring.size(); ++i) {
        pseudo_vertices.emplace(pseudo_vertice_key(i, i, 0.0), ring[i]);       
	}

	using point_t = geometry::point_type_t<Ring>;

	// Detect intersections and generate pseudo-vertices
	geometry::strategies::cartesian<> strategy;
    typedef geometry::detail::overlay::turn_info
        <
            point_t
        > turn_info;

    std::vector<turn_info> turns;

    geometry::detail::self_get_turn_points::no_interrupt_policy policy;
    geometry::self_turns
        <
			assign_policy
        >(ring, strategy, turns, policy);

	for(auto const &turn: turns) {
		auto p = turn.point;
		auto i = std::min(turn.operations[0].seg_id.segment_index, turn.operations[1].seg_id.segment_index);
		auto j = std::max(turn.operations[0].seg_id.segment_index, turn.operations[1].seg_id.segment_index);

		auto const offset_1 = geometry::comparable_distance(p, ring[i]);
		auto const offset_2 = geometry::comparable_distance(p, ring[j]);

		auto const length = geometry::comparable_distance(ring[i], ring[j]);
		if ((offset_1 > 0 && offset_1 < length) || (offset_2 > 0 && offset_2 < length)) {
			pseudo_vertice_key key_j(j, i, offset_2);
			pseudo_vertices.emplace(pseudo_vertice_key(i, j, offset_1, true), pseudo_vertice<point_t>(p, key_j));
			pseudo_vertices.emplace(key_j, p);
			start_keys.insert(key_j);

			pseudo_vertice_key key_i(i, j, offset_1);
			pseudo_vertices.emplace(pseudo_vertice_key(j, i, offset_2, true), pseudo_vertice<point_t>(p, key_i));
			pseudo_vertices.emplace(key_i, p);
			start_keys.insert(key_i);
		}
	}
}

// Remove invalid points (NaN) from ring
template <typename Ring>
inline void correct_invalid(Ring &ring)
{
	for(auto i = ring.begin(); i != ring.end(); ) {
		if(!geometry::is_valid(*i))
			i = ring.erase(i);
		else
			++i;
	}	
}

// Correct orientation of ring
template <typename Ring>
inline void correct_orientation(Ring &ring, geometry::order_selector order)
{
	auto area = geometry::area(ring);
	bool should_reverse =
		(order == geometry::clockwise && area < 0) ||
		(order == geometry::counterclockwise && area > 0);

	if(should_reverse) {
		std::reverse(ring.begin(), ring.end());
	} 
}

// Close ring if not closed
template <typename Ring>
inline void correct_close(Ring &ring)
{
	// Close ring if not closed
	if(!ring.empty() && !geometry::equals(ring.back(), ring.front()))
		ring.push_back(ring.front());

}

template <typename Ring, typename PseudoVertices, typename Area>
inline std::vector<std::pair<Ring, Area>> dissolve_generate_rings(
			PseudoVertices &pseudo_vertices,
    		std::set<pseudo_vertice_key, compare_pseudo_vertice_key> const &all_start_keys, 
			geometry::order_selector order, Area remove_spike_min_area)
{
	std::vector<std::pair<Ring, Area>> result;

	// Generate all polygons by tracing all the intersections
	// Perform union to combine all polygons into single polygon again
	auto start_keys = all_start_keys;
    while(!start_keys.empty()) {    
		Ring new_ring;

		// Store point in generated polygon
		auto push_point = [&new_ring](auto const &p) { 
            if(new_ring.empty() || geometry::comparable_distance(new_ring.back(), p) > 0) {
                new_ring.push_back(p);
			}
		};

		// Store newly generated ring
		auto push_ring = [&result, remove_spike_min_area](auto& new_ring) {
			auto const area = geometry::area(new_ring);
			if(std::abs(area) > remove_spike_min_area) {
		    	result.push_back(std::make_pair(std::move(new_ring), area));
			}
		};

        auto i = pseudo_vertices.find(*start_keys.begin());	
		
		using point_t = geometry::point_type_t<Ring>;
    
    	std::vector< std::pair<point_t, std::size_t> > start_points;
		start_points.push_back(std::make_pair(i->second.p, 0));

		// Check if the outer or inner ring is closed
		auto is_closed = [&new_ring, &start_points, &push_ring](auto const& p) {
			for(auto const &i: start_points) {
				if(new_ring.size() > i.second+1 && geometry::comparable_distance(i.first, p) == 0) {
					if(i.second == 0) return true;

					// Copy the new inner ring
					Ring inner_ring(new_ring.begin() + i.second, new_ring.end());
					push_ring(inner_ring);

					// Remove the inner ring
					new_ring.erase(new_ring.begin() + i.second, new_ring.end());
				}
			}
			return false;
		};

        do {
            auto const &key = i->first;
            auto const &value = i->second;
        
			// Store the point in output polygon
			push_point(value.p);
            
			// Remove the key from the starting keys list
			auto compare_key = [&key](pseudo_vertice_key const &i) {
				return (key.index_1 == i.index_1 && key.index_2 == i.index_2 && key.scale == i.scale && key.reroute == i.reroute);
			};

			start_keys.erase(key);

			// Store possible new inner ring starting point
			if(all_start_keys.find(key) != all_start_keys.end())
				start_points.push_back(std::make_pair(value.p, new_ring.size() - 1));

            if(key.reroute) {
				// Follow by-pass
                i = pseudo_vertices.find(value.link);
			} else {
				// Continu following original polygon
                ++i;
                if(i == pseudo_vertices.end())
                    i = pseudo_vertices.begin();
            }

			// Repeat until back at starting point
		} while(!is_closed(new_ring.back()));

		// Combine with already generated polygons
		push_ring(new_ring);
   	}

    return result;
}

template <typename Ring, typename Area>
inline std::vector<std::pair<Ring, Area>> correct(Ring const &ring,
		geometry::order_selector order, Area remove_spike_min_area)
{
	constexpr std::size_t min_nodes = 3;
	if(ring.size() < min_nodes)
		return { };

	using point_t = geometry::point_type_t<Ring>;
    std::map<pseudo_vertice_key, pseudo_vertice<point_t>, compare_pseudo_vertice_key> pseudo_vertices;    
    std::set<pseudo_vertice_key, compare_pseudo_vertice_key> start_keys;

	Ring new_ring = ring;

	// Remove invalid coordinates
	correct_invalid(new_ring);

	// Close ring
	correct_close(new_ring);

	// Correct orientation
	correct_orientation(new_ring, order);

	// Detect self-intersection points
	dissolve_find_intersections(new_ring, pseudo_vertices, start_keys);

	if(start_keys.empty()) {
		auto const area = geometry::area(new_ring);
		if(std::abs(area) > remove_spike_min_area) 
			return { std::make_pair(new_ring, area) };
		else
			return { };
	}

	return dissolve_generate_rings<Ring>(pseudo_vertices, start_keys, order, remove_spike_min_area);
}

template <typename MultiPolygonCollection>
inline void fill_normalize_polygons(MultiPolygonCollection &input)
{
	for(auto &i: input) {
		for(auto &poly: i.first) {
			if(i.second < 0) {
				std::reverse(geometry::exterior_ring(poly).begin(), geometry::exterior_ring(poly).end());
			}
		}
	}
}

template <typename MultiPolygon>
struct fill_non_zero_winding
{
	template <typename MultiPolygonCollection>
	inline void operator()(MultiPolygonCollection &input) const
	{
		auto compare = [](auto const &a, auto const &b) { return std::abs(a.second) > std::abs(b.second); };
		std::sort(input.begin(), input.end(), compare);

		std::vector<int> scores;
		for(auto const &mp: input) {
			scores.push_back(mp.second > 0 ? 1 : -1);
		}

		fill_normalize_polygons(input);

		for(std::size_t i = 0; i < input.size(); ++i) {
			for(std::size_t j = i + 1; j < input.size(); ++j) {
				if(geometry::covered_by(input[j].first, input[i].first)) { 
					scores[j] += scores[i];
				}
			}
		}

		MultiPolygon combined_outers;
		MultiPolygon combined_inners;

		for(std::size_t i = 0; i < input.size(); ++i) {
			if(scores[i] != 0)
				result_combine_multiple::apply(combined_outers, input[i].first);
			else
				result_combine_multiple::apply(combined_inners, input[i].first);
		}

		MultiPolygon output;
		geometry::difference(combined_outers, combined_inners, output);

		input.resize(1);
		input.front().first = std::move(output);
	}
};


template <typename MultiPolygon>
struct fill_odd_even
{
	template <typename MultiPolygonCollection>
	inline void operator()(MultiPolygonCollection &input) const
	{
		auto compare = [](auto const &a, auto const &b) { return std::abs(a.second) < std::abs(b.second); };
		std::sort(input.begin(), input.end(), compare);

		fill_normalize_polygons(input);

		while(input.size() > 1) {
			std::size_t divide_i = input.size() / 2 + input.size() % 2;
			for(std::size_t i = 0; i < input.size() / 2; ++i) {
				std::size_t index = i + divide_i;
				if(index < input.size()) {
					MultiPolygon result;
					geometry::sym_difference(input[index].first, input[i].first, result);
					input[i].first = std::move(result);
				}
			}

			input.resize(divide_i);
		} 
	}
};
 
template<
	typename FillFunction,
	typename CombinePolicy,
	typename DifferenceFunction,
	typename Polygon,
	typename MultiPolygon,
	typename Area
>
inline void correct(Polygon const &input, MultiPolygon &output, Area remove_spike_min_area,
	FillFunction const &fill, CombinePolicy const &combine_policy, DifferenceFunction const &difference)
{
	auto const order = geometry::point_order<Polygon>::value;
	auto outer_rings = correct(geometry::exterior_ring(input), order, remove_spike_min_area);

	// Calculate all outers 
	std::vector<std::pair<MultiPolygon, Area>> combined_outers;

	for(auto &i: outer_rings) {
		Polygon poly;
		poly.outer() = std::move(i.first);

		combined_outers.push_back(std::make_pair(MultiPolygon(), i.second));
		combined_outers.back().first.push_back(std::move(poly));
	}

	// fill the collected outers and combine into single multi_polygon
	fill(combined_outers);

	// Calculate all inners and combine them if possible
	MultiPolygon combined_inners;
	for(auto const &ring: geometry::interior_rings(input)) {
		Polygon poly;
		poly.outer() = std::move(ring);

		MultiPolygon new_inners;
		correct(poly, new_inners, remove_spike_min_area, fill, combine_policy, difference);
		combine_policy.apply(combined_inners, new_inners);
	}

	// Cut out all inners from all the outers
	if(!combined_outers.empty()) {
		difference(combined_outers.front().first, combined_inners, output);
	}
}

template<
	typename FillFunction,
	typename CombinePolicy,
	typename DifferenceFunction,
	typename MultiPolygon,
	typename Area
	>
inline void correct(MultiPolygon const &input, MultiPolygon &output, Area remove_spike_min_area, FillFunction const &fill,
	CombinePolicy const &combine_policy, DifferenceFunction const &difference)
{
	for(auto const &polygon: input)
	{
		MultiPolygon new_polygons;
		correct(polygon, new_polygons, remove_spike_min_area, fill, combine_policy, difference);
		combine_policy.apply(output, new_polygons);
	}
}

} // impl

template <typename Polygon, typename MultiPolygon, typename Area>
inline void correct_non_zero(Polygon const &input, MultiPolygon &output, Area remove_spike_min_area = 0.0)
{
	impl::correct(input, output, remove_spike_min_area, 
		impl::fill_non_zero_winding<MultiPolygon>(), 
		impl::result_combine_multiple(), 
		geometry::difference<MultiPolygon, MultiPolygon, MultiPolygon>
		);
}

template<typename Polygon, typename MultiPolygon, typename Area>
inline void correct_odd_even(Polygon const &input, MultiPolygon &output, Area remove_spike_min_area = 0.0)
{
	impl::correct(input, output, remove_spike_min_area, 
		impl::fill_odd_even<MultiPolygon>(), 
		[](MultiPolygon &a, MultiPolygon const &b) {
			MultiPolygon result;
			geometry::sym_difference(a, b, result);
			a = std::move(result); 
		},
		geometry::sym_difference<MultiPolygon, MultiPolygon, MultiPolygon>
		);

}


template <typename MultiPolygon, typename Area>
inline void correct_non_zero(MultiPolygon const &input, MultiPolygon &output, Area remove_spike_min_area = 0.0)
{
	impl::correct(input, output, remove_spike_min_area, 
		impl::fill_non_zero_winding<MultiPolygon>(),
		impl::result_combine_multiple(), 
		geometry::difference<MultiPolygon, MultiPolygon, MultiPolygon>
		);
}

template<typename MultiPolygon, typename Area>
inline void correct_odd_even(MultiPolygon const &input, MultiPolygon &output, Area remove_spike_min_area = 0.0)
{
	impl::correct(input, output, remove_spike_min_area, 
		impl::fill_odd_even<MultiPolygon>(),
		[](MultiPolygon &a, MultiPolygon const &b) {
			MultiPolygon result;
			geometry::sym_difference(a, b, result);
			a = std::move(result); 
		},
		geometry::sym_difference<MultiPolygon, MultiPolygon, MultiPolygon>
		);
}

}}

#endif
