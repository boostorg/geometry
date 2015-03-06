// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_TEST_MODULE)
# define BOOST_TEST_MODULE Boost.Geometry Util compress_variant
#endif

#include <geometry_test_common.hpp>

#include <boost/geometry/util/compress_variant.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/variant/variant.hpp>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(compress_variant)

template <typename ExpectedTypes, BOOST_VARIANT_ENUM_PARAMS(typename T)>
void check_variant_types(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>)
{
    BOOST_MPL_ASSERT((
        boost::mpl::equal<
            typename boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types,
            ExpectedTypes
        >
    ));
}

template <typename Variant, typename ExpectedTypes>
void test_variant_result()
{
    check_variant_types<ExpectedTypes>(typename boost::geometry::compress_variant<Variant>::type());
}

template <typename Variant, typename ExpectedType>
void test_single_type_result()
{
    BOOST_MPL_ASSERT((
        boost::is_same<
            typename boost::geometry::compress_variant<Variant>::type,
            ExpectedType
        >
    ));
}

BOOST_AUTO_TEST_CASE(multiple_type)
{
    test_variant_result<
        boost::variant<int, float, double>,
        boost::mpl::vector < int, float, double >
    >();

    test_variant_result<
        boost::variant<int, float, double, int, int, float, double, double, float>,
        boost::mpl::vector < int, double, float >
    >();
}

BOOST_AUTO_TEST_CASE(single_type)
{
    test_single_type_result<
        boost::variant<int>,
        int
    >();

    test_single_type_result<
        boost::variant<double, double, double, double, double>,
        double
    >();
}

BOOST_AUTO_TEST_SUITE_END()
