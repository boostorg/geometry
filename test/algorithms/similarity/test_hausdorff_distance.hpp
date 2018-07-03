// Boost.Geometry

// Copyright (c) 2018 Yaghyavardhan Singh Khangarot, Hyderabad, India.

// Contributed and/or modified by Yaghyavardhan Singh Khangarot, as part of Google Summer of Code 2018 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#ifndef BOOST_GEOMETRY_TEST_HAUSDORFF_DISTANCE_HPP
#define BOOST_GEOMETRY_TEST_HAUSDORFF_DISTANCE_HPP


//#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "../../../include/boost/geometry/algorithms/hausdorff_distance.hpp"
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/variant/variant.hpp>
#include <boost/mpl/if.hpp>


namespace bg = boost::geometry;


template <typename Geometry1,typename Geometry2>
void test_hausdorff_distance(Geometry1 const& geometry1,Geometry2 const& geometry2,
	typename bg::distance_result
        <
            typename bg::point_type<Geometry1>::type,
            typename bg::point_type<Geometry2>::type
        >::type expected_hausdorff_distance )
{
	using namespace bg;
    typedef typename distance_result
        <
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type
        >::type result_type;
    long double h_distance = bg::hausdorff_distance(geometry1,geometry2);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::ostringstream out;
    out << typeid(typename bg::coordinate_type<Geometry1>::type).name()
        << std::endl
        << typeid(typename bg::coordinate_type<Geometry2>::type).name()
        << std::endl
        << typeid(h_distance).name()
        << std::endl
        << "hausdorff_distance : " << bg::hausdorff_distance(geometry1,geometry2)
        << std::endl;
    std::cout << out.str();
#endif

   // BOOST_CHECK_CLOSE(h_distance, expected_hausdorff_distance, 0.0001);
}



template <typename Geometry1,typename Geometry2>
void test_geometry(std::string const& wkt1,std::string const& wkt2,
	typename bg::distance_result
        <
            typename bg::point_type<Geometry1>::type,
            typename bg::point_type<Geometry2>::type
        >::type expected_hausdorff_distance)
{
    Geometry1 geometry1;
    bg::read_wkt(wkt1, geometry1);
    Geometry2 geometry2;
    bg::read_wkt(wkt2, geometry2);
    test_hausdorff_distance(geometry1,geometry2,expected_hausdorff_distance);
#if !defined(BOOST_GEOMETRY_TEST_DEBUG)
    test_hausdorff_distance(boost::variant<Geometry1>(geometry1),boost::variant<Geometry2>(geometry2), expected_hausdorff_distance);
#endif
}

template <typename Geometry1,typename Geometry2 ,typename Strategy>
void test_hausdorff_distance(Geometry1 const& geometry1,Geometry2 const& geometry2,Strategy strategy,
	typename bg::distance_result
        <
            typename bg::point_type<Geometry1>::type,
            typename bg::point_type<Geometry2>::type,
            Strategy
        >::type expected_hausdorff_distance )
{
	using namespace bg;
    typedef typename distance_result
        <
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type,
            Strategy
        >::type result_type;
    long double h_distance = bg::hausdorff_distance(geometry1,geometry2,strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::ostringstream out;
    out << typeid(typename bg::coordinate_type<Geometry1>::type).name()
        << std::endl
        << typeid(typename bg::coordinate_type<Geometry2>::type).name()
        << std::endl
        << typeid(h_distance).name()
        << std::endl
        << "hausdorff_distance : " << bg::hausdorff_distance(geometry1,geometry2,strategy)
        << std::endl;
    std::cout << out.str();
#endif

    //BOOST_CHECK_CLOSE(h_distance, expected_hausdorff_distance, 0.0001);
}



template <typename Geometry1,typename Geometry2,typename Strategy>
void test_geometry(std::string const& wkt1,std::string const& wkt2,Strategy strategy,
	typename bg::distance_result
        <
            typename bg::point_type<Geometry1>::type,
            typename bg::point_type<Geometry2>::type,
            Strategy
        >::type expected_hausdorff_distance)
{
    Geometry1 geometry1;
    bg::read_wkt(wkt1, geometry1);
    Geometry2 geometry2;
    bg::read_wkt(wkt2, geometry2);
    test_hausdorff_distance(geometry1,geometry2,strategy,expected_hausdorff_distance);
#if !defined(BOOST_GEOMETRY_TEST_DEBUG)
    test_hausdorff_distance(boost::variant<Geometry1>(geometry1),boost::variant<Geometry2>(geometry2),strategy, expected_hausdorff_distance);
#endif
}


template <typename Geometry1,typename Geometry2>
void test_empty_input(Geometry1 const& geometry1,Geometry2 const& geometry2)
{
    try
    {
        bg::hausdorff_distance(geometry1,geometry2);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    //BOOST_CHECK_MESSAGE(false, "A empty_input_exception should have been thrown" );
}


#endif
