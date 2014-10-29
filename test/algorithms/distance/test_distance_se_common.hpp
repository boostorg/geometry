// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_TEST_DISTANCE_SE_COMMON_HPP
#define BOOST_GEOMETRY_TEST_DISTANCE_SE_COMMON_HPP

#include <iostream>
#include <string>

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/multi/io/wkt/write.hpp>

#include <boost/geometry/io/dsv/write.hpp>
#include <boost/geometry/multi/io/dsv/write.hpp>

#include <boost/geometry/algorithms/num_interior_rings.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>

#include "../from_wkt.hpp"

#include <string_from_type.hpp>


#ifndef BOOST_GEOMETRY_TEST_DISTANCE_HPP

namespace bg = ::boost::geometry;

static const double earth_radius_km = 6371.0;
static const double earth_radius_miles = 3959.0;

// function copied from BG's test_distance.hpp

template <typename Geometry1, typename Geometry2>
void test_empty_input(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    try
    {
        bg::distance(geometry1, geometry2);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false, "A empty_input_exception should have been thrown" );
}
#endif // BOOST_GEOMETRY_TEST_DISTANCE_HPP



//========================================================================



template
<
    typename PointLike1,
    typename PointLike2,
    typename Strategy,
    typename Tag1 = typename bg::tag<PointLike1>::type,
    typename Tag2 = typename bg::tag<PointLike2>::type
>
struct distance_brute_force
{};

template 
<
    typename PointLike1,
    typename PointLike2,
    typename Strategy
>
struct distance_brute_force
<
    PointLike1, PointLike2, Strategy,
    bg::point_tag, bg::point_tag
>
{
    typedef typename bg::distance_result
        <
            PointLike1, PointLike2, Strategy
        >::type distance_type;

    static inline distance_type apply(PointLike1 const& p1,
                                      PointLike2 const& p2,
                                      Strategy const& strategy)
    {
        return bg::distance(p1, p2, strategy);
    }
};

template 
<
    typename PointLike1,
    typename PointLike2,
    typename Strategy
>
struct distance_brute_force
<
    PointLike1, PointLike2, Strategy,
    bg::point_tag, bg::multi_point_tag
>
{
    typedef typename bg::distance_result
        <
            PointLike1, PointLike2, Strategy
        >::type distance_type;

    static inline distance_type apply(PointLike1 const& p,
                                      PointLike2 const& mp,
                                      Strategy const& strategy)
    {
        typedef typename boost::range_iterator<PointLike2 const>::type iterator;

        bool first = true;
        distance_type d_min;
        for (iterator it = boost::begin(mp); it != boost::end(mp);
             ++it, first = false)
        {
            distance_type d = bg::distance(p, *it, strategy);

            if ( first || d < d_min )
            {
                d_min = d;
            }
        }
        return d_min;
    }
};

template 
<
    typename PointLike1,
    typename PointLike2,
    typename Strategy
>
struct distance_brute_force
<
    PointLike1, PointLike2, Strategy,
    bg::multi_point_tag, bg::multi_point_tag
>
{
    typedef typename bg::distance_result
        <
            PointLike1, PointLike2, Strategy
        >::type distance_type;

    static inline distance_type apply(PointLike1 const& mp1,
                                      PointLike2 const& mp2,
                                      Strategy const& strategy)
    {
        typedef typename boost::range_iterator
            <
                PointLike1 const
            >::type iterator1;

        typedef typename boost::range_iterator
            <
                PointLike2 const
            >::type iterator2;

        bool first = true;
        distance_type d_min;
        for (iterator1 it1 = boost::begin(mp1); it1 != boost::end(mp1); ++it1)
        {
            for (iterator2 it2 = boost::begin(mp2); it2 != boost::end(mp2);
                 ++it2, first = false)
            {
                distance_type d = bg::distance(*it1, *it2, strategy);

                if ( first || d < d_min )
                {
                    d_min = d;
                }
            }
        }
        return d_min;
    }
};



//========================================================================



#ifdef BOOST_GEOMETRY_TEST_DEBUG
// pretty print geometry -- START
template <typename Geometry, typename GeometryTag>
struct pretty_print_geometry_dispatch
{
    template <typename Stream>
    static inline Stream& apply(Geometry const& geometry, Stream& os)
    {
        os << bg::wkt(geometry);
        return os;
    }
};

template <typename Geometry>
struct pretty_print_geometry_dispatch<Geometry, bg::segment_tag>
{
    template <typename Stream>
    static inline Stream& apply(Geometry const& geometry, Stream& os)
    {
        os << "SEGMENT" << bg::dsv(geometry);
        return os;
    }
};

