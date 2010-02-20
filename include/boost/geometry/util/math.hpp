// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_MATH_HPP
#define BOOST_GEOMETRY_UTIL_MATH_HPP

#include <cmath>
#include <limits>

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/util/select_most_precise.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace math {


template <typename T, bool Floating>
struct equals
{
    static inline bool apply(T const& a, T const& b)
    {
        return a == b;
    }
};

template <typename T>
struct equals<T, true>
{
    static inline bool apply(T const& a, T const& b)
    {
        // See http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.17,
        // FUTURE: replace by some boost tool or boost::test::close_at_tolerance
        return std::abs(a - b) <= std::numeric_limits<T>::epsilon() * std::abs(a);
    }
};


}} // namespace detail::math
#endif

namespace math
{


// Maybe replace this by boost equals or boost ublas numeric equals or so

/*!
    \brief returns true if both arguments are equal.
    \ingroup utility
    \param a first argument
    \param b second argument
    \return true if a == b
    \note If both a and b are of an integral type, comparison is done by ==.
    If one of the types is floating point, comparison is done by abs and
    comparing with epsilon. If one of the types is non-fundamental, it might
    be a high-precision number and comparison is done using the == operator
    of that class.
*/

template <typename T1, typename T2>
inline bool equals(T1 const& a, T2 const& b)
{
    typedef typename select_most_precise<T1, T2>::type select_type;
    return detail::math::equals
        <
            select_type,
            boost::is_floating_point<select_type>::type::value
        >::apply(a, b);
}


// TODO: The idea is to replace "double const globals" with inline
// template functions, so code/data is not generated if not necessary.
// --mloskot
//template <typename T>
//inline T pi()
//{
//   return boost::math::constants::pi<T>();
//}
//
//template <>
//inline double pi<double>()
//{
//   return boost::math::constants::pi<double>();
//}
double const pi = boost::math::constants::pi<double>();
double const two_pi = 2.0 * pi;
double const d2r = pi / 180.0;
double const r2d = 1.0 / d2r;

/*!
    \brief Calculates the haversine of an angle
    \ingroup utility
    \note See http://en.wikipedia.org/wiki/Haversine_formula
    haversin(alpha) = sin2(alpha/2)
*/
template <typename T>
inline T hav(T const& theta)
{
    using boost::math::constants::half;
    T const sn = std::sin(half<T>() * theta);
    return sn * sn;
}

/*!
\brief Short utility to return the square
\ingroup utility
\param value Value to calculate the square from
\return The squared value
*/
template <typename T>
inline T sqr(T const& value)
{
    return value * value;
}

} // namespace math


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_MATH_HPP
