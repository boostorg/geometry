// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_promote_integral
#endif

#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
#include <climits>
#endif
#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include <boost/config.hpp>
#include <boost/type_traits/is_same.hpp>

#include <geometry_test_common.hpp>

#include <boost/geometry/util/promote_integral.hpp>

#include <boost/multiprecision/cpp_int.hpp>

#if defined(BOOST_GEOMETRY_TEST_DEBUG) && defined(BOOST_HAS_INT128)
std::ostream& operator<<(std::ostream& os, boost::int128_type i)
{
    os << double(i);
    return os;
}
#endif


namespace bg = boost::geometry;

template <typename Type, typename ExpectedPromotedType>
inline void test_promote_integral()
{
    typedef typename bg::promote_integral<Type>::type promoted_integral_type;
    bool const same_types = boost::is_same
        <
            promoted_integral_type, ExpectedPromotedType
        >::type::value;

    BOOST_CHECK(same_types);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "type : " << typeid(Type).name()
              << ", sizeof: " << sizeof(Type)
              << ", max value: "
              << std::numeric_limits<Type>::max()
              << std::endl;
    std::cout << "detected promoted type : "
              << typeid(promoted_integral_type).name()
              << ", sizeof: " << sizeof(promoted_integral_type)
              << ", max value: "
              << std::numeric_limits<promoted_integral_type>::max()
              << std::endl;
    std::cout << "expected promoted type : "
              << typeid(ExpectedPromotedType).name()
              << ", sizeof: " << sizeof(ExpectedPromotedType)
              << ", max value: "
              << std::numeric_limits<ExpectedPromotedType>::max()
              << std::endl;
    std::cout << std::endl;
#endif
}

template <typename T>
void test_promotion()
{
    if (sizeof(short) >= 2 * sizeof(T))
    {
        test_promote_integral<T, short>();
    }
    else if (sizeof(int) >= 2 * sizeof(T))
    {
        test_promote_integral<T, int>();
    }
    else if (sizeof(long) >= 2 * sizeof(T))
    {
        test_promote_integral<T, long>();
    }
#if defined(BOOST_HAS_LONG_LONG)
    else if (sizeof(boost::long_long_type) >= 2 * sizeof(T))
    {
        test_promote_integral<T, boost::long_long_type>();
    }
#endif
#if defined(BOOST_HAS_INT128)
    else if (sizeof(boost::int128_type) >= 2 * sizeof(T))
    {
        test_promote_integral<T, boost::int128_type>();
    }
#endif
    else
    {
#if !defined(BOOST_GEOMETRY_NO_MULTIPRECISION_INTEGER)
        namespace bm = boost::multiprecision;
        typedef bm::number
            <
                bm::cpp_int_backend
                    <
                        2 * CHAR_BIT * sizeof(T),
                        2 * CHAR_BIT * sizeof(T),
                        bm::signed_magnitude,
                        bm::unchecked,
                        void
                    >
            > multiprecision_integer_type;

        test_promote_integral<T, multiprecision_integer_type>();
#else
        test_promote_integral<T, T>();
#endif
    }
}


BOOST_AUTO_TEST_CASE( test_char )
{
    test_promotion<char>();
}

BOOST_AUTO_TEST_CASE( test_short )
{
    test_promotion<short>();
}

BOOST_AUTO_TEST_CASE( test_int )
{
    test_promotion<int>();
}

BOOST_AUTO_TEST_CASE( test_long )
{
    test_promotion<long>();
}

#ifdef BOOST_HAS_LONG_LONG
BOOST_AUTO_TEST_CASE( test_long_long )
{
    test_promotion<boost::long_long_type>();
}
#endif

#if defined(BOOST_HAS_INT128)
BOOST_AUTO_TEST_CASE( test_int128 )
{
    test_promotion<boost::int128_type>();
}
#endif

BOOST_AUTO_TEST_CASE( test_custom_types )
{
    namespace bm = boost::multiprecision;
    typedef bm::number
        <
            bm::cpp_int_backend
                <
                    17,
                    17,
                    bm::signed_magnitude,
                    bm::unchecked,
                    void
                >
        > custom_integral_type1;

    typedef bm::number
        <
            bm::cpp_int_backend
                <
                    500,
                    500,
                    bm::signed_magnitude,
                    bm::unchecked,
                    void
                >
        > custom_integral_type2;

    // for user defined number types we do not do any promotion
    test_promote_integral<custom_integral_type1, custom_integral_type1>();
    test_promote_integral<custom_integral_type2, custom_integral_type2>();
}

BOOST_AUTO_TEST_CASE( test_floating_point )
{
    // for floating-point types we do not do any promotion
    test_promote_integral<float, float>();
    test_promote_integral<double, double>();
    test_promote_integral<long double, long double>();

#ifdef HAVE_TTMATH
    test_promote_integral<ttmath_big, ttmath_big>();
#endif
}
