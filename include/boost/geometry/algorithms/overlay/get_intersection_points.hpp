// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_GET_INTERSECTION_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_GET_INTERSECTION_POINTS_HPP

/*!
\defgroup overlay overlay helper operations (getting intersection points, etc)
*/


#include <cstddef>
#include <map>

#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/is_multi.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/iterators/range_type.hpp>

#include <boost/geometry/strategies/cartesian/cart_intersect.hpp>
#include <boost/geometry/strategies/intersection.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>


#include <boost/geometry/algorithms/overlay/intersection_point.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>

#include <boost/geometry/algorithms/combine.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/sectionalize.hpp>
#include <boost/geometry/algorithms/get_section.hpp>
#include <boost/geometry/algorithms/within.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
#  include <sstream>
#  include <boost/geometry/util/write_dsv.hpp>
#endif


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace get_intersection_points {


template
<
    typename Segment1,
    typename Segment2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct relate
{
private :
    template <typename Segment, typename Point>
    static inline void copy_point(int how, Segment const& segment, Point& point)
    {
        if (how == 1)
        {
            // Arrival
            geometry::copy_coordinates(segment.first, point);
        }
        else
        {
            // Departure
            geometry::copy_coordinates(segment.second, point);
        }
    }

    template <typename Info, typename IntersectionPoint, typename Direction, typename Segment>
    static inline void process_one(
                Info& info,
                IntersectionPoint const& ipoint,
                Segment const& segment,
                segment_identifier const& seg_id_first,
                segment_identifier const& seg_id_second,
                Direction const& dir, int arrival, int direction, bool reverse)
    {
        info.how = dir.how;
        info.opposite = dir.opposite;

        info.seg_id = seg_id_first;
        info.other_id = seg_id_second;
        copy_point(arrival, segment, info.other_point);

        info.distance = geometry::distance(ipoint, segment.first);

        info.arrival = arrival;
        info.direction = direction;
        info.sides = dir.sides;

        // Seen from B, the first one is B, the second one is A
        if (reverse)
        {
            info.sides.reverse();
        }
    }


    template <typename IntersectionPoint, typename Direction>
    static inline void process(IntersectionPoint& ipoint,
                Direction const& dir,
                Segment1 const& s1, Segment2 const& s2,
                segment_identifier const& seg_id1,
                segment_identifier const& seg_id2,
                IntersectionPoints& out, bool& trivial)
    {
        typedef typename boost::range_value
            <
                IntersectionPoints
            >::type intersection_point;

        typename intersection_point::traversal_type info;

        ipoint.info.resize(ipoint.info.size() + 1);
        process_one(ipoint.info.back(), ipoint,
            s1, seg_id1, seg_id2,
            dir, dir.how_a, dir.dir_a, false);

        ipoint.info.resize(ipoint.info.size() + 1);
        process_one(ipoint.info.back(), ipoint,
            s2, seg_id2, seg_id1,
            dir, dir.how_b, dir.dir_b, true);


        if (dir.how != 'i')
        {
            trivial = false;
            ipoint.trivial = false;
        }

        out.push_back(ipoint);
    }



public :
    static inline bool apply(Segment1 const& s1, Segment2 const& s2,
                segment_identifier const& seg_id1,
                segment_identifier const& seg_id2,
                IntersectionPoints& out, bool& trivial)
    {
        typename IntersectionStrategy::return_type result
                = IntersectionStrategy::apply(s1, s2);

        for (std::size_t i = 0; i < result.get<0>().count; i++)
        {
            process(
                // Process the tupled result value
                result.get<0>().intersections[i],
                result.get<1>(),
                s1, s2, seg_id1, seg_id2,
                out, trivial);
        }
        return result.get<0>().count > 0;
    }
};

template
<
    typename Geometry1, typename Geometry2,
    typename Section1, typename Section2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
class get_ips_in_sections
{
public :
    static inline void apply(
            std::size_t source_id1, Geometry1 const& geometry1,
                Section1 const& sec1,
            std::size_t source_id2, Geometry2 const& geometry2,
                Section2 const& sec2,
            bool return_if_found,
            IntersectionPoints& intersection_points,
            bool& trivial)
    {

        typedef typename boost::range_const_iterator
            <
                typename geometry::range_type<Geometry1>::type
            >::type range1_iterator;
        typedef typename boost::range_const_iterator
            <
                typename geometry::range_type<Geometry2>::type
            >::type range2_iterator;

        int const dir1 = sec1.directions[0];
        int const dir2 = sec2.directions[0];
        int index1 = sec1.begin_index;
        int ndi1 = sec1.non_duplicate_index;

        bool const same_source =
            source_id1 == source_id2
                    && sec1.multi_index == sec2.multi_index
                    && sec1.ring_index == sec2.ring_index;

        // Note that it is NOT possible to have section-iterators here
        // because of the logistics of "index" (the section-iterator automatically
        // skips to the begin-point, we loose the index or have to recalculate it)
        // So we mimic it here
        range1_iterator it1, end1;
        get_section(geometry1, sec1, it1, end1);

        // Mimic 1: Skip to point such that section interects other box
        range1_iterator prev1 = it1++;
        for(; it1 != end1 && preceding<0>(dir1, *it1, sec2.bounding_box);
            prev1 = it1++, index1++, ndi1++)
        {
        }
        // Go back one step because we want to start completely preceding
        it1 = prev1;

        // Walk through section and stop if we exceed the other box
        for (prev1 = it1++;
            it1 != end1 && ! exceeding<0>(dir1, *prev1, sec2.bounding_box);
            prev1 = it1++, index1++, ndi1++)
        {
            segment1_type s1(*prev1, *it1);

            int index2 = sec2.begin_index;
            int ndi2 = sec2.non_duplicate_index;

            range2_iterator it2, end2;
            get_section(geometry2, sec2, it2, end2);

            range2_iterator prev2 = it2++;

            // Mimic 2:
            for(; it2 != end2 && preceding<0>(dir2, *it2, sec1.bounding_box);
                prev2 = it2++, index2++, ndi2++)
            {
            }
            it2 = prev2;

            for (prev2 = it2++;
                it2 != end2 && ! exceeding<0>(dir2, *prev2, sec1.bounding_box);
                prev2 = it2++, index2++, ndi2++)
            {
                bool skip = same_source;
                if (skip)
                {
                    // If sources are the same (possibly self-intersecting):
                    // check if it is a neighbouring sement.
                    // (including first-last segment
                    //  and two segments with one or more degenerate/duplicate
                    //  (zero-length) segments in between)

                    // Also skip if index1 < index2 to avoid getting all
                    // intersections twice (only do this on same source!)

                    // About n-2:
                    //   (square: range_count=5, indices 0,1,2,3
                    //    -> 0-3 are adjacent)
                    skip = index2 >= index1
                        || ndi1 == ndi2 + 1
                        || (index2 == 0 && index1 >= int(sec1.range_count) - 2)
                        ;
                }

                if (! skip)
                {
                    if (relate<segment1_type, segment2_type, IntersectionPoints, IntersectionStrategy>
                        ::apply(s1, segment2_type(*prev2, *it2),
                            segment_identifier(source_id1,
                                        sec1.multi_index, sec1.ring_index, index1),
                            segment_identifier(source_id2,
                                        sec2.multi_index, sec2.ring_index, index2),
                            intersection_points, trivial)
                        && return_if_found)
                    {
                        return;
                    }
                }
            }
        }
    }


private :
    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;
    typedef typename geometry::segment<const point1_type> segment1_type;
    typedef typename geometry::segment<const point2_type> segment2_type;


    template <size_t Dim, typename Point, typename Box>
    static inline bool preceding(int dir, Point const& point, Box const& box)
    {
        return (dir == 1  && get<Dim>(point) < get<min_corner, Dim>(box))
            || (dir == -1 && get<Dim>(point) > get<max_corner, Dim>(box));
    }

    template <size_t Dim, typename Point, typename Box>
    static inline bool exceeding(int dir, Point const& point, Box const& box)
    {
        return (dir == 1  && get<Dim>(point) > get<max_corner, Dim>(box))
            || (dir == -1 && get<Dim>(point) < get<min_corner, Dim>(box));
    }


};


template
<
    typename Ring, typename Box,
    typename Section1, typename Section2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
class get_ips_range_box
{
public :
    static inline void apply(
            std::size_t source_id1, Ring const& ring,
            std::size_t source_id2, Box const& box,
            Section1 const& sec1, Section2 const& sec2,
            IntersectionPoints& intersection_points, bool& trivial)
    {
        get_ips_in_sections<Ring, Box, Section1, Section2, IntersectionPoints, IntersectionStrategy>
            ::apply(
                source_id1, ring, sec1,
                source_id2, box, sec2,
                false,
                intersection_points, trivial);
    }
};

template <typename Section, int Dimension, int Index>
struct compare_section
{
    inline bool operator()(Section const& left, Section const& right) const
    {
        return
            geometry::get<Index, Dimension>(left.bounding_box)
                < geometry::get<Index, Dimension>(right.bounding_box);
    }
};



template
<
    typename Geometry1,
    typename Geometry2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
class get_ips_generic
{
    template <typename Box, typename Sections>
    static inline void add_sections(Box& box, Sections const& sections)
    {
        for (typename boost::range_iterator<const Sections>::type
                    it = sections.begin();
            it != sections.end();
            ++it)
        {
            geometry::combine(box, it->bounding_box);
        }
    }

    template <typename Sections, typename Box>
    static inline void get_sections(Sections const& sections,
            Box const& box, Sections& selection)
    {
        for (typename boost::range_iterator<const Sections>::type
                    it = sections.begin();
            it != sections.end();
            ++it)
        {
            if (! geometry::detail::disjoint::disjoint_box_box(box, it->bounding_box))
            {
                selection.push_back(*it);
            }
        }
    }

    template <typename Sections1, typename Sections2, typename Map>
    static inline void intersect(
            std::size_t source_id1, Geometry1 const& geometry1,
            std::size_t source_id2, Geometry2 const& geometry2,
            IntersectionPoints& intersection_points,
            Sections1 const& sec1, Sections2 const& sec2,
            Map& map,
            bool &trivial)
    {
        for (typename boost::range_const_iterator<Sections1>::type
                    it1 = sec1.begin();
            it1 != sec1.end();
            ++it1)
        {
            for (typename boost::range_const_iterator<Sections2>::type
                        it2 = sec2.begin();
                it2 != sec2.end();
                ++it2)
            {
                std::pair<int, int> p = std::make_pair(it1->id, it2->id);
                bool processed = map[p];
                if (! processed)
                {
                    map[p] = true;
                    if (! geometry::detail::disjoint::disjoint_box_box(
                                    it1->bounding_box, it2->bounding_box))
                    {
                        get_ips_in_sections
                        <
                            Geometry1,
                            Geometry2,
                            typename boost::range_value<Sections1>::type,
                            typename boost::range_value<Sections2>::type,
                            IntersectionPoints,
                            IntersectionStrategy
                        >::apply(
                                source_id1, geometry1, *it1,
                                source_id2, geometry2, *it2,
                                false,
                                intersection_points, trivial);
                    }
                }
            }
        }
    }


    // Divide and conquer (suggested by Luke during GGL Formal Review)
    template
        <
            std::size_t Dimension,
            typename Box,
            typename Sections1, typename Sections2,
            typename Map
        >
    static inline void divide_and_conquer(
            std::size_t source_id1, Geometry1 const& geometry1,
            std::size_t source_id2, Geometry2 const& geometry2,
            IntersectionPoints& intersection_points,

            Box const& box,
            Sections1 const& sec1, Sections2 const& sec2,
            Map& map,
            bool &trivial,
            std::size_t iteration = 0, std::size_t previous_count = 0)
    {
        // To stop the iteration, fallback to (quadratic) behaviour below certain limits,
        // or if dividing does not give any profit.
        std::size_t n = sec1.size() + sec2.size();
        if (sec1.size() < 5
            || sec2.size() < 5
            || n == previous_count
            || iteration > 100)
        {
            intersect(source_id1, geometry1, source_id2, geometry2,
                            intersection_points, sec1, sec2, map, trivial);
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

        Sections1 lower1, upper1;
        Sections2 lower2, upper2;
        get_sections(sec1, lower, lower1);
        get_sections(sec2, lower, lower2);
        get_sections(sec1, upper, upper1);
        get_sections(sec2, upper, upper2);

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION_DIVIDE_AND_CONQUER
        std::cout
            << "Get IP's, iteration: " << iteration
            << " box: " << geometry::dsv(box)
            << " n: " << n
            << " lower: " << lower1.size() << " , " << lower2.size()
            << " upper: " << upper1.size() << " , " << upper2.size()
            << std::endl;
#endif

        // Recursively handle lower and upper half, dividing in other dimension
        divide_and_conquer<1 - Dimension>(source_id1, geometry1,
                source_id2, geometry2, intersection_points,
                lower, lower1, lower2, map, trivial, iteration + 1, n);

        divide_and_conquer<1 - Dimension>(source_id1, geometry1,
                source_id2, geometry2, intersection_points,
                upper, upper1, upper2, map, trivial, iteration + 1, n);
    }

public:
    static inline bool apply(
            std::size_t source_id1, Geometry1 const& geometry1,
            std::size_t source_id2, Geometry2 const& geometry2,
            IntersectionPoints& intersection_points)
    {
        // Create monotonic sections in ONE direction
        // - in most cases ONE direction is faster (e.g. ~1% faster for the NLP4 testset)
        // - the sections now have a limit (default 10) so will not be too large

        // Note that the sections contain boxes, are dynamic, and therefore
        // are specified using output/intersection-point-type
        // (to enable input-pointer-point-types)
        typedef typename boost::range_value<IntersectionPoints>::type ip_type;
        typedef typename ip_type::point_type point_type;
        typedef typename geometry::sections<geometry::box<point_type>, 1> sections1_type;
        typedef typename geometry::sections<geometry::box<point_type>, 1> sections2_type;

        sections1_type sec1;
        sections2_type sec2;

        geometry::sectionalize(geometry1, sec1);
        geometry::sectionalize(geometry2, sec2);

        // Divide and conquer
        geometry::box<point_type> box;
        geometry::assign_inverse(box);
        add_sections(box, sec1);
        add_sections(box, sec2);

        // House-keeping map, to avoid section-pairs being compared twice
        std::map<std::pair<int, int>, bool> map;

        bool trivial = true;
        divide_and_conquer<1>(source_id1, geometry1, source_id2, geometry2,
            intersection_points, box, sec1, sec2, map, trivial);
        return trivial;
    }
};



template
<
    typename Range,
    typename Box,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_ips_cs
{
    static inline void apply(std::size_t source_id1, Range const& range,
            int multi_index, int ring_index,
            std::size_t source_id2, Box const& box,
            IntersectionPoints& intersection_points,
            bool& trivial)
    {
        if (boost::size(range) <= 1)
        {
            return;
        }


        typedef typename geometry::point_type<Box>::type box_point_type;
        typedef typename geometry::point_type<Range>::type point_type;

        typedef segment<const box_point_type> box_segment_type;
        typedef segment<const point_type> segment_type;

        point_type lower_left, upper_left, lower_right, upper_right;
        assign_box_corners(box, lower_left, lower_right, upper_left, upper_right);

        box_segment_type left(lower_left, upper_left);
        box_segment_type top(upper_left, upper_right);
        box_segment_type right(upper_right, lower_right);
        box_segment_type bottom(lower_right, lower_left);


        typedef typename boost::range_const_iterator<Range>::type iterator_type;
        iterator_type it = boost::begin(range);

        bool first = true;

        char previous_side[2] = {0, 0};

        int index = 0;

        for (iterator_type prev = it++;
            it != boost::end(range);
            prev = it++, index++)
        {
            segment_type segment(*prev, *it);

            if (first)
            {
                previous_side[0] = get_side<0>(box, *prev);
                previous_side[1] = get_side<1>(box, *prev);
            }

            char current_side[2];
            current_side[0] = get_side<0>(box, *it);
            current_side[1] = get_side<1>(box, *it);

            // There can NOT be intersections if
            // 1) EITHER the two points are lying on one side of the box (! 0 && the same)
            // 2) OR same in Y-direction
            // 3) OR all points are inside the box (0)
            /*if (! (
                (current_side[0] != 0 && current_side[0] == previous_side[0])
                || (current_side[1] != 0 && current_side[1] == previous_side[1])
                || (current_side[0] == 0
                        && current_side[1] == 0
                        && previous_side[0] == 0
                        && previous_side[1] == 0)
                  )
                )*/
            if (true)
            {
                segment_identifier seg_id(source_id1,
                            multi_index, ring_index, index);

                typedef relate
                    <
                        segment_type,
                        box_segment_type,
                        IntersectionPoints,
                        IntersectionStrategy
                    > relater;

                // Depending on code some relations can be left out
                relater::apply(segment, left, seg_id,
                        segment_identifier(source_id2, -1, -1, 0),
                        intersection_points, trivial);
                relater::apply(segment, top, seg_id,
                        segment_identifier(source_id2, -1, -1, 1),
                        intersection_points, trivial);
                relater::apply(segment, right, seg_id,
                        segment_identifier(source_id2, -1, -1, 2),
                        intersection_points, trivial);
                relater::apply(segment, bottom, seg_id,
                        segment_identifier(source_id2, -1, -1, 3),
                        intersection_points, trivial);

            }
        }
    }

private:
    template<std::size_t Index, typename Point>
    static inline int get_side(Box const& box, Point const& point)
    {
        // Inside -> 0
        // Outside -> -1 (left/below) or 1 (right/above)
        // On border -> -2 (left/lower) or 2 (right/upper)
        // The only purpose of the value is to not be the same,
        // and to denote if it is inside (0)

        typename coordinate_type<Point>::type const& c = get<Index>(point);
        typename coordinate_type<Box>::type const& left = get<min_corner, Index>(box);
        typename coordinate_type<Box>::type const& right = get<max_corner, Index>(box);

        if (geometry::math::equals(c, left)) return -2;
        else if (geometry::math::equals(c, right)) return 2;
        else if (c < left) return -1;
        else if (c > right) return 1;
        else return 0;
    }


};


}} // namespace detail::get_intersection_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag1, typename GeometryTag2,
    bool IsMulti1, bool IsMulti2,
    typename Geometry1, typename Geometry2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_intersection_points
{
};


