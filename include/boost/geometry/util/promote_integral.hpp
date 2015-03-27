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
#include <cstddef>

#include <boost/mpl/begin.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/size_t.hpp>

#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
#include <boost/multiprecision/cpp_int.hpp>
#endif

#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_unsigned.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace promote_integral
{


template
<
    typename T,
    typename Iterator,
    typename EndIterator,
    std::size_t MinSize
>
struct promote_to_larger
{
    typedef typename boost::mpl::if_c
        <
            (sizeof(typename boost::mpl::deref<Iterator>::type) >= MinSize),
            typename boost::mpl::deref<Iterator>::type,
            typename promote_to_larger
                <
                    T,
                    typename boost::mpl::next<Iterator>::type,
                    EndIterator,
                    MinSize
                >::type
        >::type type;
};

// The following specialization is required to finish the loop over
// all list elements
template <typename T, typename EndIterator, std::size_t MinSize>
struct promote_to_larger<T, EndIterator, EndIterator, MinSize>
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
template
<
    typename T,
    bool PromoteUnsignedToUnsigned = false,
    bool UseCheckedMultiprecisionInteger = false
>
class promote_integral
{
private:
    static bool const is_unsigned = boost::is_unsigned<T>::type::value;

#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
    // Define the proper check policy for the multiprecision integer
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
        >::type check_policy_type;

    // Meta-function to get the multiprecision integer type for the
    // given size and type (signed/unsigned)
    template
    <
        unsigned int Size,
        boost::multiprecision::cpp_integer_type IntegerType
    >
    struct multiprecision_integer_type
    {
        typedef boost::multiprecision::number
            <
                boost::multiprecision::cpp_int_backend
                    <
                        Size,
                        Size,
                        IntegerType,
                        check_policy_type::value,
                        void
                    >
            > type;
    };

    // Meta-function to get the multiprecision signed integer type for
    // the given size
    template <unsigned int Size>
    struct multiprecision_signed_integer_type
    {
        typedef typename multiprecision_integer_type
            <
                Size, boost::multiprecision::signed_magnitude
            >::type type;
    };

    // Meta-function to get the multiprecision unsigned integer type for
    // the given size
    template <unsigned int Size>
    struct multiprecision_unsigned_integer_type
    {
        typedef typename multiprecision_integer_type
            <
                Size, boost::multiprecision::unsigned_magnitude
            >::type type;
    };

    // Define the minimum (and maximum) bit size for the
    // multiprecision integer needed
    // If T is the input type and P the promoted type, then the
    // minimum number of bits for P are (below b stands for the number
    // of bits of T):
    // * if T is unsigned and P is unsigned: 2 * b
    // * if T is signed and P is signed: 2 * b - 1
    // * if T is unsigned and P is signed: 2 * b + 1
    typedef typename boost::mpl::if_c
        <
            (PromoteUnsignedToUnsigned && is_unsigned),
            boost::integral_constant
                <
                    unsigned int, (2 * CHAR_BIT * sizeof(T))
                >,
            boost::integral_constant
                <
                    unsigned int,
                    (2 * CHAR_BIT * sizeof(T) + (is_unsigned ? 1 : -1))
                >
        >::type multiprecision_integer_min_bit_size_type;
#endif

    // Define the minimum size (in bytes) for the promoted type
    typedef typename boost::mpl::if_c
        <
            (PromoteUnsignedToUnsigned && is_unsigned),
            boost::mpl::size_t<(2 * sizeof(T))>,
            boost::mpl::size_t<(2 * sizeof(T) + (is_unsigned ? 1 : -1))>
        >::type min_size_type;

    // Define the list of signed integral types we are goind to use
    // for promotion
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
            , typename multiprecision_signed_integer_type
                <
                    multiprecision_integer_min_bit_size_type::value
                >::type
#endif
        > signed_integral_types;

    // Define the list of unsigned integral types we are goind to use
    // for promotion
    typedef boost::mpl::list
        <
            unsigned short, unsigned int, unsigned long, std::size_t
#if defined(BOOST_HAS_LONG_LONG)
            , boost::ulong_long_type
#endif
#if defined(BOOST_HAS_INT128)
            , boost::uint128_type
#endif
#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
            , typename multiprecision_unsigned_integer_type
                <
                    multiprecision_integer_min_bit_size_type::value
                >::type
#endif
        > unsigned_integral_types;

    // Define the list of integral types that will be used for
    // promotion (depending in whether we was to promote unsigned to
    // unsigned or not)
    typedef typename boost::mpl::if_c
        <
            (is_unsigned && PromoteUnsignedToUnsigned),
            unsigned_integral_types,
            signed_integral_types
        >::type integral_types;

public:
    typedef typename boost::mpl::if_c
        <
            boost::is_integral<T>::type::value,
            typename detail::promote_integral::promote_to_larger
                <
                    T,
                    typename boost::mpl::begin<integral_types>::type,
                    typename boost::mpl::end<integral_types>::type,
                    min_size_type::value
                >::type,
            T
        >::type type;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_PROMOTE_INTEGRAL_HPP
