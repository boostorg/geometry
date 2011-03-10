// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/detail/divide_and_conquer.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/within_util.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{



template
<
    typename Item,
    typename Geometry1, typename Geometry2,
    typename RingCollection
>
static inline bool within_selected_input(Item const& item2, ring_identifier const& ring_id, 
        Geometry1 const& geometry1, Geometry2 const& geometry2,
        RingCollection const& collection)
{
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;

    int code = -1;
    switch (ring_id.source_index)
    {
        case 0 :
            code = point_in_ring(item2.point,
                get_ring<tag1>::apply(ring_id, geometry1));
            break;
        case 1 :
            code = point_in_ring(item2.point,
                get_ring<tag2>::apply(ring_id, geometry2));
            break;
        case 2 :
            code = point_in_ring(item2.point,
                get_ring<void>::apply(ring_id, collection));
            break;
    }
    return code == 1;
}


template <typename Point>
struct ring_info_helper
{
    typedef typename geometry::area_result<Point>::type area_type;

    ring_identifier id;
    area_type real_area;
    area_type abs_area;
    model::box<Point> envelope;

    inline ring_info_helper(ring_identifier i, area_type a)
        : id(i), real_area(a), abs_area(abs(a))
    {}

};


struct ring_info_helper_get_box
{
    template <typename Box, typename InputItem>
    static inline void apply(Box& total, InputItem const& item)
    {
        boost::geometry::combine(total, item.envelope);
    }
};

struct ring_info_helper_ovelaps_box
{
    template <typename Box, typename InputItem>
    static inline bool apply(Box const& box, InputItem const& item)
    {
        return ! boost::geometry::detail::disjoint::disjoint_box_box(box, item.envelope);
    }
};

template <typename Geometry1, typename Geometry2, typename Collection, typename RingMap>
struct assign_visitor
{
    typedef typename RingMap::mapped_type ring_info_type;

    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Collection const& m_collection;
    RingMap& m_ring_map;
    bool m_check_for_orientation;


    inline assign_visitor(Geometry1 const& g1, Geometry2 const& g2, Collection const& c, 
                RingMap& map, bool check)
        : m_geometry1(g1)
        , m_geometry2(g2)
        , m_collection(c)
        , m_ring_map(map)
        , m_check_for_orientation(check)
    {}

    template <typename Item>
    inline void apply(Item const& outer, Item const& inner, bool , bool , bool first = true)
    {
        if (first && outer.real_area < 0)
        {
            // Call reversed function
            apply(inner, outer, false, false, false);
            return;
        }

        if (outer.real_area > 0)
        {
            if (inner.real_area < 0 || m_check_for_orientation)
            {
                ring_info_type& inner_in_map = m_ring_map[inner.id];

                if (geometry::within(inner_in_map.point, outer.envelope)
                   && within_selected_input(inner_in_map, outer.id, m_geometry1, m_geometry2, m_collection)
                   )
                {
                    // Only assign parent if that parent is smaller (or if it is the first)
                    if (inner_in_map.parent.source_index == -1 
                        || outer.abs_area < inner_in_map.parent_area)
                    {
                        inner_in_map.parent = outer.id;
                        inner_in_map.parent_area = outer.abs_area;
                    }
                }
            }
        }
    }
};




template
<
    typename Geometry1, typename Geometry2,
    typename RingCollection,
    typename RingMap
