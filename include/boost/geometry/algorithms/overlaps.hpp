// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAPS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAPS_HPP

/*!
\defgroup overlaps overlaps: detect overlap between two geometries
\par Source descriptions:
- Egenhofer: Two objects overlap if they have common interior faces and the bounding faces have common parts
with the opposite interior faces.

\par Geometries:
- \b box + \b box

*/


#include <boost/geometry/core/access.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlaps {

template
<
    typename Box1,
    typename Box2,
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct box_box_loop
{
    static inline void apply(Box1 const& b1, Box2 const& b2,
            bool& overlaps, bool& one_in_two, bool& two_in_one)
    {
        assert_dimension_equal<Box1, Box2>();

        typedef typename coordinate_type<Box1>::type coordinate_type1;
        typedef typename coordinate_type<Box2>::type coordinate_type2;

        coordinate_type1 const& min1 = get<min_corner, Dimension>(b1);
        coordinate_type1 const& max1 = get<max_corner, Dimension>(b1);
        coordinate_type2 const& min2 = get<min_corner, Dimension>(b2);
        coordinate_type2 const& max2 = get<max_corner, Dimension>(b2);

        // We might use the (not yet accepted) Boost.Interval
        // submission in the future

        // If:
        // B1: |-------|
        // B2:           |------|
        // in any dimension -> no overlap
        if (max1 <= min2 || min1 >= max2)
        {
            overlaps = false;
            return;
        }

        // If:
        // B1: |--------------------|
        // B2:   |-------------|
        // in all dimensions -> within, then no overlap
        // B1: |--------------------|
        // B2: |-------------|
        // this is "within-touch" -> then no overlap. So use < and >
        if (min1 < min2 || max1 > max2)
        {
            one_in_two = false;
        }
        // Same other way round
        if (min2 < min1 || max2 > max1)
        {
            two_in_one = false;
        }

        box_box_loop
            <
                Box1,
                Box2,
                Dimension + 1,
                DimensionCount
            >::apply(b1, b2, overlaps, one_in_two, two_in_one);
    }
};

template
<
    typename Box1,
    typename Box2,
    std::size_t DimensionCount
>
struct box_box_loop<Box1, Box2, DimensionCount, DimensionCount>
{
    static inline void apply(Box1 const& , Box2 const&, bool&, bool&, bool&)
    {
    }
};

template
<
    typename Box1,
    typename Box2
>
struct box_box
{
    static inline bool apply(Box1 const& b1, Box2 const& b2)
    {
        bool overlaps = true;
        bool within1 = true;
        bool within2 = true;
        box_box_loop
            <
                Box1,
                Box2,
                0,
                dimension<Box1>::type::value
            >::apply(b1, b2, overlaps, within1, within2);

        /*
        \see http://docs.codehaus.org/display/GEOTDOC/02+Geometry+Relationships#02GeometryRelationships-Overlaps
        where is stated that "inside" is not an "overlap",
        this is true and is implemented as such.
        */
        return overlaps && ! within1 && ! within2;
    }
};



}} // namespace detail::overlaps
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1,
    typename Tag2,
    typename Geometry1,
    typename Geometry2
>
struct overlaps
{};


template <typename Box1, typename Box2>
struct overlaps<box_tag, box_tag, Box1, Box2>
    : detail::overlaps::box_box<Box1, Box2>
{};




} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Determines overlap between two geometries
    \ingroup overlaps
    \return true if there is overlap
 */
template <typename Geometry1, typename Geometry2>
inline bool overlaps(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    return dispatch::overlaps
        <
            typename tag<Geometry1>::type,
            typename tag<Geometry2>::type,
            Geometry1,
            Geometry2
        >::apply(geometry1, geometry2);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAPS_HPP
