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
#include <boost/mpl/assert.hpp>
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
        geometry::set<I>(point, m_value);
    }

private:

    CoordinateType m_value;
};


/*!
    \brief Assigns all coordinates of a specific point to a value
    \ingroup assign
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
        geometry::set<Index, Dimension>(box, value);
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

    geometry::set<0>(point, boost::numeric_cast<coordinate_type>(get<Corner1, 0>(box)));
    geometry::set<1>(point, boost::numeric_cast<coordinate_type>(get<Corner2, 1>(box)));
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


template <typename Geometry>
struct assign_2d_box_or_segment
{
    typedef typename coordinate_type<Geometry>::type coordinate_type;

    // Here we assign 4 coordinates to a box of segment
    // -> Most logical is: x1,y1,x2,y2
    // In case the user reverses x1/x2 or y1/y2, for a box, we could reverse them (THAT IS NOT IMPLEMENTED)

    template <typename Type>
    static inline void apply(Geometry& geometry,
                Type const& x1, Type const& y1, Type const& x2, Type const& y2)
    {
        geometry::set<0, 0>(geometry, boost::numeric_cast<coordinate_type>(x1));
        geometry::set<0, 1>(geometry, boost::numeric_cast<coordinate_type>(y1));
        geometry::set<1, 0>(geometry, boost::numeric_cast<coordinate_type>(x2));
        geometry::set<1, 1>(geometry, boost::numeric_cast<coordinate_type>(y2));
    }
};


}} // namespace detail::assign
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag, typename Geometry, std::size_t DimensionCount>
struct assign
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};

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
    : detail::assign::assign_2d_box_or_segment<Box>
{};

template <typename Segment>
struct assign<segment_tag, Segment, 2>
    : detail::assign::assign_2d_box_or_segment<Segment>
{};



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
\brief Assign two coordinates to a geometry (usually a 2D point)
\ingroup assign
\tparam Geometry \tparam_geometry
\tparam Type \tparam_numeric to specify the coordinates
\param geometry \param_geometry
\param c1 \param_x
\param c2 \param_y

\qbk{distinguish, 2 coordinate values}
\qbk{
[heading Example]
[assign_2d_point] [assign_2d_point_output]

[heading See also]
\* [link geometry.reference.algorithms.make.make_2_2_coordinate_values make]
}
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
\brief Assign three values to a geometry (usually a 3D point)
\ingroup assign
\tparam Geometry \tparam_geometry
\tparam Type \tparam_numeric to specify the coordinates
\param geometry \param_geometry
\param c1 \param_x
\param c2 \param_y
\param c3 \param_z

\qbk{distinguish, 3 coordinate values}
\qbk{
[heading Example]
[assign_3d_point] [assign_3d_point_output]

[heading See also]
\* [link geometry.reference.algorithms.make.make_3_3_coordinate_values make]
}
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
\brief Assign four values to a geometry (usually a box or segment)
\ingroup assign
\tparam Geometry \tparam_geometry
\tparam Type \tparam_numeric to specify the coordinates
\param geometry \param_geometry
\param c1 First coordinate (usually x1)
\param c2 Second coordinate (usually y1)
\param c3 Third coordinate (usually x2)
\param c4 Fourth coordinate (usually y2)

\qbk{distinguish, 4 coordinate values}
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
\brief Assign a range of points to a linestring, ring or polygon
\note The point-type of the range might be different from the point-type of the geometry
\ingroup assign
\tparam Geometry \tparam_geometry
\tparam Range \tparam_range_point
\param geometry \param_geometry
\param range \param_range_point

\qbk{distinguish, with a range}
\qbk{
[heading Example]
[assign_with_range] [assign_with_range_output]

[heading See also]
\* [link geometry.reference.algorithms.make.make_1_with_a_range make]
}
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
\ingroup assign
\tparam Geometry \tparam_geometry
\param geometry \param_geometry

\qbk{
[heading Example]
[assign_inverse] [assign_inverse_output]

[heading See also]
\* [link geometry.reference.algorithms.make.make_inverse make]
}
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
\ingroup assign
\details The assign_zero function initializes a 2D or 3D point or box with coordinates of zero
\tparam Geometry \tparam_geometry
\param geometry \param_geometry

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
\brief Assign the four points of a 2D box
\ingroup assign
\note The order is crucial. Most logical is LOWER, UPPER and sub-order LEFT, RIGHT
    so this is how it is implemented.
\tparam Box \tparam_box
\tparam Point \tparam_point
\param box \param_box
\param lower_left point being assigned to lower left coordinates of the box
\param lower_right point being assigned to lower right coordinates of the box
\param upper_left point being assigned to upper left coordinates of the box
\param upper_right point being assigned to upper right coordinates of the box

\qbk{
[heading Example]
[assign_box_corners] [assign_box_corners_output]
}
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
\ingroup assign
\tparam Index indicates which box-corner, min_corner (0) or max_corner (1)
    or which point of segment (0/1)
\tparam Point \tparam_point
\tparam Geometry \tparam_box_or_segment
\param point \param_point
\param geometry \param_box_or_segment

\qbk{
[heading Example]
[assign_point_to_index] [assign_point_to_index_output]
}
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
\ingroup assign
\tparam Index indicates which box-corner, min_corner (0) or max_corner (1)
    or which point of segment (0/1)
\tparam Geometry \tparam_box_or_segment
\tparam Point \tparam_point
\param geometry \param_box_or_segment
\param point \param_point

\qbk{
[heading Example]
[assign_point_from_index] [assign_point_from_index_output]
}
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
