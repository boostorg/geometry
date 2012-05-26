// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_MARK_SPIKES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_MARK_SPIKES_HPP

#include <algorithm>

#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/area_surveyor.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/algorithms/detail/sections/sectionalize.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/perimeter.hpp>

#include <boost/geometry/geometries/box.hpp>


/*
Mark spikes in a ring/polygon.
Ring
+---------+
|         |
|         |
|   +=====  ^this "indentation" or "intrusion" or "spikey feature" is marked
|   ||    |
|   ||    |
|   ++    |
+---------+
(the actualy determination if it is marked is done by a policy)
(things are only marked, removal is done afterwards)

*/


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace mark_spikes
{

template <typename Range, typename Iterator>
inline Iterator circular_next(Range const& range, Iterator it)
{
    ++it;
    if (it == boost::end(range))
    {
        it = boost::begin(range);
    }
    return it;
}

inline std::size_t circular_next_i(std::size_t i, std::size_t const n)
{
    if (++i == n)
    {
        i = 0;
    }
    return i;
}


// Calculate the distance over the ring, in the range [it1 .. it2]
// if it1 < it2: walk from it1 .. it2
// if it1 > it2: walk from it1 .. end(ring) and from begin(ring) to it2
// Do NOT call this using begin(ring), end(ring) or 0.0 will be returned
template
<
    typename Range,
    typename Iterator,
    typename AreaStrategy,
    typename DistanceStrategy
>
inline void part_area_and_perimeter(Range const& range,
            Iterator it1, Iterator it2,
            AreaStrategy const& area_strategy,
            DistanceStrategy const& distance_strategy,
            double& area, double& perimeter, int& count)
{
    perimeter = 0;
    area = 0;
    count = 0;
    if (it1 == boost::end(range) || it2 == boost::end(range) || it1 == it2)
    {
        return;
    }

    typename AreaStrategy::state_type area_state;
    Iterator it = circular_next(range, it1), previous = it1;
    Iterator end = circular_next(range, it2);
    while (it != end)
    {
        area_strategy.apply(*previous, *it, area_state);
        perimeter += distance_strategy.apply(*previous, *it);
        previous = it;
        it = circular_next(range, it);
        count++;
    }

    // Close the ring, for area
    area_strategy.apply(*it2, *it1, area_state);
    // Do the same for distance to get correct ratio (though this might be discussed)
    perimeter += distance_strategy.apply(*it2, *it1);

    area = geometry::math::abs(area_strategy.result(area_state));
}


template <typename Iterator>
struct helper
{
    helper(int i1, int i2, Iterator t1, Iterator t2,
        double g, double a, double p, int c)
        : index1(i1)
        , index2(i2)
        , it1(t1)
        , it2(t2)
        , gap_distance(g)
        , area(a)
        , perimeter(p)
        , count(c)
    {
    }

    int index1, index2;
    Iterator it1, it2;
    double area, perimeter, gap_distance;
    int count;

    inline bool operator<(helper<Iterator> const& other) const
    {
        return this->count > other.count;
    }
};


template <typename Range, typename MarkMap, typename Policy>
struct range_mark_spikes
{
    typedef typename point_type<Range>::type point_type;

    typedef typename strategy::side::services::default_strategy
    <
        typename cs_tag<Range>::type
    >::type side_strategy_type;

    typedef typename strategy::area::services::default_strategy
        <
            typename cs_tag<point_type>::type,
            point_type
        >::type area_strategy_type;

    typedef typename strategy::distance::services::default_strategy
        <
            point_tag,
            point_type,
            point_type
        >::type distance_strategy_type;

    static inline void apply(Range const& range, ring_identifier id,
                    MarkMap& mark_map, Policy const& policy)
    {
        std::size_t const n = boost::size(range);
        if (n < 5)
        {
            return;
        }

        typedef typename boost::range_iterator<Range const>::type iterator_type;

        // Divide polygon in monotonic sections (in two directions)
        typedef model::box<point_type> box_type;
        typedef geometry::sections<box_type, 2> sections_type;
        sections_type sections;
        geometry::sectionalize<false>(range, sections);

        for (typename boost::range_iterator<sections_type>::type it = boost::begin(sections);
            it != boost::end(sections);
            ++it)
        {
            // Enlarge each box with the wished max with of the gap to be sure that
            // when walking through sections all point-pairs are considered
            geometry::buffer(it->bounding_box, it->bounding_box, policy.gap_width() * 1.001);
        }

        double const whole_area = geometry::area(range);


        typedef typename boost::range_iterator<sections_type>::type section_iterator_type;


        // Find pair-of-points lying the most close to each other,
        // where:
        // - it is in another section
        // - the distance over the ring-part is larger than X
        // - the area of the polygon formed by that ring-part smaller than X

        typedef helper<iterator_type> helper_type;
        typedef std::vector<helper_type> helper_vector_type;
        helper_vector_type candidates;

        // Quadratic loop over all sections (note this normally does not result in a quadratic loop
        // over all points).
        for(section_iterator_type sit1 = boost::begin(sections); sit1 != boost::end(sections); ++sit1)
        {
            // Note, even though combination sit1/sit2 is handled, the combination sit2/sit1 travels
            // another part over the ring and should be handled as well
            for(section_iterator_type sit2 = boost::begin(sections); sit2 != boost::end(sections); ++sit2)
            {
                if (sit1->id != sit2->id
                    && ! geometry::disjoint(sit1->bounding_box, sit2->bounding_box))
                {
                    // Check all point combinations in these boxes
                    int index1 = sit1->begin_index;
                    iterator_type it1 = boost::begin(range) + sit1->begin_index;
                    for (unsigned int i = 0; i < sit1->count; i++, ++it1, ++index1)
                    {
                        iterator_type it2 = boost::begin(range) + sit2->begin_index;
                        int index2 = sit2->begin_index;
                        for (unsigned int j = 0; j < sit2->count; j++, ++it2, ++index2)
                        {
                            double dg = geometry::distance(*it1, *it2);
                            if (dg < policy.gap_width())
                            {
                                double area, perimeter;
                                int count;
                                part_area_and_perimeter(range, it1, it2,
                                    area_strategy_type(), distance_strategy_type(),
                                    area, perimeter, count);

                                if (count >= 2
                                    && policy.apply(dg, whole_area, count, area, perimeter))
                                {
                                    candidates.push_back(
                                        helper_type(index1, index2, it1, it2, dg, area, perimeter, count));
                                }
                            }
                        }
                    }
                }
            }
        }

        if (boost::size(candidates) == 0)
        {
            return;
        }

        std::sort(candidates.begin(), candidates.end());

        /***
        if (boost::size(candidates) > 1)
        {

            // Remove overlaps
            bool first = true;
            typename boost::range_iterator<helper_vector_type>::type it = boost::begin(candidates);
            typename boost::range_iterator<helper_vector_type>::type prev = it;
            ++it;
            while (it != boost::end(candidates))
            {

                if ((it->index1 >= prev->index1 && it->index2 <= prev->index2)

                    )
                {
                    candidates.erase(it);
                    it = prev + 1;
                }
                else
                {
                    prev = it;
                }
            }
        }
        ***/

        // Check if some index combinations refer to larger combinations
#if defined(BOOST_GEOMETRY_DEBUG_MARK_SPIKES)
        for(typename boost::range_iterator<helper_vector_type>::type it
            = boost::begin(candidates); it != boost::end(candidates); ++it)
        {
            std::cout << it->count << " " << it->index1 << " " << it->index2
                << " gd=" << it->gap_distance
                << " a=" << it->area << " p=" << it->perimeter
                << " r=" << (it->perimeter > 0 ? it->area / it->perimeter : 0)
                // << " p1=" << geometry::wkt(*it->it1) << " p2=" << geometry::wkt(*it->it2)
                << std::endl;
        }
#endif

        typedef typename MarkMap::mapped_type bit_vector_type;

        // Add new vector to map if necessary
        if (mark_map.find(id) == mark_map.end())
        {
            // Add one to vector
            mark_map[id] = bit_vector_type();

            // Initialize it
            bit_vector_type& bits = mark_map[id];
            for (std::size_t i = 0; i < n; i++)
            {
                bits.push_back(false);
            }
        }

        bit_vector_type& bits = mark_map[id];

        // Mark this range or these ranges
        // TODO: we might use the fact that it is sorted and that ranges are inside others,
        // so skip those...
        for(typename boost::range_iterator<helper_vector_type const>::type it
            = boost::begin(candidates); it != boost::end(candidates); ++it)
        {
            iterator_type pit = boost::begin(range) + it->index1;
            iterator_type end = boost::begin(range) + it->index2;
            int i = it->index1;
            while (pit != end)
            {
                if (i != it->index1 && i != it->index2)
                {
                    bits[i] = true;
                }
                pit = circular_next(range, pit);
                i = circular_next_i(i, n);
            }
        }
    }
};


template <typename Polygon, typename MarkMap, typename Policy>
struct polygon_mark_spikes
{
    static inline void apply(Polygon const& polygon, ring_identifier id,
                    MarkMap& mark_map, Policy const& policy)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;

        typedef range_mark_spikes<ring_type, MarkMap, Policy> per_range;

        // Exterior ring (-1)
        id.ring_index = -1;
        per_range::apply(exterior_ring(polygon), id, mark_map, policy);

        typename interior_return_type<Polygon const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO_TPL(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            // Interior ring (zero based)
            id.ring_index++;
            per_range::apply(*it, id, mark_map, policy);
        }
    }
};


