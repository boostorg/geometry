// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SPLIT_RINGS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SPLIT_RINGS_HPP

#include <boost/geometry/algorithms/overlay/get_turns.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>


#include <boost/geometry/algorithms/intersects.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/sections/sectionalize.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace split_rings
{


template <typename Range>
struct split_range
{
/*

   1             2
   +-------------+
   |      4     /
   |       |\  /
   |       | \/____ IP
   |       | /\
   |       |/  \
   |      3     \
   +-------------+
  0,6            5

  - we want to split the range at the IP into two rings
  - At the IP: we have segment_indices 2,4 (result of get_turns_in_sections)
  - We want to copy and remove vertices 3,4
  --> count=4-2
  --> copy [3,5) -> copy(begin()+id1+1, begin()+id1+count+1)
  --> erase: idem
  --> insert(begin()+id1+1)

  --> we use id1+1

*/
    static inline void apply(Range& range, Range& output
        , segment_identifier const& id1
        , segment_identifier const& id2
        , typename geometry::point_type<Range>::type const& point
        )
    {
        if (id1.ring_index == id2.ring_index
            && id1.multi_index == id2.multi_index)
        {
            int mn = (std::min)(id1.segment_index, id2.segment_index);
            mn++;

            typename boost::range_iterator<Range>::type first = range.begin();
            first += mn;

            typename boost::range_iterator<Range>::type last = first;
            last += std::abs(id2.segment_index - id1.segment_index);

            // Create splitted ring
            output.push_back(point);
            std::copy(first, last, std::back_inserter(output));
            output.push_back(point);

            // Remove the loop from the range
            range.erase(first, last);

            // Iterator is invalid because of erasure, construct again
            range.insert(range.begin() + mn, point);
        }
    }
};


template <typename Polygon>
struct split_polygon
{
    typedef typename geometry::ring_type<Polygon>::type ring_type;

    static inline void apply(Polygon& polygon, ring_type& splitted
        , segment_identifier const& id1
        , segment_identifier const& id2
        , typename geometry::point_type<Polygon>::type const& point
        )
    {
        if (id1.ring_index == id2.ring_index
            && id1.multi_index == id2.multi_index)
        {
            ring_type& ring = id1.ring_index < 0
                ? geometry::exterior_ring(polygon)
                : geometry::interior_rings(polygon)[id1.ring_index];

            split_range<ring_type>::apply(ring, splitted, id1, id2, point);
        }
    }
};


template <typename Tag, typename Geometry>
struct split
{};


template <typename Ring>
struct split<ring_tag, Ring> : split_range<Ring>
{};


template <typename Polygon>
struct split<polygon_tag, Polygon> : split_polygon<Polygon>
{};


template <typename Tag, typename RingCollection, typename Geometry>
struct insert_rings
{};


template <typename RingCollection, typename Ring>
struct insert_rings<ring_tag, RingCollection, Ring>
{
    static inline void apply(RingCollection& ring_collection, Ring const& ring)
    {
#ifdef BOOST_GEOMETRY_DEBUG_SPLIT_RINGS
std::cout << geometry::wkt(ring)
        << " ; " << geometry::area(ring)
        << " " << ring.size()
        //<< " at " << geometry::wkt(first.point)
        << std::endl;
/*std::cout << "geometry "
    << " " << geometry::area(geometry)
    << std::endl;*/
#endif

        ring_collection.push_back(ring);
    }
};


template <typename RingCollection, typename Polygon>
struct insert_rings<polygon_tag, RingCollection, Polygon>
{
    static inline void apply(RingCollection& ring_collection, Polygon const& polygon)
    {
       ring_collection.push_back(exterior_ring(polygon));
       for (typename boost::range_iterator
                <
                    typename interior_type<Polygon>::type const
                >::type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it)
        {
#ifdef BOOST_GEOMETRY_DEBUG_SPLIT_RINGS
std::cout << geometry::wkt(*it)
        << " ; " << geometry::area(*it)
        << " " << it->size()
        //<< " at " << geometry::wkt(first.point)
        << std::endl;
/*std::cout << "geometry "
    << " " << geometry::area(geometry)
    << std::endl;*/
#endif

            ring_collection.push_back(*it);
        }
    }
};


template <typename Range, typename RingCollection>
class range_split_rings
{
    typedef typename geometry::tag<Range>::type tag;
    typedef typename geometry::point_type<Range>::type point_type;

    typedef typename geometry::ring_type<Range>::type ring_type;

    // create sections in one dimension
    typedef box<point_type> box_type;
    typedef sections<box_type, 1> sections_type;
    typedef typename sections_type::value_type section_type;

    typedef typename boost::range_reverse_iterator<sections_type const>::type
        section_iterator;

    typedef typename boost::range_iterator
        <
            typename geometry::range_type<Range>::type const
        >::type iterator_type;

    typedef typename boost::range_reverse_iterator
        <
            typename geometry::range_type<Range>::type const
        >::type reverse_iterator_type;

    // for getting intersection points
    typedef detail::overlay::turn_info<point_type> turn_info;
    typedef std::vector<turn_info> turns_type;

    typedef typename strategy_intersection
        <
            typename cs_tag<point_type>::type,
            point_type,
            point_type,
            point_type
        >::segment_intersection_strategy_type strategy;



    static inline bool call(Range& range, RingCollection& ring_collection, int trial)
    {
        sections_type sections;
        geometry::sectionalize(range, sections);

        // One section or less, there can be no intersection points.
        if (boost::size(sections) <= 1)
        {
            // assign output
            return false;
        }

        turns_type turns;

        // Outer loop iterates over segments
        iterator_type it1 = boost::begin(range);

        /*if (trial % 10 == 0)
        {
            std::cout << "trial " << trial << std::endl;
        }*/

        int index1 = 0;
        for (iterator_type prev1 = it1++;
            it1 != boost::end(range);
            prev1 = it1++, index1++)
        {
            geometry::segment<point_type const> segment1(*prev1, *it1);
            box_type segment_box = geometry::make_envelope<box_type>(segment1);

            // Inner loop iterates reversely over sections
            for (section_iterator sit = boost::rbegin(sections);
                sit != boost::rend(sections);
                sit++)
            {
                if (! detail::disjoint::disjoint_box_box(segment_box, sit->bounding_box))
                {
                    // Calculate the reverse and not-reverse index
                    // Suppose section is [0..2] [begin_index..end_index] and range.size() = 6
                    // we want to have 1-2 and then 0-1
                    // -> rbegin=5, we want to have [rbegin+3..rbegin+5]
                    int index2 = sit->end_index - 1;
                    std::size_t count = 0;


                    // Iterate (reversely) over segments within section
                    for (iterator_type it2 = boost::begin(range) + index2 + 1;
                        count < sit->count;
                        it2--, index2--, count++)
                    {
                        iterator_type prev2 = it2 - 1;
                        // Do not check adjacent segments
                        bool skip = index2 + 1 >= index1
                            || (index2 == 0 && index1 >= int(sit->range_count) - 2);
                        if (! skip)
                        {
                            geometry::segment<point_type const> segment2(*prev2, *it2);

                            typename strategy::return_type result
                                = strategy::apply(segment1, segment2);

                            if (result.get<0>().count > 0)
                            {
                                ring_type copy = range;
                                ring_type splitted;

                                // Insert the intersection point.
                                // Split off the sub-ring
                                split<ring_tag, Range>::apply(range, splitted,
                                        segment_identifier(-1, -1, -1, index2),
                                        segment_identifier(-1, -1, -1, index1),
                                        result.get<0>().intersections[0]);

                                ring_collection.push_back(splitted);

#ifdef BOOST_GEOMETRY_DEBUG_SPLIT_RINGS
                                if (geometry::intersects(splitted))
                                {
                                    std::cout << geometry::wkt(copy) << std::endl;
                                    std::cout << geometry::wkt(splitted) << std::endl;
                                    std::cout << "not OK " << trial << std::endl;
                                }


if (geometry::area(splitted) > 0)
{
std::cout << geometry::wkt(splitted)
        << " ; " << geometry::area(splitted)
        << " " << splitted.size()
        << " at " << geometry::wkt(result.get<0>().intersections[0])
        << std::endl;
}
#endif

                                return true;
                            }
                        }
                    }
               }
            }
        }
        return false;
    }


public :
    // Copy by value of range is intentional, copy is modified here
    static inline void apply(Range range, RingCollection& ring_collection)
    {
        int trial = 1;
        while(call(range, ring_collection, trial++))
        {
            // no operation is intentional
        }

        // Add the (possibly untouched) input range
        insert_rings<tag, RingCollection, Range>::apply(ring_collection, range);
    }
};


template <typename Polygon, typename RingCollection>
struct polygon_split_rings
{
  typedef range_split_rings
        <
            typename ring_type<Polygon>::type,
            RingCollection
        > per_ring;

    static inline void apply(Polygon const& polygon, RingCollection& ring_collection)
    {
        per_ring::apply(exterior_ring(polygon), ring_collection);
        for (typename boost::range_iterator
                <
                    typename interior_type<Polygon>::type const
                >::type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it)
        {
            per_ring::apply(*it, ring_collection);
        }
    }
};


}} // namespace detail::split_rings
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    typename Geometry,
    typename RingCollection
>
struct split_rings
{};


template<typename Polygon, typename RingCollection>
struct split_rings<polygon_tag, Polygon, RingCollection>
    : detail::split_rings::polygon_split_rings<Polygon, RingCollection>
{};


template<typename Ring, typename RingCollection>
struct split_rings<ring_tag, Ring, RingCollection>
    : detail::split_rings::range_split_rings<Ring, RingCollection>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template
<
    typename Geometry,
    typename RingCollection
>
inline void split_rings(Geometry const& geometry, RingCollection& out)
{
    concept::check<Geometry const>();
    concept::check<typename boost::range_value<RingCollection>::type>();

    dispatch::split_rings
    <
        typename tag<Geometry>::type,
        Geometry,
        RingCollection
    >::apply(geometry, out);
}

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SPLIT_RINGS_HPP