template <typename Geometry>
struct pretty_print_geometry_dispatch<Geometry, bg::box_tag>
{
    template <typename Stream>
    static inline Stream& apply(Geometry const& geometry, Stream& os)
    {
        os << "BOX" << bg::dsv(geometry);
        return os;
    }
};


template <typename Geometry>
struct pretty_print_geometry
{
    template <typename Stream>
    static inline Stream& apply(Geometry const& geometry, Stream& os)
    {
        return pretty_print_geometry_dispatch
            <
                Geometry, typename bg::tag<Geometry>::type
            >::apply(geometry, os);
    }
};
// pretty print geometry -- END
#endif // BOOST_GEOMETRY_TEST_DEBUG


//========================================================================


template <typename T>
struct check_equal
{
    static inline void apply(T const& value1, T const& value2)
    {
        BOOST_CHECK( value1 == value2 );
    }
};

template <>
struct check_equal<double>
{
    static inline void apply(double value1, double value2)
    {
        BOOST_CHECK_CLOSE( value1, value2, 0.0001 );
    }
};


//========================================================================

template
<
    typename Geometry1, typename Geometry2,
    int id1 = bg::geometry_id<Geometry1>::value,
    int id2 = bg::geometry_id<Geometry2>::value
>
struct test_distance_of_geometries
    : public test_distance_of_geometries<Geometry1, Geometry2, 0, 0>
{};


template <typename Geometry1, typename Geometry2>
struct test_distance_of_geometries<Geometry1, Geometry2, 0, 0>
{
    template
    <
        typename DistanceType,
        typename ComparableDistanceType,
        typename Strategy
    >
    static inline
    void apply(std::string const& wkt1,
               std::string const& wkt2,
               DistanceType const& expected_distance,
               ComparableDistanceType const& expected_comparable_distance,
               Strategy const& strategy,
               bool test_reversed = true)
    {
        Geometry1 geometry1 = from_wkt<Geometry1>(wkt1);
        Geometry2 geometry2 = from_wkt<Geometry2>(wkt2);

        apply(geometry1, geometry2,
              expected_distance, expected_comparable_distance,
              strategy, test_reversed);
    }