template<typename Polygon, typename Box, typename IntersectionPoints, typename IntersectionStrategy>
struct get_intersection_points
    <
        polygon_tag, box_tag, false, false,
        Polygon, Box,
        IntersectionPoints,
        IntersectionStrategy
    >
{

    static inline bool apply(
            std::size_t source_id1, Polygon const& polygon,
            std::size_t source_id2, Box const& box,
            IntersectionPoints& intersection_points)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;

        typedef typename boost::range_const_iterator
            <
                typename interior_type<Polygon>::type
            >::type iterator_type;


        typedef detail::get_intersection_points::get_ips_cs
            <
                ring_type,
                Box,
                IntersectionPoints,
                IntersectionStrategy
            > intersector_type;

        bool trivial = true;
        intersector_type::apply(
                source_id1, geometry::exterior_ring(polygon), -1, -1,
                source_id2, box,
                intersection_points, trivial);

        int i = 0;
        for (iterator_type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it, ++i)
        {
            intersector_type::apply(
                    source_id1, *it, -1, i,
                    source_id2, box, intersection_points, trivial);
        }

        return trivial;
    }
};


template<typename Ring, typename Box, typename IntersectionPoints, typename IntersectionStrategy>
struct get_intersection_points
    <
        ring_tag, box_tag, false, false,
        Ring, Box,
        IntersectionPoints, IntersectionStrategy
    >
{
    static inline bool apply(
            std::size_t source_id1, Ring const& ring,
            std::size_t source_id2, Box const& box,
            IntersectionPoints& intersection_points)
    {
        typedef typename boost::range_const_iterator
            <
                Ring
            >::type iterator_type;

        typedef detail::get_intersection_points::get_ips_cs
            <Ring, Box, IntersectionPoints, IntersectionStrategy> intersector_type;

        bool trivial = true;
        intersector_type::apply(
                source_id1, ring, -1, -1,
                source_id2, box,
                intersection_points, trivial);

        return trivial;
    }
};