>
inline void assign_parents(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            RingCollection const& collection,
            RingMap& ring_map,
            bool check_for_orientation = false)
{
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;

    typedef typename RingMap::mapped_type ring_info_type;
    typedef typename ring_info_type::point_type point_type;
    typedef model::box<point_type> box_type;

    typedef typename RingMap::iterator map_iterator_type;


    // A map is not sortable, so copy ring_id/area and added envelope to vector
    {
        typedef ring_info_helper<point_type> helper;
        typedef std::vector<helper> vector_type;
        typedef typename boost::range_iterator<vector_type const>::type vector_iterator_type;

#ifdef BOOST_GEOMETRY_TIME_OVERLAY
        boost::timer timer;
#endif

        vector_type vector;
        vector.reserve(ring_map.size());

        int count_total = ring_map.size();
        int count_positive = 0;

        for (map_iterator_type it = boost::begin(ring_map); it != boost::end(ring_map); ++it)
        {
            vector.push_back(helper(it->first, it->second.get_area()));
            helper& back = vector.back();
            switch(it->first.source_index)
            {
                case 0 :
                    geometry::envelope(get_ring<tag1>::apply(it->first, geometry1),
                            back.envelope);
                    break;
                case 1 :
                    geometry::envelope(get_ring<tag2>::apply(it->first, geometry2),
                            back.envelope);
                    break;
                case 2 :
                    geometry::envelope(get_ring<void>::apply(it->first, collection),
                            back.envelope);
                    break;
            }
            if (back.real_area > 0)
            {
                count_positive++;
            }
        }

#ifdef BOOST_GEOMETRY_TIME_OVERLAY
        std::cout << " ap: created helper vector: " << timer.elapsed() << std::endl;
#endif

        if (count_positive == count_total && ! check_for_orientation)
        {
            // Only positive rings, no assignment of parents or reversal necessary, ready here.
            return;
        }

        // If there are only a few positive rings, the loop below is not quadratic and can be handled.
        // If there are many positive rings + many negative ones, we divide and conquer.
        if (count_positive < 5)
        {
            // Assign parents
            // Semi-quadratic loop over rings to compare them to each other (envelope first)
            // Walks from largest abs(area) to smallest -> most direct parent comes last.
            int np = 0, nn = 0;
            for (vector_iterator_type out_it = boost::begin(vector);
                out_it != boost::end(vector); ++out_it)
            {
                if (out_it->real_area > 0)
                {
                    np++;
                    vector_iterator_type inn_it = out_it;
                    for (vector_iterator_type inn_it = boost::begin(vector); 
                        inn_it != boost::end(vector); ++inn_it)
                    {
                        // TODO: this is effectively now the same as above (in visitor), harmonize
                        if ( (inn_it->real_area < 0 || check_for_orientation))
                        {
                            ring_info_type& inner = ring_map[inn_it->id];
                            if (geometry::within(inner.point, out_it->envelope)
                               && within_selected_input(inner, out_it->id, geometry1, geometry2, collection))
                            {
                                if (inner.parent.source_index == -1 || out_it->abs_area < inner.parent_area)
                                {
                                    inner.parent = out_it->id;
                                    inner.parent_area = out_it->abs_area;
                                }
                            }
                        }
                    }
                }
                else
                {
                    nn++;
                }
            }
        }
        else
        {
            assign_visitor<Geometry1, Geometry2, RingCollection, RingMap> 
                        visitor(geometry1, geometry2, collection, ring_map, check_for_orientation);

            geometry::divide_and_conquer
                <
                    box_type, ring_info_helper_get_box, ring_info_helper_ovelaps_box
                >::apply(vector, visitor, 32);
        }
#ifdef BOOST_GEOMETRY_TIME_OVERLAY
        std::cout << " ap: quadradic loop: " << timer.elapsed() << std::endl;
        std::cout << " ap: POS " << np << " NEG: " << nn << std::endl;
        std::cout << " ap: check_for_orientation " << check_for_orientation << std::endl;
#endif
    }

    if (check_for_orientation)
    {
        for (map_iterator_type it = boost::begin(ring_map); it != boost::end(ring_map); ++it)
        {
            if (geometry::math::equals(it->second.get_area(), 0))
            {
                it->second.discarded = true;
            }
            else if (it->second.parent.source_index >= 0 && it->second.get_area() > 0)
            {
                // Discard positive inner ring with parent
                it->second.discarded = true;
                it->second.parent.source_index = -1;
            }
            else if (it->second.parent.source_index < 0 && it->second.get_area() < 0)
            {
                // Reverse negative ring without parent
                it->second.reversed = true;
            }
        }
    }

    // Assign childlist
    for (map_iterator_type it = boost::begin(ring_map); it != boost::end(ring_map); ++it)
    {
        if (it->second.parent.source_index >= 0)
        {
            ring_map[it->second.parent].children.push_back(it->first);
        }
    }
}

template
<
    typename Geometry,
    typename RingCollection,
    typename RingMap
>
inline void assign_parents(Geometry const& geometry,
            RingCollection const& collection,
            RingMap& ring_map)
{
    // Call it with an empty geometry
    // (ring_map should be empty for source_id==1)

    Geometry empty;
    assign_parents(geometry, empty, collection, ring_map, true);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP
