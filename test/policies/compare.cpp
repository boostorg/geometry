// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <ggl_test_common.hpp>

#include <algorithm>

#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/util/write_dsv.hpp>

#include <boost/geometry/policies/compare.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <test_common/test_point.hpp>


template <typename Container>
inline std::string coordinates(Container const& points)
{
    std::ostringstream out;
    typedef typename boost::range_value<Container>::type point_type;
    for (typename boost::range_const_iterator<Container>::type it = boost::begin(points);
        it != boost::end(points);
        ++it)
    {
        out << boost::geometry::dsv(*it);
    }
    return out.str();
}

template <typename P>
void test_2d_compare()
{
    P p1 = boost::geometry::make<P>(3, 1);
    P p2 = boost::geometry::make<P>(3, 1);
    P p3 = boost::geometry::make<P>(1, 3);
    P p4 = boost::geometry::make<P>(5, 2);
    P p5 = boost::geometry::make<P>(3, 2);

    // Test in all dimensions
    {
        boost::geometry::equal_to<P> et;
        boost::geometry::less<P> lt;
        boost::geometry::greater<P> gt;

        BOOST_CHECK_EQUAL(et(p1, p2), true);
        BOOST_CHECK_EQUAL(et(p1, p3), false);
        BOOST_CHECK_EQUAL(et(p1, p4), false);
        BOOST_CHECK_EQUAL(et(p1, p5), false);
        BOOST_CHECK_EQUAL(et(p3, p4), false);

        BOOST_CHECK_EQUAL(lt(p1, p2), false);
        BOOST_CHECK_EQUAL(lt(p1, p3), false);
        BOOST_CHECK_EQUAL(lt(p1, p4), true);
        BOOST_CHECK_EQUAL(lt(p1, p5), true);
        BOOST_CHECK_EQUAL(lt(p3, p4), true);

        BOOST_CHECK_EQUAL(gt(p1, p2), false);
        BOOST_CHECK_EQUAL(gt(p1, p3), true);
        BOOST_CHECK_EQUAL(gt(p1, p4), false);
        BOOST_CHECK_EQUAL(gt(p1, p5), false);
        BOOST_CHECK_EQUAL(gt(p3, p4), false);
    }


    // Test in dimension 0, X
    {
        boost::geometry::equal_to<P, 0> et;
        boost::geometry::less<P, 0> lt;
        boost::geometry::greater<P, 0> gt;

        BOOST_CHECK_EQUAL(et(p1, p2), true);
        BOOST_CHECK_EQUAL(et(p1, p3), false);
        BOOST_CHECK_EQUAL(et(p1, p4), false);
        BOOST_CHECK_EQUAL(et(p1, p5), true);
        BOOST_CHECK_EQUAL(et(p3, p4), false);

        BOOST_CHECK_EQUAL(lt(p1, p2), false);
        BOOST_CHECK_EQUAL(lt(p1, p3), false);
        BOOST_CHECK_EQUAL(lt(p1, p4), true);
        BOOST_CHECK_EQUAL(lt(p1, p5), false);
        BOOST_CHECK_EQUAL(lt(p3, p4), true);

        BOOST_CHECK_EQUAL(gt(p1, p2), false);
        BOOST_CHECK_EQUAL(gt(p1, p3), true);
        BOOST_CHECK_EQUAL(gt(p1, p4), false);
        BOOST_CHECK_EQUAL(gt(p1, p5), false);
        BOOST_CHECK_EQUAL(gt(p3, p4), false);
    }

    // Test in dimension 1, Y
    {
        boost::geometry::equal_to<P, 1> et;
        boost::geometry::less<P, 1> lt;
        boost::geometry::greater<P, 1> gt;

        BOOST_CHECK_EQUAL(et(p1, p2), true);
        BOOST_CHECK_EQUAL(et(p1, p3), false);
        BOOST_CHECK_EQUAL(et(p1, p4), false);
        BOOST_CHECK_EQUAL(et(p1, p5), false);
        BOOST_CHECK_EQUAL(et(p3, p4), false);

        BOOST_CHECK_EQUAL(lt(p1, p2), false);
        BOOST_CHECK_EQUAL(lt(p1, p3), true);
        BOOST_CHECK_EQUAL(lt(p1, p4), true);
        BOOST_CHECK_EQUAL(lt(p1, p5), true);
        BOOST_CHECK_EQUAL(lt(p3, p4), false);

        BOOST_CHECK_EQUAL(gt(p1, p2), false);
        BOOST_CHECK_EQUAL(gt(p1, p3), false);
        BOOST_CHECK_EQUAL(gt(p1, p4), false);
        BOOST_CHECK_EQUAL(gt(p1, p5), false);
        BOOST_CHECK_EQUAL(gt(p3, p4), true);
    }
}


