// Boost.Geometry
// Unit Test

// Copyright (c) 2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/core/visit.hpp>
#include <boost/geometry/geometries/adapted/boost_variant.hpp>
#include <boost/geometry/geometries/adapted/boost_variant2.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/util/type_traits.hpp>


using point_t = bg::model::point<double, 2, bg::cs::cartesian>;
using linestring_t = bg::model::linestring<point_t>;
using polygon_t = bg::model::polygon<point_t>;

template <typename DynamicGeometry>
void test_all()
{
    DynamicGeometry dg = point_t(1, 2);
    bg::traits::visit<DynamicGeometry>::apply([](auto g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(! std::is_const<decltype(g)>::value);
    }, dg);
    bg::traits::visit<DynamicGeometry>::apply([](auto const g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(std::is_const<decltype(g)>::value);
    }, dg);
    bg::traits::visit<DynamicGeometry>::apply([](auto & g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(! std::is_const<std::remove_reference_t<decltype(g)>>::value);
    }, dg);
    bg::traits::visit<DynamicGeometry>::apply([](auto const& g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(std::is_const<std::remove_reference_t<decltype(g)>>::value);
    }, dg);

    DynamicGeometry const cdg = point_t(3, 4);
    bg::traits::visit<DynamicGeometry>::apply([](auto g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(! std::is_const<decltype(g)>::value);
    }, cdg);
    bg::traits::visit<DynamicGeometry>::apply([](auto const g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(std::is_const<decltype(g)>::value);
    }, cdg);
    bg::traits::visit<DynamicGeometry>::apply([](auto & g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(std::is_const<std::remove_reference_t<decltype(g)>>::value);
    }, cdg);
    bg::traits::visit<DynamicGeometry>::apply([](auto const& g)
    {
        BOOST_CHECK(bg::util::is_point<decltype(g)>::value);
        static_assert(std::is_const<std::remove_reference_t<decltype(g)>>::value);
    }, cdg);
}

int test_main(int, char* [])
{
    using variant_t = boost::variant<point_t, linestring_t, polygon_t>;
    using variant2_t = boost::variant2::variant<point_t, linestring_t, polygon_t>;

    test_all<variant_t>();
    test_all<variant2_t>();

    return 0;
}
