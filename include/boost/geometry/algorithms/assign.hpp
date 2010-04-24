// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_ASSIGN_HPP
#define BOOST_GEOMETRY_ALGORITHMS_ASSIGN_HPP


#include <cstddef>

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


#include <boost/geometry/util/copy.hpp>
#include <boost/geometry/util/for_each_coordinate.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace assign
{

template <typename CoordinateType>
struct assign_operation
{
    inline assign_operation(CoordinateType const& value)
        : m_value(value)
    {}

    template <typename P, std::size_t I>
    inline void apply(P& point) const
    {
        set<I>(point, m_value);
    }

private:

    CoordinateType m_value;
};


/*!
    \brief Assigns all coordinates of a specific point to a value
    \ingroup access
    \details
    \param p Point
    \param value Value which is assigned to all coordinates of point p
 */
template <typename Point>
inline void assign_value(Point& p,
        typename coordinate_type<Point>::type const& value)
{
    for_each_coordinate(p,
            assign_operation<typename coordinate_type<Point>::type>(value));
}



template
<
    typename Box, std::size_t Index,
    std::size_t Dimension, std::size_t DimensionCount
>
struct initialize
{
    typedef typename coordinate_type<Box>::type coordinate_type;

    static inline void apply(Box& box, coordinate_type const& value)
    {
        set<Index, Dimension>(box, value);
        initialize<Box, Index, Dimension + 1, DimensionCount>::apply(box, value);
    }
};


template <typename Box, std::size_t Index, std::size_t DimensionCount>
struct initialize<Box, Index, DimensionCount, DimensionCount>
{
    typedef typename coordinate_type<Box>::type coordinate_type;

    static inline void apply(Box&,  coordinate_type const& )
    {}
};


template <typename Point>
struct assign_zero_point
{
    static inline void apply(Point& point)
    {
        typedef typename coordinate_type<Point>::type coordinate_type;
        assign_value(point, 0);
    }
};


template <typename Box>
struct assign_inverse_box
{
    typedef typename point_type<Box>::type point_type;

    static inline void apply(Box& box)
    {
        typedef typename coordinate_type<point_type>::type coordinate_type;

        typedef typename boost::mpl::if_
            <
                typename boost::is_arithmetic<coordinate_type>::type,
                coordinate_type,
                double
            >::type bound_type;

        initialize
            <
                Box, min_corner, 0, dimension<Box>::type::value
            >::apply(
            box, boost::numeric::bounds<bound_type>::highest());
        initialize
            <
                Box, max_corner, 0, dimension<Box>::type::value
            >::apply(
            box, boost::numeric::bounds<bound_type>::lowest());
    }
};


template <typename Box>
struct assign_zero_box
{
    static inline void apply(Box& box)
    {
        typedef typename coordinate_type<Box>::type coordinate_type;

        initialize
            <
                Box, min_corner, 0, dimension<Box>::type::value
            >::apply(box, coordinate_type());
        initialize
            <
                Box, max_corner, 0, dimension<Box>::type::value
            >::apply(box, coordinate_type());
    }
};


template
<
    std::size_t Corner1, std::size_t Corner2,
    typename Box, typename Point
>
inline void assign_box_2d_corner(Box const& box, Point& point)
{
    // Be sure both are 2-Dimensional
    assert_dimension<Box, 2>();
    assert_dimension<Point, 2>();

    // Copy coordinates
    typedef typename coordinate_type<Point>::type coordinate_type;

    set<0>(point, boost::numeric_cast<coordinate_type>(get<Corner1, 0>(box)));
    set<1>(point, boost::numeric_cast<coordinate_type>(get<Corner2, 1>(box)));
}



template
<
    typename Geometry, typename Point,
    std::size_t Index,
    std::size_t Dimension, std::size_t DimensionCount
>
struct assign_point_to_index
{

    static inline void apply(Point const& point, Geometry& geometry)
    {
        geometry::set<Index, Dimension>(geometry, boost::numeric_cast
            <
                typename coordinate_type<Geometry>::type
            >(geometry::get<Dimension>(point)));

        assign_point_to_index
            <
                Geometry, Point, Index, Dimension + 1, DimensionCount
            >::apply(point, geometry);
    }
};

template
<
    typename Geometry, typename Point,
    std::size_t Index,
    std::size_t DimensionCount
>
struct assign_point_to_index
    <
        Geometry, Point,
        Index,
        DimensionCount, DimensionCount
    >
{
    static inline void apply(Point const& , Geometry& )
    {
    }
};


template
<
    typename Geometry, typename Point,
    std::size_t Index,
    std::size_t Dimension, std::size_t DimensionCount
>
struct assign_point_from_index
{

    static inline void apply(Geometry const& geometry, Point& point)
    {
        geometry::set<Dimension>( point, boost::numeric_cast
            <
                typename coordinate_type<Geometry>::type
            >(geometry::get<Index, Dimension>(geometry)));

        assign_point_from_index
            <
                Geometry, Point, Index, Dimension + 1, DimensionCount
            >::apply(geometry, point);
    }
};

template
<
    typename Geometry, typename Point,
    std::size_t Index,
    std::size_t DimensionCount
>
struct assign_point_from_index
    <
        Geometry, Point,
        Index,
        DimensionCount, DimensionCount
    >
{
    static inline void apply(Geometry const&, Point&)
    {
    }
};



}} // namespace detail::assign
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag, typename Geometry, std::size_t DimensionCount>
struct assign {};

