// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_EQUALS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_EQUALS_HPP

/*!
\defgroup equals equals: detect if two geometries are spatially equal
\details Equals returns true if geometries are spatially equal. Spatially equal
means including the same point-set. A linestring can be spatially equal to
another linestring, even if both do not have the same amount of points.
A polygon can be spatially equal to a multi-polygon (which then has
only one element).

\par Geometries:
- \b point + \b point
- \b box + \b box

\note There is a functor \ref compare "compare" as well, which can be used
for std:: library compatibility.

*/


#include <cstddef>
#include <deque>

#include <boost/mpl/if.hpp>
#include <boost/static_assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/is_multi.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/algorithms/detail/not.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/util/math.hpp>



namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace equals 
{


template
<
    typename Box1,
    typename Box2,
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct box_box
{
    static inline bool apply(Box1 const& box1, Box2 const& box2)
    {
        if (!geometry::math::equals(get<min_corner, Dimension>(box1), get<min_corner, Dimension>(box2))
            || !geometry::math::equals(get<max_corner, Dimension>(box1), get<max_corner, Dimension>(box2)))
        {
            return false;
        }
        return box_box<Box1, Box2, Dimension + 1, DimensionCount>::apply(box1, box2);
    }
};

template <typename Box1, typename Box2, std::size_t DimensionCount>
struct box_box<Box1, Box2, DimensionCount, DimensionCount>
{
    static inline bool apply(Box1 const& , Box2 const& )
    {
        return true;
    }
};

struct equals_interrupt_policy
{
    static bool const enabled = true;

    // As soon as a turn is detected, this flag is set to true
    // and the process of getting turns (intersection points)
    // is interrupted
    bool turns_inside_or_outside;

    inline equals_interrupt_policy()
        : turns_inside_or_outside(false)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        for (typename boost::range_iterator<Range const>::type
            it = boost::begin(range);
            it != boost::end(range);
            ++it)
        {
            if (it->method == detail::overlay::method_collinear
                || it->method == detail::overlay::method_equal
                )
            {
                typedef typename boost::range_value<Range>::type turn_type;
                // If it is not such that both turns are collinear, the rings are not equal
                for (typename boost::range_iterator
                        <
                            typename turn_type::container_type const
                        >::type oit = boost::begin(it->operations);
                    oit != boost::end(it->operations);
                    oit++)
                {
                    if (oit->operation != detail::overlay::operation_continue)
                    {
                        turns_inside_or_outside = true;
                        return true;
                    }
                }
            }
            else
            {
                turns_inside_or_outside = true;
                return true;
            }
        }
        // It is not yet known, so don't interrupt
        return false;
    }

};



template <typename Ring1, typename Ring2>
struct ring_ring
{


    static inline bool apply(Ring1 const& ring1, Ring2 const& ring2, bool check_area = true)
    {
        // Note: this implementation makes use of getting interections (turns)
        // and merge them. If all IP's disappear, the ring should be the same
        // (because merging removes collinear or non-collinear
        // IP's following the same path)

        // However, this implementation should be re-done using
        // a linear time algorithm (getting left-most points of both
        // and follow them using circular iterator and distance/side)

        // obvious check, area's should be the same.
        if (check_area && geometry::area(ring1) != geometry::area(ring2))
        {
            return false;
        }
        // We could check more (perimeter,centroid,envelope)
        // For now we go directly to intersection points

        typedef typename geometry::point_type<Ring1>::type point_type;
        typedef detail::overlay::traversal_turn_info<point_type> turn_info;
        std::vector<turn_info> turns;

        equals_interrupt_policy policy;

        boost::geometry::get_turns
            <
                detail::overlay::assign_null_policy
            >(ring1, ring2, turns, policy);

        return turns.size() > 0
            && ! policy.turns_inside_or_outside;
    }
};


template <typename T>
struct equal_sortable
{
    int index;
    T area;
    bool counterpart_found;
    inline equal_sortable(int i, T const& a)
        : index(i)
        , area(a)
        , counterpart_found(false)
    {}

    inline bool operator<(equal_sortable<T> const& other) const
    {
        return area < other.area;
    }
};


template <typename Range, typename Collection>
inline void fill_equal_sortable(Range const& range,
            Collection& v)
{
    typedef typename boost::range_value<Collection>::type item_type;
    int i = 0;
    for (typename boost::range_iterator<Range const>::type
            it = boost::begin(range);
         it != boost::end(range);
         ++it, ++i)
    {
        v.push_back(item_type(i, geometry::area(*it)));
    }
    std::sort(v.begin(), v.end());
    /***
    for (typename std::vector<sortable>::iterator it = v.begin();
        it != v.end();
        ++it)
    {
        std::cout << "Ring: " << it->index << " area: " << it->area << std::endl;
    }
    ***/
}


template
<
    typename Policy,
    typename Range1,
    typename Range2
