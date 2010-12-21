// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <deque>
#include <vector>

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/geometries/concepts/linestring_concept.hpp>


#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>

//#define TEST_FAIL_CLEAR
//#define TEST_FAIL_APPEND



// ----------------------------------------------------------------------------
// First custom linestring, requires ONLY one traits: to register itself as a linestring
template <typename P>
struct custom_linestring1 : std::vector<P> {};

namespace boost { namespace geometry { namespace traits {
    template <typename P>
    struct tag< custom_linestring1<P> > { typedef linestring_tag type; };
}}} // namespace bg::traits

// ----------------------------------------------------------------------------
// Second custom linestring, decides to implement all edit operations itself
// by specializing the "use_std" traits to false.
// It should therefore implement the traits:: clear / append_point
template <typename P>
struct custom_linestring2 : std::deque<P> // std::pair<typename std::vector<P>::const_iterator, typename std::vector<P>::const_iterator> 
{
};

namespace boost { namespace geometry { namespace traits {
    template <typename P>
    struct tag< custom_linestring2<P> > { typedef linestring_tag type; };

#if ! defined(TEST_FAIL_CLEAR)
    template <typename P>
    struct clear< custom_linestring2<P> >
    {
        // does not use std::vector<P>.clear() but something else.
        static inline void apply(custom_linestring2<P>& ls) { ls.resize(0); }
    };
#endif

}}} // namespace bg::traits

// ----------------------------------------------------------------------------

template <typename G>
void test_linestring()
{
    BOOST_CONCEPT_ASSERT( (bg::concept::Linestring<G>) );
    BOOST_CONCEPT_ASSERT( (bg::concept::ConstLinestring<G>) );

    G geometry;
    typedef typename bg::point_type<G>::type P;

    bg::clear(geometry);
    BOOST_CHECK_EQUAL(boost::size(geometry), 0);

    bg::append(geometry, bg::make_zero<P>());
    BOOST_CHECK_EQUAL(boost::size(geometry), 1);

    //std::cout << geometry << std::endl;

    bg::clear(geometry);
    BOOST_CHECK_EQUAL(boost::size(geometry), 0);


    //P p = boost::range::front(geometry);
}

template <typename P>
void test_all()
{
    test_linestring<bg::model::linestring<P> >();
    test_linestring<bg::model::linestring<P, std::vector> >();
    test_linestring<bg::model::linestring<P, std::deque> >();

    test_linestring<custom_linestring1<P> >();
    test_linestring<custom_linestring2<P> >();

    test_linestring<std::vector<P> >();
    test_linestring<std::deque<P> >();
    //test_linestring<std::list<P> >();
}

int test_main(int, char* [])
{
    test_all<test::test_point>();
    test_all<boost::tuple<float, float> >();
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();

    return 0;
}