template <typename Point>
struct assign<point_tag, Point, 2>
{
    typedef typename coordinate_type<Point>::type coordinate_type;

    template <typename T>
    static inline void apply(Point& point, T const& c1, T const& c2)
    {
        set<0>(point, boost::numeric_cast<coordinate_type>(c1));
        set<1>(point, boost::numeric_cast<coordinate_type>(c2));
    }
};

template <typename Point>
struct assign<point_tag, Point, 3>
{
    typedef typename coordinate_type<Point>::type coordinate_type;

    template <typename T>
    static inline void apply(Point& point, T const& c1, T const& c2, T const& c3)
    {
        set<0>(point, boost::numeric_cast<coordinate_type>(c1));
        set<1>(point, boost::numeric_cast<coordinate_type>(c2));
        set<2>(point, boost::numeric_cast<coordinate_type>(c3));
    }
};

template <typename Box>
struct assign<box_tag, Box, 2>
{
    typedef typename coordinate_type<Box>::type coordinate_type;

    // Here we assign 4 coordinates to a box.
    // -> Most logical is: x1,y1,x2,y2
    // In case the user reverses x1/x2 or y1/y2, we could reverse them (THAT IS NOT IMPLEMENTED)

    // Note also comment in util/assign_box_corner ->
    //   ("Most logical is LOWER, UPPER and sub-order LEFT, RIGHT")
    //   (That is assigning 4 points from a box. So lower-left, lower-right, upper-left, upper-right)
    template <typename T>
    static inline void apply(Box& box,
                T const& x1, T const& y1, T const& x2, T const& y2)
    {
        set<min_corner, 0>(box, boost::numeric_cast<coordinate_type>(x1));
        set<min_corner, 1>(box, boost::numeric_cast<coordinate_type>(y1));
        set<max_corner, 0>(box, boost::numeric_cast<coordinate_type>(x2));
        set<max_corner, 1>(box, boost::numeric_cast<coordinate_type>(y2));
    }
};



template <typename GeometryTag, typename Geometry>
struct assign_zero {};


template <typename Point>
struct assign_zero<point_tag, Point>
    : detail::assign::assign_zero_point<Point>
{};

template <typename Box>
struct assign_zero<box_tag, Box>
    : detail::assign::assign_zero_box<Box>
{};


template <typename GeometryTag, typename Geometry>
struct assign_inverse {};

template <typename Box>
struct assign_inverse<box_tag, Box>
    : detail::assign::assign_inverse_box<Box>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief assign two values to a 2D point
    \ingroup access
 */