template
<
    typename Ring1,
    typename Ring2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_intersection_points
    <
        ring_tag, ring_tag, false, false,
        Ring1, Ring2,
        IntersectionPoints, IntersectionStrategy
    >
    : detail::get_intersection_points::get_ips_generic
        <
            Ring1,
            Ring2,
            IntersectionPoints,
            IntersectionStrategy
        >
{};


template
<
    typename Polygon1,
    typename Polygon2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_intersection_points
    <
        polygon_tag, polygon_tag, false, false,
        Polygon1, Polygon2,
        IntersectionPoints, IntersectionStrategy
    >
    : detail::get_intersection_points::get_ips_generic
        <
            Polygon1,
            Polygon2,
            IntersectionPoints,
            IntersectionStrategy
        >
{};

template
<
    typename Polygon,
    typename Ring,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_intersection_points
    <
        polygon_tag, ring_tag, false, false,
        Polygon, Ring,
        IntersectionPoints, IntersectionStrategy
    >
    : detail::get_intersection_points::get_ips_generic
        <
            Polygon,
            Ring,
            IntersectionPoints,
            IntersectionStrategy
        >
{};

template
<
    typename LineString1,
    typename LineString2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_intersection_points
    <
        linestring_tag, linestring_tag, false, false,
        LineString1, LineString2,
        IntersectionPoints, IntersectionStrategy
    >
    : detail::get_intersection_points::get_ips_generic
        <
            LineString1,
            LineString2,
            IntersectionPoints,
            IntersectionStrategy
        >
{};