template <typename P>
void test_2d_sort()
{
    typedef typename boost::geometry::coordinate_type<P>::type ct;

    std::vector<P> v;
    v.push_back(boost::geometry::make<P>(3, 1));
    v.push_back(boost::geometry::make<P>(2, 3));
    v.push_back(boost::geometry::make<P>(2, 2));
    v.push_back(boost::geometry::make<P>(1, 3));

    // Sort on coordinates in order x,y,z
    std::sort(v.begin(), v.end(), boost::geometry::less<P>());
    std::string s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(1, 3)(2, 2)(2, 3)(3, 1)");

    // Reverse sort
    std::sort(v.begin(), v.end(), boost::geometry::greater<P>());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(3, 1)(2, 3)(2, 2)(1, 3)");

    // Sort backwards on coordinates in order x,y,z
    //std::sort(v.begin(), v.end(), boost::geometry::greater<P>());
    //std::string s = coordinates(v);
    //BOOST_CHECK_EQUAL(s, "(1, 3)(2, 2)(2, 3)(3, 1)");

    // Refill to remove duplicate coordinates
    v.clear();
    v.push_back(boost::geometry::make<P>(4, 1));
    v.push_back(boost::geometry::make<P>(3, 2));
    v.push_back(boost::geometry::make<P>(2, 3));
    v.push_back(boost::geometry::make<P>(1, 4));

    // Sort ascending on only x-coordinate
    std::sort(v.begin(), v.end(), boost::geometry::less<P, 0>());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(1, 4)(2, 3)(3, 2)(4, 1)");

    // Sort ascending on only y-coordinate
    std::sort(v.begin(), v.end(), boost::geometry::less<P, 1>());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(4, 1)(3, 2)(2, 3)(1, 4)");

    // Sort descending on only x-coordinate
    std::sort(v.begin(), v.end(), boost::geometry::greater<P, 0>());
    s = coordinates(v);
    //BOOST_CHECK_EQUAL(s, "(4, 1)(3, 2)(2, 3)(1, 4)");

    // Sort descending on only y-coordinate
    std::sort(v.begin(), v.end(), boost::geometry::greater<P, 1>());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(1, 4)(2, 3)(3, 2)(4, 1)");

    // Make non-unique vector
    v.push_back(boost::geometry::make<P>(4, 1));
    v.push_back(boost::geometry::make<P>(3, 2));
    v.push_back(boost::geometry::make<P>(2, 3));
    v.push_back(boost::geometry::make<P>(1, 4));
    v.push_back(boost::geometry::make<P>(1, 5));
    std::sort(v.begin(), v.end(), boost::geometry::less<P>());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(1, 4)(1, 4)(1, 5)(2, 3)(2, 3)(3, 2)(3, 2)(4, 1)(4, 1)");


    std::vector<P> v2;
    std::unique_copy(v.begin(), v.end(), std::back_inserter(v2), boost::geometry::equal_to<P>());
    s = coordinates(v2);
    BOOST_CHECK_EQUAL(s, "(1, 4)(1, 5)(2, 3)(3, 2)(4, 1)");
}


template <typename P>
void test_spherical()
{
    typedef typename boost::geometry::coordinate_type<P>::type ct;

    std::vector<P> v;
    v.push_back(boost::geometry::make<P>( 179.73, 71.56)); // east
    v.push_back(boost::geometry::make<P>( 177.47, 71.23)); // less east
    v.push_back(boost::geometry::make<P>(-178.78, 70.78)); // further east, = west, this is the most right point

    // Sort on coordinates in order x,y,z
    std::sort(v.begin(), v.end(), boost::geometry::less<P>());
    std::string s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(177.47, 71.23)(179.73, 71.56)(-178.78, 70.78)");

    // Sort ascending on only x-coordinate
    std::sort(v.begin(), v.end(), boost::geometry::less<P, 0>());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(177.47, 71.23)(179.73, 71.56)(-178.78, 70.78)");

    // Sort ascending on only x-coordinate, but override with std-comparison,
    // (so this is the normal sorting behaviour that would have been used
    // if it would not have been spherical)
    std::sort(v.begin(), v.end(), boost::geometry::less<P, 0, std::less<ct> >());
    s = coordinates(v);
    BOOST_CHECK_EQUAL(s, "(-178.78, 70.78)(177.47, 71.23)(179.73, 71.56)");
}


int test_main(int, char* [])
{
    test_2d_compare<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_2d_compare<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    test_2d_sort<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_2d_sort<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_2d_sort<boost::tuple<double, double> >();
    test_2d_sort<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    test_spherical<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::degree> > >();

    return 0;
}
