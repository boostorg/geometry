// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_GET_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_GET_TURNS_HPP


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
#include <boost/geometry/iterators/ever_circling_iterator.hpp>

#include <boost/geometry/strategies/cartesian/cart_intersect.hpp>
#include <boost/geometry/strategies/intersection.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>

#include <boost/geometry/algorithms/detail/sections/get_full_section.hpp>


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
namespace detail { namespace get_turns {



template
<
    typename Geometry1, typename Geometry2,
    typename Section1, typename Section2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
class get_turns_in_sections
{

public :
    static inline void apply(
            int source_id1, Geometry1 const& geometry1,
                Section1 const& sec1,
            int source_id2, Geometry2 const& geometry2,
                Section2 const& sec2,
            bool return_if_found,
            TurnCollection& turns,
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

        range1_iterator prev1, it1, end1;
        range1_iterator begin_range_1, end_range_1;

        ever_circling_iterator<range1_iterator> next1
            = start_at_section(sec1, prev1, it1, end1, begin_range_1, end_range_1,
                    index1, ndi1, geometry1, dir1, sec2.bounding_box);

        // Walk through section and stop if we exceed the other box
        for (prev1 = it1++, next1++;
            it1 != end1 && ! exceeding<0>(dir1, *prev1, sec2.bounding_box);
            prev1 = it1++, index1++, next1++, ndi1++)
        {
            ever_circling_iterator<range1_iterator> nd_next1(
                    begin_range_1, end_range_1, next1, true);
            advance_to_non_duplicate_next(nd_next1, it1, sec1);

            int index2 = sec2.begin_index;
            int ndi2 = sec2.non_duplicate_index;

            range2_iterator prev2, it2, end2;
            range2_iterator begin_range_2, end_range_2;

            ever_circling_iterator<range2_iterator> next2 =
                    start_at_section(sec2, prev2, it2, end2, begin_range_2, end_range_2,
                        index2, ndi2, geometry2, dir2, sec1.bounding_box);

            for (prev2 = it2++, next2++;
                it2 != end2 && ! exceeding<0>(dir2, *prev2, sec1.bounding_box);
                prev2 = it2++, index2++, next2++, ndi2++)
            {
                bool skip = same_source;
                if (skip)
                {
                    // If sources are the same (possibly self-intersecting):
                    // skip if it is a neighbouring sement.
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
                    // Move to the "non duplicate next"
                    ever_circling_iterator<range2_iterator> nd_next2(
                            begin_range_2, end_range_2, next2, true);
                    advance_to_non_duplicate_next(nd_next2, it2, sec2);

                    typedef typename boost::range_value<TurnCollection>::type turn_info;
                    typedef typename turn_info::point_type ip;

                    turn_info ti;
                    ti.operations[0].seg_id = segment_identifier(source_id1,
                                        sec1.multi_index, sec1.ring_index, index1),
                    ti.operations[1].seg_id = segment_identifier(source_id2,
                                        sec2.multi_index, sec2.ring_index, index2),

                    ti.operations[0].other_id = ti.operations[1].seg_id;
                    ti.operations[1].other_id = ti.operations[0].seg_id;

                    detail::overlay::get_turn_info
                        <
                            point1_type,
                            point2_type,
                            turn_info,
                            AssignPolicy
                        >::apply(*prev1, *it1, *nd_next1, *prev2, *it2, *nd_next2,
                            ti, std::back_inserter(turns));

                    if (return_if_found
                        && boost::size(turns) > 0)
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

    template <typename Iterator, typename RangeIterator, typename Section>
    static inline void advance_to_non_duplicate_next(Iterator& next,
            RangeIterator const& it, Section const& section)
    {
        // To see where the next segments bend to, in case of touch/intersections
        // on end points, we need (in case of degenerate/duplicate points) an extra
        // iterator which moves to the REAL next point, so non duplicate.
        // This needs an extra comparison (disjoint).
        // (Note that within sections, non duplicate points are already asserted,
        //   by the sectionalize process).

        // So advance to the "non duplicate next"
        // (the check is defensive, to avoid endless loops)
        std::size_t check = 0;
        while(! detail::disjoint::disjoint_point_point(*it, *next)
            && check++ < section.range_count)
        {
            next++;
        }
    }

    // It is NOT possible to have section-iterators here
    // because of the logistics of "index" (the section-iterator automatically
    // skips to the begin-point, we loose the index or have to recalculate it)
    // So we mimic it here
    template <typename RangeIterator, typename Section, typename Geometry, typename Box>
    static inline ever_circling_iterator<RangeIterator> start_at_section(Section & section,
            RangeIterator& it, RangeIterator& prev, RangeIterator& end,
            RangeIterator& begin_range_it, RangeIterator& end_range_it,
            int& index, int& ndi,
            Geometry const& geometry,
            int dir, Box const& other_bounding_box)
    {
        get_section(geometry, section, it, end);

        // Mimic section-iterator:
        // Skip to point such that section interects other box
        prev = it++;
        for(; it != end && preceding<0>(dir, *it, other_bounding_box);
            prev = it++, index++, ndi++)
        {}
        // Go back one step because we want to start completely preceding
        it = prev;

        get_full_section(geometry, section, begin_range_it, end_range_it);

        ever_circling_iterator<RangeIterator> next(begin_range_it, end_range_it, it, true);
        next++;
        return next;
    }
};


template
<
    typename Ring, typename Box,
    typename Section1, typename Section2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
class get_turns_range_box
{
public :
    static inline void apply(
            int source_id1, Ring const& ring,
            int source_id2, Box const& box,
            Section1 const& sec1, Section2 const& sec2,
            TurnCollection& turns, bool& trivial)
    {
        get_turns_in_sections
            <
                Ring,
                Box,
                Section1,
                Section2,
                TurnCollection,
                IntersectionStrategy,
                AssignPolicy
            >
            ::apply(
                source_id1, ring, sec1,
                source_id2, box, sec2,
                false,
                turns, trivial);
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
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
class get_turns_generic
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
            int source_id1, Geometry1 const& geometry1,
            int source_id2, Geometry2 const& geometry2,
            TurnCollection& turns,
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
                        get_turns_in_sections
                        <
                            Geometry1,
                            Geometry2,
                            typename boost::range_value<Sections1>::type,
                            typename boost::range_value<Sections2>::type,
                            TurnCollection,
                            IntersectionStrategy,
                            AssignPolicy
                        >::apply(
                                source_id1, geometry1, *it1,
                                source_id2, geometry2, *it2,
                                false,
                                turns, trivial);
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
            int source_id1, Geometry1 const& geometry1,
            int source_id2, Geometry2 const& geometry2,
            TurnCollection& turns,

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
                            turns, sec1, sec2, map, trivial);
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
                source_id2, geometry2, turns,
                lower, lower1, lower2, map, trivial, iteration + 1, n);

        divide_and_conquer<1 - Dimension>(source_id1, geometry1,
                source_id2, geometry2, turns,
                upper, upper1, upper2, map, trivial, iteration + 1, n);
    }

public:
    static inline bool apply(
            int source_id1, Geometry1 const& geometry1,
            int source_id2, Geometry2 const& geometry2,
            TurnCollection& turns)
    {
        // Create monotonic sections in ONE direction
        // - in most cases ONE direction is faster (e.g. ~1% faster for the NLP4 testset)
        // - the sections now have a limit (default 10) so will not be too large

        // Note that the sections contain boxes, are dynamic, and therefore
        // are specified using output/intersection-point-type
        // (to enable input-pointer-point-types)
        typedef typename boost::range_value<TurnCollection>::type ip_type;
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
            turns, box, sec1, sec2, map, trivial);
        return trivial;
    }
};


// Get turns for something with a box, following Cohen-Sutherland (cs) approach
template
<
    typename Range,
    typename Box,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns_cs
{
    static inline void apply(int source_id1, Range const& range,
            int multi_index, int ring_index,
            int source_id2, Box const& box,
            TurnCollection& turns,
            bool& trivial)
    {
        if (boost::size(range) <= 1)
        {
            return;
        }


        typedef typename geometry::point_type<Box>::type box_point_type;
        typedef typename geometry::point_type<Range>::type point_type;

        point_type lower_left, upper_left, lower_right, upper_right;
        assign_box_corners(box, lower_left, lower_right, upper_left, upper_right);

        /*
        box_segment_type left(lower_left, upper_left);
        box_segment_type top(upper_left, upper_right);
        box_segment_type right(upper_right, lower_right);
        box_segment_type bottom(lower_right, lower_left);
        */

        typedef typename boost::range_iterator
            <Range const>::type iterator_type;
        iterator_type it = boost::begin(range);

        ever_circling_iterator<iterator_type> next(boost::begin(range),
            boost::end(range), it, true);
        next++;
        next++;

        bool first = true;

        char previous_side[2] = {0, 0};

        int index = 0;

        for (iterator_type prev = it++;
            it != boost::end(range);
            prev = it++, next++, index++)
        {
            segment_identifier seg_id(source_id1,
                        multi_index, ring_index, index);

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
                typedef typename boost::range_value<TurnCollection>::type turn_info;
                typedef detail::overlay::get_turn_info
                    <
                        box_point_type,
                        point_type,
                        turn_info,
                        AssignPolicy
                    > relater;

                // Depending on code some relations can be left out
                turn_info ti;
                ti.operations[0].seg_id = seg_id;
                ti.operations[1].other_id = ti.operations[0].seg_id;

                ti.operations[1].seg_id = segment_identifier(source_id2, -1, -1, 0);
                ti.operations[0].other_id = ti.operations[1].seg_id;
                relater::apply(*prev, *it, *next,
                        lower_left, upper_left, upper_right,
                        ti, std::back_inserter(turns));

                ti.operations[1].seg_id = segment_identifier(source_id2, -1, -1, 1);
                ti.operations[0].other_id = ti.operations[1].seg_id;
                relater::apply(*prev, *it, *next,
                        upper_left, upper_right, lower_right,
                        ti, std::back_inserter(turns));

                ti.operations[1].seg_id = segment_identifier(source_id2, -1, -1, 2);
                ti.operations[0].other_id = ti.operations[1].seg_id;
                relater::apply(*prev, *it, *next,
                        upper_right, lower_right, lower_left,
                        ti, std::back_inserter(turns));

                ti.operations[1].seg_id = segment_identifier(source_id2, -1, -1, 3);
                ti.operations[0].other_id = ti.operations[1].seg_id;
                relater::apply(*prev, *it, *next,
                        lower_right, lower_left, upper_left,
                        ti, std::back_inserter(turns));

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


}} // namespace detail::get_turns
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag1, typename GeometryTag2,
    bool IsMulti1, bool IsMulti2,
    typename Geometry1, typename Geometry2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
{
};


template
<
    typename Polygon,
    typename Box,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
    <
        polygon_tag, box_tag, false, false,
        Polygon, Box,
        TurnCollection,
        IntersectionStrategy,
        AssignPolicy
    >
{

    static inline bool apply(
            int source_id1, Polygon const& polygon,
            int source_id2, Box const& box,
            TurnCollection& turns)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;

        typedef typename boost::range_const_iterator
            <
                typename interior_type<Polygon>::type
            >::type iterator_type;


        typedef detail::get_turns::get_turns_cs
            <
                ring_type,
                Box,
                TurnCollection,
                IntersectionStrategy,
                AssignPolicy
            > intersector_type;

        bool trivial = true;
        intersector_type::apply(
                source_id1, geometry::exterior_ring(polygon), -1, -1,
                source_id2, box,
                turns, trivial);

        int i = 0;
        for (iterator_type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it, ++i)
        {
            intersector_type::apply(
                    source_id1, *it, -1, i,
                    source_id2, box, turns, trivial);
        }

        return trivial;
    }
};


template
<
    typename Ring,
    typename Box,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
    <
        ring_tag, box_tag, false, false,
        Ring, Box,
        TurnCollection,
        IntersectionStrategy,
        AssignPolicy
    >
{
    static inline bool apply(
            int source_id1, Ring const& ring,
            int source_id2, Box const& box,
            TurnCollection& turns)
    {
        typedef typename boost::range_const_iterator
            <
                Ring
            >::type iterator_type;

        typedef detail::get_turns::get_turns_cs
            <
                Ring,
                Box,
                TurnCollection,
                IntersectionStrategy,
                AssignPolicy
            > intersector_type;

        bool trivial = true;
        intersector_type::apply(
                source_id1, ring, -1, -1,
                source_id2, box,
                turns, trivial);

        return trivial;
    }
};


template
<
    typename Ring1,
    typename Ring2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
    <
        ring_tag, ring_tag, false, false,
        Ring1, Ring2,
        TurnCollection, IntersectionStrategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            Ring1,
            Ring2,
            TurnCollection,
            IntersectionStrategy,
            AssignPolicy
        >
{};


template
<
    typename Polygon1,
    typename Polygon2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
    <
        polygon_tag, polygon_tag, false, false,
        Polygon1, Polygon2,
        TurnCollection, IntersectionStrategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            Polygon1,
            Polygon2,
            TurnCollection,
            IntersectionStrategy,
            AssignPolicy
        >
{};

template
<
    typename Polygon,
    typename Ring,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
    <
        polygon_tag, ring_tag, false, false,
        Polygon, Ring,
        TurnCollection, IntersectionStrategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            Polygon,
            Ring,
            TurnCollection,
            IntersectionStrategy,
            AssignPolicy
        >
{};

template
<
    typename LineString1,
    typename LineString2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
    <
        linestring_tag, linestring_tag, false, false,
        LineString1, LineString2,
        TurnCollection, IntersectionStrategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            LineString1,
            LineString2,
            TurnCollection,
            IntersectionStrategy,
            AssignPolicy
        >
{};

template
<
    typename GeometryTag1, typename GeometryTag2,
    bool IsMulti1, bool IsMulti2,
    typename Geometry1, typename Geometry2,
    typename TurnCollection,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns_reversed
{
    static inline bool apply(
            int source_id1, Geometry1 const& g1,
            int source_id2, Geometry2 const& g2,
            TurnCollection& turns)
    {
        return get_turns
            <
                GeometryTag2, GeometryTag1,
                IsMulti2, IsMulti1,
                Geometry2, Geometry1,
                TurnCollection, IntersectionStrategy,
                AssignPolicy
            >::apply(source_id2, g2, source_id1, g1, turns);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



/*!
    \brief Calculate intersection points of two geometries
    \ingroup overlay
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam TurnCollection type of turn-container (e.g. vector of "intersection/turn point"'s)
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param turns container which will contain intersection points
 */
template
<
    typename AssignPolicy,
    typename Geometry1,
    typename Geometry2,
    typename TurnCollection
>
inline void get_turns(Geometry1 const& geometry1,
            Geometry2 const& geometry2, TurnCollection& turns)
{
    concept::check_concepts_and_equal_dimensions<const Geometry1, const Geometry2>();

    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    typedef typename strategy_intersection
        <
            typename cs_tag<Geometry1>::type,
            Geometry1,
            Geometry2,
            typename boost::range_value<TurnCollection>::type
        >::segment_intersection_strategy_type segment_intersection_strategy_type;

    boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::get_turns_reversed
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                is_multi<Geometry1>::type::value,
                is_multi<Geometry2>::type::value,
                ncg1_type,
                ncg2_type,
                TurnCollection,
                segment_intersection_strategy_type,
                AssignPolicy
            >,
            dispatch::get_turns
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                is_multi<Geometry1>::type::value,
                is_multi<Geometry2>::type::value,
                ncg1_type,
                ncg2_type,
                TurnCollection,
                segment_intersection_strategy_type,
                AssignPolicy
            >
        >::type::apply(
            0, geometry1,
            1, geometry2,
            turns);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_GET_TURNS_HPP