template <typename MultiGeometry, typename MarkMap, typename Policy, typename SinglePolicy>
struct multi_mark_spikes
{
    static inline void apply(MultiGeometry const& multi, ring_identifier id,
                    MarkMap& mark_map, Policy const& policy)
    {
        id.multi_index = 0;
        for (typename boost::range_iterator<MultiGeometry const>::type
                it = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            SinglePolicy::apply(*it, id, mark_map, policy);
            id.multi_index++;
        }
    }
};


}} // namespace detail::mark_spikes
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Tag,
    typename Geometry,
    typename MarkMap,
    typename Policy
>
struct mark_spikes
{
    static inline void apply(Geometry&, Policy const&)
    {}
};


template <typename Ring, typename MarkMap, typename Policy>
struct mark_spikes<ring_tag, Ring, MarkMap, Policy>
    : detail::mark_spikes::range_mark_spikes<Ring, MarkMap, Policy>
{};



template <typename Polygon, typename MarkMap, typename Policy>
struct mark_spikes<polygon_tag, Polygon, MarkMap, Policy>
    : detail::mark_spikes::polygon_mark_spikes<Polygon, MarkMap, Policy>
{};


template <typename MultiPolygon, typename MarkMap, typename Policy>
struct mark_spikes<multi_polygon_tag, MultiPolygon, MarkMap, Policy>
    : detail::mark_spikes::multi_mark_spikes
        <
            MultiPolygon,
            MarkMap,
            Policy,
            detail::mark_spikes::polygon_mark_spikes
            <
                typename boost::range_value<MultiPolygon>::type,
                MarkMap,
                Policy
            >
        >
{};



} // namespace dispatch
#endif


/*!
    \ingroup mark_spikes
    \tparam Geometry geometry type
    \param geometry the geometry to make mark_spikes
*/
template <typename Geometry, typename MarkMap, typename Policy>
inline bool mark_spikes(Geometry const& geometry,
        MarkMap& mark_map,
        Policy const& policy)
{
    concept::check<Geometry const>();

    ring_identifier id;

    dispatch::mark_spikes
        <
            typename tag<Geometry>::type,
            Geometry,
            MarkMap,
            Policy
        >::apply(geometry, id, mark_map, policy);
    return mark_map.size() > 0;
}

template <typename T = double>
class select_gapped_spike
{
public :
    inline select_gapped_spike(T const gap_width, T const ratio = 0.1)
        : m_gap_width(gap_width)
        , m_ratio(ratio)
    {}


    inline T gap_width() const
    {
        return m_gap_width;
    }

    inline bool apply(T const gap_distance, T const whole_area,
                int count, T const area, T const perimeter) const
    {
        T const ratio = perimeter == 0 ? 0 : area / perimeter;
        return
            perimeter > gap_distance
            && area < whole_area / 10.0
            && ratio < m_ratio;
    }


private :
    T m_gap_width;
    T m_ratio;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_MARK_SPIKES_HPP
