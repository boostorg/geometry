// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_TEST_DISTANCE_COMMON_HPP
#define BOOST_GEOMETRY_TEST_DISTANCE_COMMON_HPP

#include <iostream>
#include <string>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/multi/io/wkt/write.hpp>

#include <boost/geometry/io/dsv/write.hpp>
#include <boost/geometry/multi/io/dsv/write.hpp>

#include "from_wkt.hpp"



#ifndef BOOST_GEOMETRY_TEST_DISTANCE_HPP

namespace bg = ::boost::geometry;

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
//========================================================================
//========================================================================


// create geometries -- START
template<typename Segment>
Segment make_segment(double x1, double y1, double x2, double y2)
{
    typename bg::point_type<Segment>::type p(x1, y1), q(x2, y2);
    return Segment(p, q);
}


template <typename Box>
Box make_box2d(double xmin, double ymin, double xmax, double ymax)
{
    typedef typename bg::point_type<Box>::type BoxPoint;

    return Box(BoxPoint(xmin, ymin), BoxPoint(xmax, ymax));
}

template <typename Box>
Box make_box3d(double xmin, double ymin, double zmin,
               double xmax, double ymax, double zmax)
{
    typedef typename bg::point_type<Box>::type BoxPoint;

    return Box(BoxPoint(xmin, ymin, zmin), BoxPoint(xmax, ymax, zmax));
}
// create geometries -- END


//========================================================================
//========================================================================
//========================================================================


#ifdef GEOMETRY_TEST_DEBUG
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
#endif // GEOMETRY_TEST_DEBUG




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
    template <typename Strategy>
    void operator()(std::string const& wkt1,
                    std::string const& wkt2,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy,
                    bool test_reversed = false) const
    {
        Geometry1 geometry1 = from_wkt<Geometry1>(wkt1);
        Geometry2 geometry2 = from_wkt<Geometry2>(wkt2);

        operator()(geometry1, geometry2,
                   expected_distance, expected_comparable_distance,
                   strategy, test_reversed);
    }

    template <typename Strategy>
    void operator()(Geometry1 const& geometry1,
                    std::string const& wkt2,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy,
                    bool test_reversed = false) const
    {
        Geometry2 geometry2 = from_wkt<Geometry2>(wkt2);

        operator()(geometry1, geometry2,
                   expected_distance, expected_comparable_distance,
                   strategy, test_reversed);
    }

    template <typename Strategy>
    void operator()(std::string const& wkt1,
                    Geometry2 const& geometry2,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy,
                    bool test_reversed = false) const
    {
        Geometry1 geometry1 = from_wkt<Geometry1>(wkt1);

        operator()(geometry1, geometry2,
                   expected_distance, expected_comparable_distance,
                   strategy, test_reversed);
    }

    template <typename Strategy>
    void operator()(Geometry1 const& geometry1,
                    Geometry2 const& geometry2,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy,
                    bool test_reversed = true) const
    {

#ifdef GEOMETRY_TEST_DEBUG
        typedef pretty_print_geometry<Geometry1> PPG1;
        typedef pretty_print_geometry<Geometry2> PPG2;
        PPG1::apply(geometry1, std::cout);
        std::cout << " - ";
        PPG2::apply(geometry2, std::cout);
        std::cout << std::endl;
#endif

        typename bg::default_distance_result<Geometry1>::type dist_def
            = bg::distance(geometry1, geometry2);

        BOOST_CHECK_CLOSE(dist_def, expected_distance, 0.0001);

        typename bg::default_distance_result<Geometry1>::type dist
            = bg::distance(geometry1, geometry2, strategy);

        BOOST_CHECK_CLOSE(dist, expected_distance, 0.0001);

#ifdef GEOMETRY_TEST_DEBUG
        typename bg::default_distance_result<Geometry1>::type cdist_def
            = bg::comparable_distance(geometry1, geometry2);

        BOOST_CHECK_CLOSE(cdist_def, expected_comparable_distance, 0.001);
#endif

        typename bg::default_distance_result<Geometry1>::type cdist
            = bg::comparable_distance(geometry1, geometry2, strategy);

        BOOST_CHECK_CLOSE(cdist, expected_comparable_distance, 0.001);

#ifdef GEOMETRY_TEST_DEBUG
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
            dist_def = bg::distance(geometry2, geometry1);

            BOOST_CHECK_CLOSE(dist_def, expected_distance, 0.0001);

            dist = bg::distance(geometry2, geometry1, strategy);

            BOOST_CHECK_CLOSE(dist, expected_distance, 0.0001);

#ifdef GEOMETRY_TEST_DEBUG
            cdist_def = bg::comparable_distance(geometry2, geometry1);

            BOOST_CHECK_CLOSE(cdist_def, expected_comparable_distance, 0.001);
#endif

            cdist = bg::comparable_distance(geometry2, geometry1, strategy);

            BOOST_CHECK_CLOSE(cdist, expected_comparable_distance, 0.001);

#ifdef GEOMETRY_TEST_DEBUG
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
//========================================================================

template <typename Geometry1, typename Geometry2>
struct test_distance_of_geometries
<
    Geometry1, Geometry2,
    92 /* segment */, 3 /* polygon */
>
    : public test_distance_of_geometries<Geometry1, Geometry2, 0, 0>
{
    typedef test_distance_of_geometries<Geometry1, Geometry2, 0, 0> base;

    typedef typename bg::ring_type<Geometry2>::type ring_type;

    template <typename Segment, typename Strategy>
    void operator()(Segment const& segment,
                    std::string const& wkt,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
        Geometry2 geometry = from_wkt<Geometry2>(wkt);
        operator()(segment,
                   geometry,
                   expected_distance,
                   expected_comparable_distance,
                   strategy);
    }


    template <typename Segment, typename Strategy>
    void operator()(Segment const& segment,
                    Geometry2 const& geometry,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
        base::operator()(segment, geometry, expected_distance,
                         expected_comparable_distance, strategy);
        if ( bg::num_interior_rings(geometry) == 0 ) {
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "... testing also exterior ring ..." << std::endl;
#endif
#if 0
            // cannot run it now; needs within(point,linestring)
            test_distance_of_segment_and_geometry<ring_type>()
                (segment,
                 bg::exterior_ring(geometry),
                 expected_distance,
                 expected_comparable_distance,
                 strategy);
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

#endif // BOOST_GEOMETRY_TEST_DISTANCE_COMMON_HPP
