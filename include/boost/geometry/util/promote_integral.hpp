// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_PROMOTE_INTEGRAL_HPP
#define BOOST_GEOMETRY_UTIL_PROMOTE_INTEGRAL_HPP

// For now deactivate the use of multiprecision integers
// TODO: activate it later
#define BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER

#include <climits>

#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/pop_front.hpp>

#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
#include <boost/multiprecision/cpp_int.hpp>
#endif

#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_integral.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace promote_integral
{

template
<
    typename T,
    typename List,
    bool IsEmpty = boost::mpl::empty<List>::type::value
>
struct promote_to_larger
{
    typedef typename boost::mpl::if_c
        <
            (sizeof(typename boost::mpl::front<List>::type) >= 2 * sizeof(T)),
            typename boost::mpl::front<List>::type,
            typename promote_to_larger
                <
                    T, typename boost::mpl::pop_front<List>::type
                >::type
        >::type type;
};

// The following specialization is required to finish the loop over
// all list elements: the empty MPL list does not support
// boost::mpl::front and boost::mpl::pop_front, and thus without this
// specialization we get a compilation error
template <typename T, typename TypeList>
struct promote_to_larger<T, TypeList, true>
{
    // if promotion fails, keep the number T
    // (and cross fingers that overflow will not occur)
    typedef T type;
};

}} // namespace detail::promote_integral
#endif // DOXYGEN_NO_DETAIL



/*!
    \brief Meta-function to define an integral type with size
    than is at least twice the size of T
    \ingroup utility
    \details
    This meta-function tries to promote the fundamental integral type
    T to a another integral type with size at least twice the size of T.

    To do this, two times the size of T is tested against the sizes of:
         short, int, long, boost::long_long_type, boost::int128_t
    and the one that matches is chosen.

    If the macro BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER is not
    defined, boost's multiprecision integer cpp_int<> is used as a
    last resort.

    If BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER is defined and an
    appropriate type cannot be detected, the input type is returned as is.

    Finally, if the passed type is either a floating-point type or a
    user-defined type it is returned as is.

    \note boost::long_long_type is considered only if the macro
    BOOST_HAS_LONG_LONG is defined

    \note boost::int128_type is considered only if the macro
    BOOST_HAS_INT128 is defined
*/
template <typename T, bool UseCheckedMultiprecisionInteger = false>
class promote_integral
{
private:
#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
    typedef typename boost::mpl::if_c
        <
            UseCheckedMultiprecisionInteger,
            boost::integral_constant
                <
                    boost::multiprecision::cpp_int_check_type,
                    boost::multiprecision::checked
                >,
            boost::integral_constant
                <
                    boost::multiprecision::cpp_int_check_type,
                    boost::multiprecision::unchecked
                >
        >::type checking_policy_type;

    typedef boost::multiprecision::number
        <
            boost::multiprecision::cpp_int_backend
                <
                    2 * CHAR_BIT * sizeof(T),
                    2 * CHAR_BIT * sizeof(T),
                    boost::multiprecision::signed_magnitude,
                    checking_policy_type::value,
                    void
                >
        > multiprecision_integer_type;
#endif

    typedef boost::mpl::list
        <
            short, int, long
#if defined(BOOST_HAS_LONG_LONG)
            , boost::long_long_type
#endif
#if defined(BOOST_HAS_INT128)
            , boost::int128_type
#endif
#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
            , multiprecision_integer_type
#endif
        > integral_types;

public:
    typedef typename boost::mpl::if_c
        <
            boost::is_integral<T>::type::value,
            typename detail::promote_integral::promote_to_larger
                <
                    T, integral_types
                >::type,
            T
        >::type type;
};

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_PROMOTE_INTEGRAL_HPP