>
inline bool range_range(Range1 const& range1, Range2 const& range2)
{
    typedef typename boost::range_value<Range1>::type geometry1;
    // Check interior rings -> first sort them on area ,
    // for performance reasons (area is not re-calculated in ring-compare)
    typedef std::vector
        <
            equal_sortable<typename geometry::area_result<geometry1>::type>
        > collection;

    collection sorted1, sorted2;

    fill_equal_sortable(range1, sorted1);
    fill_equal_sortable(range2, sorted2);

    typedef typename boost::range_iterator<collection>::type iterator;

    // Compare all items (e.g. rings having equal area)
    for (iterator it1 = sorted1.begin(); it1 != sorted1.end(); ++it1)
    {
        for (iterator it2 = sorted2.begin();
            ! it1->counterpart_found
                && it2 != sorted2.end()
                && it2->area <= it1->area; //+epsilon
            ++it2)
        {
            if (! it2->counterpart_found
                && geometry::math::equals(it1->area, it2->area))
            {
                // Call policy, without checking area
                if (Policy::apply(range1[it1->index], range2[it2->index], false))
                {
                    it1->counterpart_found = true;
                    it2->counterpart_found = true;
                }
            }
        }

        if (! it1->counterpart_found)
        {
            return false;
        }
    }
    return true;
}


template <typename Polygon1, typename Polygon2>
struct polygon_polygon
{
    static inline bool apply(Polygon1 const& polygon1, Polygon2 const& polygon2,
                    bool compare_area = false)
    {
        // Check number of rings (area check is done in exterior ring check)
        if (geometry::num_interior_rings(polygon1) != geometry::num_interior_rings(polygon2))
        {
            return false;
        }

        typedef typename geometry::ring_type<Polygon1>::type ring_type1;
        typedef typename geometry::ring_type<Polygon2>::type ring_type2;
        typedef ring_ring<ring_type1, ring_type2> compare;

        // Check exterior ring
        if (! compare::apply(exterior_ring(polygon1), exterior_ring(polygon2)))
        {
            return false;
        }

        return range_range<compare>(
                interior_rings(polygon1),
                interior_rings(polygon2));

    }
};


template <typename Polygon, typename Ring>
struct polygon_ring
{
    static inline bool apply(Polygon const& polygon, Ring const& ring)
    {
        // A polygon can be equal to a ring if there are no interior rings
        return boost::size(interior_rings(polygon)) == 0
            ? ring_ring
                <
                    typename geometry::ring_type<Polygon>::type,
                    Ring
                >::apply(exterior_ring(polygon), ring)
            : false;
    }
};


}} // namespace detail::equals
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1, typename Tag2,
    bool IsMulti1, bool IsMulti2,
    typename Geometry1,
    typename Geometry2,
    std::size_t DimensionCount
>
struct equals
{};


template <typename P1, typename P2, std::size_t DimensionCount>
struct equals<point_tag, point_tag, false, false, P1, P2, DimensionCount>
    : geometry::detail::not_
        <
            P1,
            P2,
            detail::disjoint::point_point<P1, P2, 0, DimensionCount>
        >
{};


template <typename Box1, typename Box2, std::size_t DimensionCount>
struct equals<box_tag, box_tag, false, false, Box1, Box2, DimensionCount>
    : detail::equals::box_box<Box1, Box2, 0, DimensionCount>
{};


template <typename Ring1, typename Ring2>
struct equals<ring_tag, ring_tag, false, false, Ring1, Ring2, 2>
    : detail::equals::ring_ring<Ring1, Ring2>
{};


template <typename Polygon1, typename Polygon2>
struct equals<polygon_tag, polygon_tag, false, false, Polygon1, Polygon2, 2>
    : detail::equals::polygon_polygon<Polygon1, Polygon2>
{};


template <typename Polygon, typename Ring>
struct equals<polygon_tag, ring_tag, false, false, Polygon, Ring, 2>
    : detail::equals::polygon_ring<Polygon, Ring>
{};


template
<
    typename Tag1, typename Tag2,
    bool IsMulti1, bool IsMulti2,
    typename Geometry1,
    typename Geometry2,
    std::size_t DimensionCount
>
struct equals_reversed
{
    static inline bool apply(Geometry1 const& g1, Geometry2 const& g2)
    {
        return equals
            <
                Tag2, Tag1,
                IsMulti2, IsMulti1,
                Geometry2, Geometry1,
                DimensionCount
            >::apply(g2, g1);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Detect if two geometries are spatially equal
    \ingroup equals
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if geometries are spatially equal, else false
 */
template <typename Geometry1, typename Geometry2>
inline bool equals(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check_concepts_and_equal_dimensions
        <
            Geometry1 const,
            Geometry2 const
        >();

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::equals_reversed
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                is_multi<Geometry1>::type::value,
                is_multi<Geometry2>::type::value,
                Geometry1,
                Geometry2,
                dimension<Geometry1>::type::value
            >,
            dispatch::equals
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                is_multi<Geometry1>::type::value,
                is_multi<Geometry2>::type::value,
                Geometry1,
                Geometry2,
                dimension<Geometry1>::type::value
            >
        >::type::apply(geometry1, geometry2);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_EQUALS_HPP

