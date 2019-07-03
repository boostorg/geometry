// Boost.Geometry

// Copyright (c) 2018-2019 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ARITHMETIC_GENERAL_FORM_HPP
#define BOOST_GEOMETRY_ARITHMETIC_GENERAL_FORM_HPP

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/config.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

namespace boost { namespace geometry
{

namespace arithmetic
{

//--------------------------------------------------------------------------
// Structure containing the General Form of a line, a*x + b*y + c == 0
// Might be conceptized later. Therefore operations are implemented outside
// the structure itself.
template <typename GeneralType = double>
struct general_form
{
    general_form()
        : a(GeneralType())
        , b(GeneralType())
        , c(GeneralType())
        , normalized(false)
    {}

    // Horizontal: a == 0, for example y-3=0, y==3
    // Vertical: b == 0, for example x-2=0, x==2
    // Through origin: c == 0
    GeneralType a;
    GeneralType b;
    GeneralType c;
    bool normalized;
};

template <typename GeneralType, typename Coordinate>
inline
general_form<GeneralType> construct_line(Coordinate const& x1,
    Coordinate const& y1, Coordinate const& x2, Coordinate const& y2)
{
    general_form<GeneralType> result;
    result.a = y1 - y2;
    result.b = x2 - x1;
    result.c = -result.a * x1 - result.b * y1;
    return result;
}

template <typename T, typename Point>
inline general_form<T> construct_line(Point const& a, Point const& b)
{
    return construct_line<T>(geometry::get<0>(a),
                             geometry::get<1>(a),
                             geometry::get<0>(b),
                             geometry::get<1>(b));
}

template <typename T, typename Segment>
inline general_form<T> construct_line(Segment const& segment)
{
    return construct_line<T>(geometry::get<0, 0>(segment),
                             geometry::get<0, 1>(segment),
                             geometry::get<1, 0>(segment),
                             geometry::get<1, 1>(segment));
}


// Calculates intersection point of two infinite lines.
// Returns true if the lines intersect.
// Returns false if lines are parallel (or collinear, possibly opposite)
template <typename Point, typename GeneralType>
inline
bool get_intersection(Point& ip,
                      general_form<GeneralType> const& p,
                      general_form<GeneralType> const& q)
{
    GeneralType const denominator = p.b * q.a - p.a * q.b;

    static GeneralType const zero = 0;
    if (math::equals(denominator, zero))
    {
        // Lines are parallel
        return false;
    }

    // Calculate the intersection coordinates
    geometry::set<0>(ip, (p.c * q.b - p.b * q.c) / denominator);
    geometry::set<1>(ip, (p.a * q.c - p.c * q.a) / denominator);

    return true;
}

//! Return a distance-side-measure for a point to a line
//! Point is located left of the line if value is positive,
//! right of the line is value is negative, and on the line if the value
//! is exactly zero
template <typename GeneralType, typename CoordinateType>
inline
typename select_most_precise<GeneralType, CoordinateType>::type
side_value(general_form<GeneralType> const& f,
    CoordinateType const& x, CoordinateType const& y)
{
    // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_an_equation
    // Distance from point to line in general form is given as:
    // (a * x + b * y + c) / sqrt(a * a + b * b);
    // In most use cases comparisons are enough, saving the sqrt
    // and often even the division.
    // Also, this gives positive values for points left to the line,
    // and negative values for points right to the line.
    return f.a * x + f.b * y + f.c;
}

template <typename GeneralType, typename Point>
inline
typename select_most_precise
<
    GeneralType,
    typename geometry::coordinate_type<Point>::type
>::type
side_value(general_form<GeneralType> const& f, Point const& p)
{
    return side_value(f, geometry::get<0>(p), geometry::get<1>(p));
}

// Returns true for two lines which are supposed to be (close to) collinear
// (which is not checked) and have a similar direction
// (in practice up to 45 degrees, TO BE VERIFIED)
// true: -----------------> p -----------------> q
// false: -----------------> p <----------------- q
template <typename GeneralType>
inline
bool similar_direction(const general_form<GeneralType>& p,
                       const general_form<GeneralType>& q)
{
    return p.a * q.a >= 0 && p.b * q.b >= 0;
}


} // namespace arithmetic


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ARITHMETIC_GENERAL_FORM_HPP
