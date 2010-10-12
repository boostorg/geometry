// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSEMBLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSEMBLE_HPP


#include <deque>
#include <map>
#include <vector>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/convert_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/add_to_containment.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
//#include <boost/geometry/strategies/intersection_result.hpp>


#include <boost/geometry/algorithms/combine.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/intersection.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
#  include <boost/geometry/util/write_dsv.hpp>
#endif


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename TurnPoints, typename Map>
inline void map_turns(Map& map, TurnPoints const& turn_points)
{
    typedef typename boost::range_value<TurnPoints>::type turn_point_type;
    typedef typename turn_point_type::container_type container_type;

    int index = 0;
    for (typename boost::range_iterator<TurnPoints const>::type
            it = boost::begin(turn_points);
         it != boost::end(turn_points);
         ++it, ++index)
    {
        if (! it->ignore())
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
                map[ring_id]++;
            }
        }
    }
}


template
<
    typename Container,
    typename Geometry1,
    typename Geometry2,
    typename RingCollection,
    typename Box
>
struct enrich_containment
{

    typedef typename boost::range_value<Container>::type item_type;
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;
    typedef void tag3; // For the ring-container


    static inline void assign(item_type& larger, item_type& smaller, int direction, bool dissolve)
    {
        typedef typename geometry::coordinate_type
            <
                Geometry1
            >::type coordinate_type;

        if (larger.signum != 0 && smaller.signum != 0)
        {
            if (larger.signum == 1)
            {
                smaller.push(larger, direction, dissolve);
            }
            else if (larger.signum == -1)
            {
                smaller.pop(/*from*/larger);
            }
        }
    }


    // Creates a selection of pointers within the specified box
    template <typename Collection>
    static inline void select_by_box(Collection& selection,
            Collection const& collection,
            Box const& box)
    {
        for (typename boost::range_iterator<Collection const>::type
                it = boost::begin(collection);
                it != boost::end(collection);
                ++it)
        {
            if (! geometry::detail::disjoint::disjoint_box_box(box, (*it)->box))
            {
                selection.push_back(*it);
            }
        }
    }

    static inline bool contains(item_type const& item1, item_type const& item2,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection)
    {
        if (item1.ring_id.source_index == 0)
        {
            return geometry::within(item2.point,
                get_ring<tag1>::apply(item1.ring_id, geometry1));
        }
        else if (item1.ring_id.source_index == 1)
        {
            return geometry::within(item2.point,
                get_ring<tag2>::apply(item1.ring_id, geometry2));
        }
        else if (item1.ring_id.source_index == 2)
        {
            return geometry::within(item2.point,
                get_ring<tag3>::apply(item1.ring_id, collection));
        }
        return false;
    }

    template <typename Selection, typename Map>
    static inline void enrich(Selection& selection, Map& map,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, bool dissolve)
    {
        typedef typename boost::range_iterator<Selection>::type iterator;

        for (iterator it1 = boost::begin(selection);
            it1 != boost::end(selection);
            ++it1)
        {
//std::cout << ".";
            item_type& item1 = **it1;
            iterator it2 = it1;
            for (it2++; it2 != boost::end(selection); ++it2)
            {
                std::pair<item_type*, item_type*> p
                            = std::make_pair(*it1, *it2);
                bool processed = map[p];
                if (! processed)
                {
                    map[p] = true;

                    item_type& item2 = **it2;
                    if (geometry::within(item2.point, item1.box)
                        && abs(item2.area) < abs(item1.area)
                        && contains(item1, item2, geometry1, geometry2,
                                    collection)
                        )
                    {
                        assign(item1, item2, direction, dissolve);
                    }
                }
            }
        }
    }