    template
    <
        typename DistanceType,
        typename ComparableDistanceType,
        typename Strategy
    >
    static inline
    void apply(Geometry1 const& geometry1,
               Geometry2 const& geometry2,
               DistanceType const& expected_distance,
               ComparableDistanceType const& expected_comparable_distance,
               Strategy const& strategy,
               bool test_reversed = true)
    {
#ifdef BOOST_GEOMETRY_TEST_DEBUG
        typedef pretty_print_geometry<Geometry1> PPG1;
        typedef pretty_print_geometry<Geometry2> PPG2;
        PPG1::apply(geometry1, std::cout);
        std::cout << " - ";
        PPG2::apply(geometry2, std::cout);
        std::cout << std::endl;
#endif
        typedef typename bg::default_distance_result
            <
                Geometry1, Geometry2
            >::type default_distance_result;

        typedef typename bg::strategy::distance::services::return_type
            <
                Strategy, Geometry1, Geometry2
            >::type distance_result_from_strategy;

        static const bool same_regular = boost::is_same
            <
                default_distance_result,
                distance_result_from_strategy
            >::type::value;

        BOOST_CHECK( same_regular );
    

        typedef typename bg::default_comparable_distance_result
            <
                Geometry1, Geometry2
            >::type default_comparable_distance_result;

        typedef typename bg::strategy::distance::services::return_type
            <
                typename bg::strategy::distance::services::comparable_type
                    <
                        Strategy
                    >::type,
                Geometry1,
                Geometry2
            >::type comparable_distance_result_from_strategy;

        static const bool same_comparable = boost::is_same
            <
                default_comparable_distance_result,
                comparable_distance_result_from_strategy
            >::type::value;
        
        BOOST_CHECK( same_comparable );


        // check distance with default strategy
        default_distance_result dist_def = bg::distance(geometry1, geometry2);

        check_equal
            <
                default_distance_result
            >::apply(dist_def, expected_distance);


        // check distance with passed strategy
        distance_result_from_strategy dist =
            bg::distance(geometry1, geometry2, strategy);

        check_equal
            <
                default_distance_result
            >::apply(dist, expected_distance * strategy.radius());

        // check against the comparable distance computed in a
        // brute-force manner
        default_distance_result dist_brute_force = distance_brute_force
            <
                Geometry1, Geometry2, Strategy
            >::apply(geometry1, geometry2, strategy);

        check_equal
            <
                default_distance_result
            >::apply(dist_brute_force, expected_distance * strategy.radius());


        // check comparable distance with default strategy
        default_comparable_distance_result cdist_def =
            bg::comparable_distance(geometry1, geometry2);

        check_equal
            <
                default_comparable_distance_result
            >::apply(cdist_def, expected_comparable_distance);


        // check comparable distance with passed strategy
        comparable_distance_result_from_strategy cdist =
            bg::comparable_distance(geometry1, geometry2, strategy);

        check_equal
            <
                default_comparable_distance_result
            >::apply(cdist, expected_comparable_distance);

        // check against the comparable distance computed in a
        // brute-force manner
        default_comparable_distance_result cdist_brute_force
            = distance_brute_force
            <
                Geometry1,
                Geometry2,
                typename bg::strategy::distance::services::comparable_type
                    <
                        Strategy
                    >::type
            >::apply(geometry1,
                     geometry2,
                     bg::strategy::distance::services::get_comparable
                         <
                             Strategy
                         >::apply(strategy));

        check_equal
            <
                default_comparable_distance_result
            >::apply(cdist_brute_force, expected_comparable_distance);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << string_from_type<typename bg::coordinate_type<Geometry1>::type>::name()
                  << string_from_type<typename bg::coordinate_type<Geometry2>::type>::name()
                  << " -> "
                  << string_from_type<default_distance_result>::name()
                  << string_from_type<default_comparable_distance_result>::name()
                  << std::endl;
        std::cout << "expected distance (default strategy) = "
                  << expected_distance << " ; "
                  << "expected distance (passed strategy) = "
                  << (expected_distance * strategy.radius()) << " ; "
                  << "expected comp. distance = "
                  << expected_comparable_distance
                  << std::endl;
        std::cout << "distance (default strategy) = " << dist_def << " ; " 
                  << "distance (passed strategy) = " << dist << " ; " 
                  << "comp. distance (default strategy) = "
                  << cdist_def << " ; "
                  << "comp. distance (passed strategy) = "
                  << cdist << std::endl;

        if ( !test_reversed )
        {
            std::cout << std::endl;
        }
#endif

        if ( test_reversed )
        {
            // check distance with default strategy
            dist_def = bg::distance(geometry2, geometry1);

            check_equal
                <
                    default_distance_result
                >::apply(dist_def, expected_distance);


            // check distance with given strategy
            dist = bg::distance(geometry2, geometry1, strategy);

            check_equal
                <
                    default_distance_result
                >::apply(dist, expected_distance * strategy.radius());


            // check comparable distance with default strategy
            cdist_def = bg::comparable_distance(geometry2, geometry1);

            check_equal
                <
                    default_comparable_distance_result
                >::apply(cdist_def, expected_comparable_distance);

            // check comparable distance with given strategy
            cdist = bg::comparable_distance(geometry2, geometry1, strategy);

            check_equal
                <
                    default_comparable_distance_result
                >::apply(cdist, expected_comparable_distance);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
            std::cout << "expected distance (default strategy) = "
                      << expected_distance << " ; "
                      << "expected distance (passed strategy) = "
                      << (expected_distance * strategy.radius()) << " ; "
                      << "expected comp. distance = "
                      << expected_comparable_distance
                      << std::endl;
            std::cout << "distance[reversed args] (def. startegy) = "
                      << dist_def << " ; "
                      << "distance[reversed args] (passed startegy) = "
                      << dist << " ; "
                      << "comp. distance[reversed args] (def. strategy) = "
                      << cdist_def << " ; "
                      << "comp. distance[reversed args] (passed strategy) = "
                      << cdist << std::endl;
            std::cout << std::endl;
#endif
        }
    }
};


//========================================================================


template <typename Geometry1, typename Geometry2, typename Strategy>
void test_empty_input(Geometry1 const& geometry1,
                      Geometry2 const& geometry2,
                      Strategy const& strategy)
{
    try
    {
        bg::distance(geometry1, geometry2, strategy);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false, "A empty_input_exception should have been thrown" );

    try
    {
        bg::distance(geometry2, geometry1, strategy);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false, "A empty_input_exception should have been thrown" );
}

#endif // BOOST_GEOMETRY_TEST_DISTANCE_SE_COMMON_HPP