template <typename Geometry, typename Type>
inline void assign(Geometry& geometry, Type const& c1, Type const& c2)
{
    concept::check<Geometry>();

    dispatch::assign
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::dimension<Geometry>::type::value
        >::apply(geometry, c1, c2);
}

/*!
    \brief assign three values to a 3D point [or the center + radius to a circle]
    \ingroup access
 */
template <typename Geometry, typename Type>
inline void assign(Geometry& geometry,
            Type const& c1, Type const& c2, Type const& c3)
{
    concept::check<Geometry>();

    dispatch::assign
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::dimension<Geometry>::type::value
        >::apply(geometry, c1, c2, c3);
}

/*!
    \brief assign center + radius to a sphere [for extension]
    \ingroup access
 */
template <typename Geometry, typename Type>
inline void assign(Geometry& geometry,
                Type const& c1, Type const& c2, Type const& c3, Type const& c4)
{
    concept::check<Geometry>();

    dispatch::assign
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::dimension<Geometry>::type::value
        >::apply(geometry, c1, c2, c3, c4);
}


/*!
    \brief assign a range of points to a linestring, ring or polygon
    \note The point-type of the range might be different from the point-type of the geometry
    \ingroup access
 */
template <typename Geometry, typename Range>
inline void assign(Geometry& geometry, Range const& range)
{
    concept::check<Geometry>();

    clear(geometry);
    geometry::append(geometry, range, -1, 0);
}


/*!
    \brief assign to a box inverse infinite
    \details The assign_inverse function initialize a 2D or 3D box with large coordinates, the
    min corner is very large, the max corner is very small. This is a convenient starting point to
    collect the minimum bounding box of a geometry.
    \ingroup access
 */
template <typename Geometry>
inline void assign_inverse(Geometry& geometry)
{
    concept::check<Geometry>();

    dispatch::assign_inverse
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
}

/*!
    \brief assign zero values to a box, point
    \ingroup access
    \details The assign_zero function initializes a 2D or 3D point or box with coordinates of zero
    \tparam Geometry the geometry type
 */
template <typename Geometry>
inline void assign_zero(Geometry& geometry)
{
    concept::check<Geometry>();

    dispatch::assign_zero
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
}


/*!
    \brief Assign the 4 points of a 2D box
    \ingroup access
    \note The order is crucial. Most logical is LOWER, UPPER and sub-order LEFT, RIGHT
        so this is how it is implemented.
*/
template <typename Box, typename Point>
inline void assign_box_corners(Box const& box,
        Point& lower_left, Point& lower_right,
        Point& upper_left, Point& upper_right)
{
    concept::check<Box const>();
    concept::check<Point>();

    detail::assign::assign_box_2d_corner
            <min_corner, min_corner>(box, lower_left);
    detail::assign::assign_box_2d_corner
            <max_corner, min_corner>(box, lower_right);
    detail::assign::assign_box_2d_corner
            <min_corner, max_corner>(box, upper_left);
    detail::assign::assign_box_2d_corner
            <max_corner, max_corner>(box, upper_right);
}



/*!
    \brief Assign a box or segment with the value of a point
    \ingroup access
    \tparam Index indicates which box-corner, min_corner (0) or max_corner (1)
        or which point of segment (0/1)
*/
template <std::size_t Index, typename Geometry, typename Point>
inline void assign_point_to_index(Point const& point, Geometry& geometry)
{
    concept::check<Point const>();
    concept::check<Geometry>();

    detail::assign::assign_point_to_index
        <
            Geometry, Point, Index, 0, dimension<Geometry>::type::value
        >::apply(point, geometry);
}


/*!
    \brief Assign a point with a point of a box or segment
    \ingroup access
    \tparam Index indicates which box-corner, min_corner (0) or max_corner (1)
        or which point of segment (0/1)
*/
template <std::size_t Index, typename Point, typename Geometry>
inline void assign_point_from_index(Geometry const& geometry, Point& point)
{
    concept::check<Geometry const>();
    concept::check<Point>();

    detail::assign::assign_point_from_index
        <
            Geometry, Point, Index, 0, dimension<Geometry>::type::value
        >::apply(geometry, point);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_ASSIGN_HPP
