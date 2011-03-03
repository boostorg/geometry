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
static inline bool contains(ring_identifier ring_id, Item const& item1, Item const& item2,
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
            // Within if it is within one of the collection
            for (BOOST_AUTO(it, boost::begin(collection)); it != boost::end(collection); ++it)
            {
                int subcode = point_in_ring(item2.point,
                    get_ring<ring_tag>::apply(ring_id, *it));
                if (subcode > code)
                {
                    code = subcode;
                }
            }
            break;
    }
    return code == 0 ? item1.get_area() < 0 : code == 1;
}


template <typename Point>
struct ring_info_helper
{
    typedef typename geometry::area_result<Point>::type area_type;

    ring_identifier id;
    area_type area;
    model::box<Point> envelope;

    inline ring_info_helper(ring_identifier i, area_type a)
        : id(i), area(a)
    {}

    // To be sorted decending
    inline bool operator<(ring_info_helper<Point> const& other) const
    {
        return abs(this->area) > abs(other.area);
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
            RingMap& ring_map)
{
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;

    typedef typename RingMap::mapped_type ring_info_type;
    typedef typename ring_info_type::point_type point_type;

    typedef typename RingMap::iterator map_iterator_type;

    // A map is not sortable, so copy ring_id/area and added envelope to vector
    {
        typedef ring_info_helper<point_type> helper;
        typedef std::vector<helper> vector_type;
        typedef typename boost::range_iterator<vector_type const>::type vector_iterator_type;

        vector_type vector;

        for (map_iterator_type it = boost::begin(ring_map); it != boost::end(ring_map); ++it)
        {
            vector.push_back(helper(it->first, abs(it->second.area)));
            switch(it->first.source_index)
            {
                case 0 :
                    geometry::envelope(get_ring<tag1>::apply(it->first, geometry1), 
                            vector.back().envelope);
                    break;
                case 1 :
                    geometry::envelope(get_ring<tag2>::apply(it->first, geometry2), 
                            vector.back().envelope);
                    break;
                case 2 :
                    geometry::envelope(get_ring<void>::apply(it->first, collection), 
                            vector.back().envelope);
                    break;
            }
        }

        std::sort(vector.begin(), vector.end());

        // Assign parents
        // Semi-quadratic loop over rings to compare them to each other (envelope first)
        // Walks from largest abs(area) to smallest -> most direct parent comes last.
        for (vector_iterator_type out_it = boost::begin(vector); out_it != boost::end(vector); ++out_it)
        {
            ring_info_type& outer = ring_map[out_it->id];

            if (outer.get_area() > 0)
            {
                vector_iterator_type inn_it = out_it;
                for (++inn_it; inn_it != boost::end(vector); ++inn_it)
                {
                    ring_info_type& inner = ring_map[inn_it->id];

                    if (inner.get_area() < 0
                        && geometry::within(inner.point, out_it->envelope)
                        && contains(out_it->id, outer, inner, geometry1, geometry2, collection))
                    {
                        inner.parent = out_it->id;
                    }
                }
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


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP
