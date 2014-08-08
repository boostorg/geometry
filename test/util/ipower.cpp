// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#if defined(DO_NOT_USE_BOOST_TEST)
#  include <cassert>
#else
#  ifndef BOOST_TEST_MODULE
#  define BOOST_TEST_MODULE test_mpl_ipower
#  endif
#  include <boost/test/included/unit_test.hpp>
#endif // DO_NOT_USE_BOOST_TEST


#include <cstddef>

#include <iostream>
#include <limits>

#include <boost/mpl/long.hpp>
#include <boost/mpl/int.hpp>
#include <boost/geometry/util/ipower.hpp>

namespace bg = boost::geometry;


// this is the function implemented with MPL (in non-recursive form)
template <typename T>
inline T ipower(const T& x, std::size_t n)
{
    T z(1);
    if ( n == 0 ) { return z; }

    T y = x;

    while (n > 1)
    {
        if ( n % 2 == 1 )
        {
            z *= y;
        }
        y *= y;
        n /= 2;
    }

    return z * y;
}


template <typename T, std::size_t Power>
struct test_power
{
    static inline void apply()
    {
        typedef typename T::value_type value_type;

        value_type const value = T::value;

        value_type result_from_mpl = bg::util::ipower<T, Power>::value;
        value_type result_from_function = ::ipower(value, Power);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << "pow[function](" << value << ", " << Power << ") = "
                  << result_from_function
                  << std::endl;

        std::cout << "pow[mpl](" << value << ", " << Power << ") = "
                  << result_from_mpl
                  << std::endl;
        std::cout << std::endl;
#endif

#if defined(DO_NOT_USE_BOOST_TEST)
        assert( result_from_mpl == result_from_function );
#else
        BOOST_CHECK( result_from_mpl == result_from_function );
#endif // DO_NOT_USE_BOOST_TEST
    }
};

void test_ipower()
{
    typedef boost::mpl::int_<-1> negative_one;
    typedef boost::mpl::size_t<2> two;
    typedef boost::mpl::long_<4> four;

    test_power<four, 22>::apply();
    test_power<four, 21>::apply();
    test_power<four, 20>::apply();
    test_power<four, 11>::apply();
    test_power<four, 10>::apply();
    test_power<four, 3>::apply();
    test_power<four, 2>::apply();
    test_power<four, 1>::apply();
    test_power<four, 0>::apply();

    test_power<two, 0>::apply();
    test_power<two, 63>::apply();

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    // print out the max possible value for std::size_t
    std::cout << "max value for std::size_t = "
              << std::numeric_limits<std::size_t>::max() << std::endl;
    std::cout << std::endl;
#endif

    test_power<negative_one, 0>::apply();

    test_power<negative_one, 8128>::apply();
#if defined(DO_NOT_USE_BOOST_TEST)
    // test for very high power value; the template depth should not
    // be a problem as we only require log2(exponent) nested templates
    //
    // b2 takes forever to compile the ones below; however, both g++
    // and clang++ when invoked from the command line just do it
    test_power<negative_one, 1000000000>::apply();
    test_power<negative_one, 1000000001>::apply();
    test_power<negative_one, 18446744073709551615lu>::apply();
#endif // DO_NOT_USE_BOOST_TEST

#if false
    // the following does not compile as expected
    test_power<boost::mpl::int_<0>, 0>::apply();
#endif
}


#if defined(DO_NOT_USE_BOOST_TEST)
int main()
{
    test_ipower();
    return 0;
}
#else
BOOST_AUTO_TEST_CASE( test_ipower_all )
{
    test_ipower();
}
#endif // DO_NOT_USE_BOOST_TEST
