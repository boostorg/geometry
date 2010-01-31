// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_ASSEMBLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_ASSEMBLE_HPP

#include <deque>
#include <vector>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>


#include <boost/geometry/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/overlay/enrich_intersection_points.hpp>
//#include <boost/geometry/algorithms/detail/overlay/enrich.hpp>
#include <boost/geometry/algorithms/overlay/traverse.hpp>


#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/combine.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/iterators/range_type.hpp>

#include <boost/geometry/strategies/intersection.hpp>

//#define BOOST_GEOMETRY_DEBUG_ASSEMBLE

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
#  include <boost/geometry/util/write_dsv.hpp>
#endif

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {



template<typename Tag>
struct get_ring
{};

template<>
struct get_ring<ring_tag>
{
    template<typename Ring>
    static inline Ring const& apply(ring_identifier const& id, Ring const& ring)
    {
        return ring;
    }
};


template<>
struct get_ring<void>
{
    template<typename Container>
    static inline typename boost::range_value<Container>::type const&
                apply(ring_identifier const& id, Container const& container)
    {
        return container[id.multi_index];
    }
};


template<>
struct get_ring<box_tag>
{
    template<typename Box>
    static inline Box const& apply(ring_identifier const& id,
                    Box const& box)
    {
        return box;
    }
};


template<>
struct get_ring<polygon_tag>
{
    template<typename Polygon>
    static inline typename ring_type<Polygon>::type const& apply(
                ring_identifier const& id,
                Polygon const& polygon)
    {
        BOOST_ASSERT
            (
                id.ring_index >= -1
                && id.ring_index < boost::size(interior_rings(polygon))
            );
        return id.ring_index < 0
            ? exterior_ring(polygon)
            : interior_rings(polygon)[id.ring_index];
    }
};




template<typename Tag>
struct convert_ring
{};

template<>
struct convert_ring<ring_tag>
{
    template<typename Destination, typename Source>
    static inline void apply(Destination& destination, Source const& source,
                bool append = false)
    {
        if (! append)
        {
            geometry::convert(source, destination);
        }
    }
};




template<>
struct convert_ring<polygon_tag>
{
    template<typename Destination, typename Source>
    static inline void apply(Destination& destination, Source const& source,
                bool append = false)
    {
        if (! append)
        {
            geometry::convert(source, exterior_ring(destination));
        }
        else
        {
            interior_rings(destination).resize(
                        interior_rings(destination).size() + 1);
            geometry::convert(source, interior_rings(destination).back());
        }
    }
};



template <typename Tag, typename Geometry>
struct add_to_containment
{};

template <typename Ring>
struct add_to_containment<ring_tag, Ring>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, Ring const& ring, Map const& map)
    {
        typedef typename range_value<ContainmentContainer>::type prop;
        container.push_back(prop(id, ring, map.find(id) != map.end()));
    }
};

template <typename Box>
struct add_to_containment<box_tag, Box>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, Box const& box, Map const& map)
    {
        typedef typename range_value<ContainmentContainer>::type prop;
        container.push_back(prop(id, box, map.find(id) != map.end()));
    }
};


template <typename Polygon>
struct add_to_containment<polygon_tag, Polygon>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, Polygon const& polygon, Map const& map)
    {
        // Add exterior ring and interior rings
        ring_identifier copy = id;

        typedef add_to_containment
            <
                ring_tag,
                typename ring_type<Polygon>::type
            > policy;

        policy::apply(container, copy, exterior_ring(polygon), map);
        copy.ring_index = 0;
        for (typename boost::range_iterator
                <
                    typename interior_type<Polygon>::type const
                >::type it = boost::begin(interior_rings(polygon));
            it != boost::end(interior_rings(polygon));
            ++it, ++copy.ring_index)
        {
            policy::apply(container, copy, *it, map);
        }
    }
};



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
    typedef typename item_type::parent parent_type;
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;
    typedef void tag3; // For the ring-container


    static inline void assign(item_type& item1, item_type& item2)
    {
        int or1 = item1.area > 0 ? 1 : item1.area < 0 ? -1 : 0;
        int or2 = item2.area > 0 ? 1 : item2.area < 0 ? -1 : 0;
        if (or1 != 0 && or2 != 0)
        {
            if (or1 == 1 && or2 == 1)
            {
                item1.c = 1;
                item2.c = -1;
            }

            if (! item1.produced)
            {
                // This is an original ring, used to set info about
                // the parents of interiors
                if (or1 == 1)
                {
                    item2.push(parent_type(item1.c, item1.ring_id));
                }
                else if (or1 == -1 && ! item1.intersects)
                {
                    item2.pop();
                }
            }
            else
            {
                if (or1 == 1 && or2 == -1 && ! item2.intersects)
                {
                    // Assign this produced parent as the parent
                    // to this hole
                    item2.has_parent_id = true;
                    item2.parent_id = item1.ring_id;
                }
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
            int direction)
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
                        && std::abs(item2.area) < std::abs(item1.area)
                        && contains(item1, item2, geometry1, geometry2,
                                    collection)
                        )
                    {
                        assign(item1, item2);
                    }
                }
            }
        }
    }

    template <int Dimension, typename Selection, typename Map>
    static inline void divide_and_conquer(Selection& selection, Map& map,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, Box const& box,
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
            enrich(selection, map, geometry1, geometry2, collection, direction);
            return;
        }

        // Divide the complete box in two (alternating) halves
        Box lower = box, upper = box;
        typename geometry::coordinate_type<Box>::type two = 2.0;
        typename geometry::coordinate_type<Box>::type mid
            = (geometry::get<min_corner, Dimension>(box)
                + geometry::get<max_corner, Dimension>(box)) / two;

        geometry::set<max_corner, Dimension>(lower, mid);
        geometry::set<min_corner, Dimension>(upper, mid);

        // select
        std::vector<item_type*> lower_sel, upper_sel;
        select_by_box(lower_sel, selection, lower);
        select_by_box(upper_sel, selection, upper);

        divide_and_conquer<1 - Dimension>(lower_sel, map, geometry1, geometry2,
                    collection, direction, lower, iteration + 1, n);
        divide_and_conquer<1 - Dimension>(upper_sel, map, geometry1, geometry2,
                    collection, direction, upper, iteration + 1, n);
    }

    static inline void enrich(Container& container,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction)
    {
        typedef typename boost::range_iterator<Container>::type iterator;

        int n = 0;

        for (iterator it1 = boost::begin(container);
            it1 != boost::end(container);
            ++it1)
        {
            item_type& item1 = *it1;
            iterator it2 = it1;
            for (it2++; it2 != boost::end(container); ++it2)
            {
                item_type& item2 = *it2;
                if (geometry::within(item2.point, item1.box)
                    && std::abs(item2.area) < std::abs(item1.area)
                    && contains(item1, item2, geometry1, geometry2, collection)
                    )
                {
                    n++;
                    assign(item1, item2);
                }
            }
        }
        //std::cout << "Enriched: "  << n << std::endl;
    }



    static inline void apply(Container& container,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            int direction, Box const& box)
    {
        if (boost::size(container) == 0)
        {
            return;
        }
        enrich(container, geometry1, geometry2, collection, direction);
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
                    direction, box);
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
            int direction,
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
        if (it->area > 0)
        {
            if (result_filled)
            {
                *out++ = result;
                previous_id.source_index = -1;
                result_filled = false;
            }

            // If it is an outer ring, it is included if there are no parents
            // (union) or if there are parents (intersection)
            if (it->included(direction))
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
                && it->included(direction))
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