template
<
    typename GeometryTag1, typename GeometryTag2,
    bool IsMulti1, bool IsMulti2,
    typename Geometry1, typename Geometry2,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct get_intersection_points_reversed
{
    static inline bool apply(
            std::size_t source_id1, Geometry1 const& g1,
            std::size_t source_id2, Geometry2 const& g2,
            IntersectionPoints& intersection_points)
    {
        return get_intersection_points
            <
                GeometryTag2, GeometryTag1,
                IsMulti2, IsMulti1,
                Geometry2, Geometry1,
                IntersectionPoints, IntersectionStrategy
            >::apply(source_id2, g2, source_id1, g1, intersection_points);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



/*!
    \brief Calculate intersection points of two geometries
    \ingroup overlay
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam IntersectionPoints type of intersection container (e.g. vector of "intersection_point"'s)
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param intersection_points container which will contain intersection points
 */
template <typename Geometry1, typename Geometry2, typename IntersectionPoints>
inline void get_intersection_points(Geometry1 const& geometry1,
            Geometry2 const& geometry2, IntersectionPoints& intersection_points)
{
    concept::check_concepts_and_equal_dimensions<const Geometry1, const Geometry2>();

    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    typedef typename strategy_intersection
        <
            typename cs_tag<Geometry1>::type,
            Geometry1,
            Geometry2,
            typename boost::range_value<IntersectionPoints>::type
        >::segment_intersection_strategy_type segment_intersection_strategy_type;

    boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::get_intersection_points_reversed
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                is_multi<Geometry1>::type::value,
                is_multi<Geometry2>::type::value,
                ncg1_type,
                ncg2_type,
                IntersectionPoints,
                segment_intersection_strategy_type
            >,
            dispatch::get_intersection_points
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                is_multi<Geometry1>::type::value,
                is_multi<Geometry2>::type::value,
                ncg1_type,
                ncg2_type,
               IntersectionPoints,
               segment_intersection_strategy_type
            >
        >::type::apply(
            0, geometry1,
            1, geometry2,
            intersection_points);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_GET_INTERSECTION_POINTS_HPP