    /***
    template <int Dimension, typename Selection, typename Map>
    static inline void divide_and_conquer(Selection& selection, Map& map,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, bool dissolve, Box const& box,
            std::size_t iteration = 0, std::size_t previous_count = 0)
    {
        std::size_t n = boost::size(selection);
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "spatial divide n="
<< n
<< " previous=" << previous_count
<< " iteration=" << iteration
<< " box: " << geometry::dsv(box) << std::endl;
#endif

        if (iteration > 3)
        {
            enrich(selection, map, geometry1, geometry2, collection, direction, dissolve);
            return;
        }

        // Divide the complete box in two (alternating) halves
        Box lower = box, upper = box;
        typedef typename geometry::coordinate_type<Box>::type coordinate_type;
        coordinate_type two = 2.0;
        coordinate_type mid
            = (geometry::get<min_corner, Dimension>(box)
                + geometry::get<max_corner, Dimension>(box)) / two;

        geometry::set<max_corner, Dimension>(lower, mid);
        geometry::set<min_corner, Dimension>(upper, mid);

        // select
        std::vector<item_type*> lower_sel, upper_sel;
        select_by_box(lower_sel, selection, lower);
        select_by_box(upper_sel, selection, upper);

        divide_and_conquer<1 - Dimension>(lower_sel, map, geometry1, geometry2,
                    collection, direction, dissolve, lower, iteration + 1, n);
        divide_and_conquer<1 - Dimension>(upper_sel, map, geometry1, geometry2,
                    collection, direction, dissolve, upper, iteration + 1, n);
    }
    ***/

    static inline void enrich(Container& container,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, bool dissolve)
    {
        typedef typename boost::range_iterator<Container>::type iterator;

        int n = 0;

        for (iterator it1 = boost::begin(container);
            it1 != boost::end(container);
            ++it1)
        {
            item_type& item1 = *it1;
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << item1.ring_id << " area: " << item1.area << std::endl;
#endif
            iterator it2 = it1;
            for (it2++; it2 != boost::end(container); ++it2)
            {
                item_type& item2 = *it2;
                if (geometry::within(item2.point, item1.box)
                    && geometry::math::abs(item2.area)
                            < geometry::math::abs(item1.area)
                    && contains(item1, item2, geometry1, geometry2, collection)
                    )
                {
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << " -> contains " << item2.ring_id;
#endif
                    n++;
                    assign(item1, item2, direction, dissolve);
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << std::endl;
#endif
                }
            }
        }
        //std::cout << "Enriched: "  << n << std::endl;
    }



    static inline void apply(Container& container,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, bool dissolve, Box const& )
    {
        if (boost::size(container) == 0)
        {
            return;
        }
        enrich(container, geometry1, geometry2, collection, direction, dissolve);

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
        for (typename boost::range_iterator<Container const>::type
                it = boost::begin(container);
            it != boost::end(container);
            ++it)
        {
            std::cout << *it << std::endl;
        }
#endif

        /***
        return;

        // Method using divide and conquer (does NOT work corretly!)

        std::vector<item_type*> selection;
        selection.reserve(boost::size(container));
        for (typename boost::range_iterator<Container>::type
                it = boost::begin(container);
            it != boost::end(container);
            ++it)
        {
            selection.push_back(&(*it));
        }

        std::map<std::pair<item_type*, item_type*>, bool> map;
        divide_and_conquer<1>(selection, map, geometry1, geometry2, collection,
                    direction, dissolve, box);
        ***/
    }
};


template
<
    typename GeometryOut,
    typename Container,
    typename Geometry1,
    typename Geometry2,
    typename RingCollection,
    typename OutputIterator
>
inline OutputIterator add_all_rings(Container& container,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, bool dissolve,
            OutputIterator out)
{
    typedef typename boost::range_iterator<Container>::type iterator;
    typedef typename geometry::tag<GeometryOut>::type tag_out;
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;
    typedef void tag3; // For the ring-container


    bool result_filled = false;
    GeometryOut result;
    ring_identifier previous_id;
    previous_id.source_index = -1;

    for (iterator it = boost::begin(container);
        it != boost::end(container);
        ++it)
    {
        if (it->positive())
        {
            if (result_filled)
            {
                *out++ = result;
                previous_id.source_index = -1;
                result_filled = false;
            }

            // If it is an outer ring, it is included if there are no parents
            // (union) or if there are parents (intersection)
            if (it->included(direction, dissolve))
            {
                geometry::clear(result);
                previous_id = it->ring_id;
                result_filled = true;
                if (it->ring_id.source_index == 0)
                {
                    convert_ring<tag_out>::apply(result,
                                get_ring<tag1>::apply(it->ring_id, geometry1));
                }
                else if (it->ring_id.source_index == 1)
                {
                    convert_ring<tag_out>::apply(result,
                                get_ring<tag2>::apply(it->ring_id, geometry2));
                }
                else if (it->ring_id.source_index == 2)
                {
                    convert_ring<tag_out>::apply(result,
                                get_ring<tag3>::apply(it->ring_id, collection));
                }
            }
        }
        else
        {
            // If it is an interior ring, it is included if
            // it's parent-id matches the id of the outputted exterior ring
            if (result_filled
                && it->id() == previous_id
                && it->included(direction, dissolve))
            {
                if (it->ring_id.source_index == 0)
                {
                    convert_ring<tag_out>::apply(result,
                                get_ring<tag1>::apply(it->ring_id,
                                            geometry1), true);
                }
                else if (it->ring_id.source_index == 1)
                {
                    convert_ring<tag_out>::apply(result,
                                get_ring<tag2>::apply(it->ring_id,
                                            geometry2), true);
                }
                else if (it->ring_id.source_index == 2)
                {
                    convert_ring<tag_out>::apply(result,
                                get_ring<tag3>::apply(it->ring_id,
                                            collection), true);
                }
            }
        }
    }
    if (result_filled)
    {
        *out++ = result;
    }
    return out;
}