template <typename Container>
inline void sort_on_parent_id(Container& container, int direction)
{
    typedef typename boost::range_iterator<Container>::type iterator;
    typedef typename boost::range_value<Container>::type item_type;

    // For negative rings produced by traversal,
    // find the parent of the selected operation
    // (by popping from the stack if necessary)
    for (iterator it = boost::begin(container);
        it != boost::end(container);
        ++it)
    {
        if (it->produced && it->area < 0)
        {
            while (! it->interior_included(direction)
                && ! it->parent_stack_empty())
            {
                it->pop();
            }
        }
    }

    // Sort container on parent-id
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble.properties sort on parent-id "
    << boost::size(container) << std::endl;
#endif
    std::sort(boost::begin(container), boost::end(container),
            sort_on_id_or_parent_id<item_type>());
}

template
<
    typename Geometry1, typename Geometry2,
    typename OutputIterator, typename GeometryOut,
    int Direction,
    typename Strategy
>
struct overlay_and_assemble
{
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;
    typedef typename geometry::tag<GeometryOut>::type tag_out;

    static inline OutputIterator apply(Geometry1 const& geometry1,
                Geometry2 const& geometry2, OutputIterator out,
                Strategy const& strategy)
    {
        if (geometry::num_points(geometry1) == 0
            || geometry::num_points(geometry2) == 0)
        {
            return out;
        }

        typedef typename geometry::point_type<GeometryOut>::type point_type;


        typedef detail::overlay::traversal_turn_info<point_type> turn_info;
        typedef std::deque<turn_info> container_type;

        // "Abuse" rangetype for ringtype: for polygon, it is the type of the
        // exterior ring. For ring, it is the ring itself. That is what is
        // wished here as well.
        typedef typename geometry::range_type<GeometryOut>::type ring_type;

        container_type turn_points;

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "get turns" << std::endl;
#endif
        boost::geometry::get_turns
            <
                detail::overlay::CalculateDistancePolicy
            >(geometry1, geometry2, turn_points);

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "enrich" << std::endl;
#endif
        typename Strategy::side_strategy_type side_strategy;
        geometry::enrich_intersection_points(turn_points, geometry1, geometry2,
                    side_strategy);

        std::vector<ring_type> rings;
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "traverse" << std::endl;
#endif
        geometry::traverse<typename Strategy::side_strategy_type, ring_type>
            (
                geometry1,
                geometry2,
                Direction == -1
                    ? boost::geometry::detail::overlay::operation_intersection
                    : boost::geometry::detail::overlay::operation_union
                    ,
                turn_points,
                std::back_inserter(rings)
            );
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble" << std::endl;
#endif


        // Map intersection-points per ring-identifier to <count>
        std::map<ring_identifier, int> map;
        map_turns(map, turn_points);

// TODO: translate/implement this (sorry for the dutch)
//1. loop ook door originele ringen heen, map[ring] = 0
//2. verander dan map.find=.end in .find -> it=0
//3. we weten nu dat er "untouched"  ringen zijn, als dat zo is,
// dan "enrich assemble" etc.kl

        typedef std::vector
            <
                ring_properties<point_type>
            > ring_properties_container_type;
        ring_properties_container_type ring_properties_container;


        add_to_containment
            <
                tag1,
                Geometry1
            >::apply(ring_properties_container,
                        ring_identifier(0,-1,-1), geometry1, map);
        add_to_containment
            <
                tag2,
                Geometry2
            >::apply(ring_properties_container,
                        ring_identifier(1,-1,-1), geometry2, map);



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
                if (it->area>0)
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
                        geometry1, geometry2, rings, Direction, total);

            sort_on_parent_id(ring_properties_container, Direction);
        }
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "assemble.add rings" << std::endl;
#endif
        return add_all_rings<GeometryOut>(ring_properties_container,
                    geometry1, geometry2, rings, Direction, out);
    }
};


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_ASSEMBLE_HPP