template
<
    typename GeometryOut,
    typename Rings, typename Map,
    typename Geometry1, typename Geometry2,
    typename OutputIterator
>
inline OutputIterator assemble(Rings const& rings, Map const& map,
            Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            int direction, bool dissolve, bool splitted,
            OutputIterator out)
{
        typedef typename geometry::tag<Geometry1>::type tag1;
        typedef typename geometry::tag<Geometry2>::type tag2;
        typedef typename geometry::tag<GeometryOut>::type tag_out;

        typedef typename geometry::point_type<GeometryOut>::type point_type;
        typedef typename boost::range_value<Rings>::type ring_type;

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble" << std::endl;
#endif

        typedef std::vector
            <
                ring_properties<point_type>
            > ring_properties_container_type;
        ring_properties_container_type ring_properties_container;

        if (! splitted)
        {
            add_to_containment
                <
                    tag1,
                    Geometry1
                >::apply(ring_properties_container,
                            ring_identifier(0, -1,-1), geometry1,
                            map, dissolve);
        }
        if (! dissolve)
        {
            add_to_containment
                <
                    tag2,
                    Geometry2
                >::apply(ring_properties_container,
                            ring_identifier(1, -1,-1), geometry2,
                            map, dissolve);
        }

        // Add all produced rings using source index 2
        {
            ring_identifier id(2, 0, -1);
            for (typename std::vector<ring_type>::const_iterator
                    it = boost::begin(rings);
                    it != boost::end(rings);
                    ++it, ++id.multi_index)
            {
                ring_properties_container.push_back(
                        ring_properties<point_type>(id, *it, false, true));
            }
        }

        bool only_positive = true;
        {
            int pos=0, neg=0;
            for (typename ring_properties_container_type::const_iterator
                    it = boost::begin(ring_properties_container);
                it != boost::end(ring_properties_container);
                ++it)
            {
                if (it->positive())
                {
                    pos++;
                }
                else
                {
                    neg++;
                    only_positive = false;
                }
            }
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
            std::cout << "assemble, pos=" << pos << " neg=" << neg << std::endl;
#endif
        }

        //if (! only_positive)
        {
            typedef geometry::box<point_type> box_type;
            box_type total = geometry::make_envelope<box_type>(geometry1);
            geometry::combine(total,
                        geometry::make_envelope<box_type>(geometry2));


#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble.properties sort "
    << boost::size(ring_properties_container) << std::endl;
#endif
            std::sort(boost::begin(ring_properties_container),
                boost::end(ring_properties_container));



#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble.enrich containment" << std::endl;
#endif
            enrich_containment
                <
                    ring_properties_container_type,
                    Geometry1,
                    Geometry2,
                    std::vector<ring_type>,
                    geometry::box<point_type>
                >::apply(ring_properties_container,
                        geometry1, geometry2, rings, direction, dissolve, total);

    // Sort container on parent-id
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble.properties sort on parent-id "
    << boost::size(ring_properties_container) << std::endl;
#endif
            std::sort(boost::begin(ring_properties_container),
                    boost::end(ring_properties_container),
                    sort_on_id_or_parent_id
                        <
                            ring_properties<point_type>
                        >());
        }
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble.add rings" << std::endl;
#endif
        return add_all_rings<GeometryOut>(ring_properties_container,
                    geometry1, geometry2, rings, direction, dissolve, out);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSEMBLE_HPP
