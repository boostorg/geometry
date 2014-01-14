#ifndef BOOST_GEOMETRY_TEST_DISTANCE1_HPP
#define BOOST_GEOMETRY_TEST_DISTANCE1_HPP

#ifndef BOOST_GEOMETRY_TEST_DISTANCE

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
#endif // BOOST_GEOMETRY_TEST_DISTANCE

#include "from_wkt.hpp"

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/write.hpp>


struct test_distance_of_segments
{
    template <typename Segment1, typename Segment2, typename Strategy>
    void operator()(Segment1 const& segment1,
                    Segment2 const& segment2,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << bg::wkt(segment1) << " - " << bg::wkt(segment2)
                  << std::endl;
#endif

        typename bg::default_distance_result<Segment1>::type distance_def
            = bg::distance(segment1, segment2);

        BOOST_CHECK_CLOSE(distance_def, expected_distance, 0.0001);

        typename bg::default_distance_result<Segment1>::type distance_ss
            = bg::distance(segment1, segment2, strategy);

        BOOST_CHECK_CLOSE(distance_ss, expected_distance, 0.0001);

        typename bg::default_distance_result<Segment1>::type comparable_distance_def
            = bg::comparable_distance(segment1, segment2);

        BOOST_CHECK_CLOSE(comparable_distance_def,
                          expected_comparable_distance, 0.001);

        typename bg::default_distance_result<Segment1>::type comparable_distance
            = bg::comparable_distance(segment1, segment2, strategy);

        BOOST_CHECK_CLOSE(comparable_distance,
                          expected_comparable_distance, 0.001);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "distance (default strategy) = " << distance_def << " ; "
                  << "distance (PS strategy) = " << distance_ss << " ; "
                  << "comp. distance (default strategy) = "
                  << comparable_distance_def << " ; "
                  << "comp. distance (PS strategy) = " << comparable_distance
                  << std::endl;
        std::cout << std::endl;
#endif
    }
};


template<typename Geometry, int = bg::geometry_id<Geometry>::value>
struct test_distance_of_segment_and_geometry
    : public test_distance_of_segment_and_geometry<Geometry,0>
{};


template<typename Geometry>
struct test_distance_of_segment_and_geometry<Geometry,0>
{
    template <typename Segment, typename Strategy>
    void operator()(Segment const& segment,
                    std::string const& wkt,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
        Geometry geometry = from_wkt<Geometry>(wkt);
        operator()(segment,
                   geometry,
                   expected_distance,
                   expected_comparable_distance,
                   strategy);
    }


    template <typename Segment, typename Strategy>
    void operator()(Segment const& segment,
                    Geometry const& geometry,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << bg::wkt(segment) << " - " << bg::wkt(geometry) << std::endl;
#endif

        typename bg::default_distance_result<Geometry>::type distance_def
            = bg::distance(segment, geometry);

        BOOST_CHECK_CLOSE(distance_def, expected_distance, 0.0001);

        typename bg::default_distance_result<Geometry>::type distance_ss
            = bg::distance(segment, geometry, strategy);

        BOOST_CHECK_CLOSE(distance_ss, expected_distance, 0.0001);

#ifdef GEOMETRY_TEST_DEBUG
        typename bg::default_distance_result<Geometry>::type comparable_distance_def
            = bg::comparable_distance(segment, geometry);

        BOOST_CHECK_CLOSE(comparable_distance_def,
                          expected_comparable_distance, 0.001);
#endif

        typename bg::default_distance_result<Geometry>::type comparable_distance
            = bg::comparable_distance(segment, geometry, strategy);

        BOOST_CHECK_CLOSE(comparable_distance,
                          expected_comparable_distance, 0.001);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "distance (default strategy) = " << distance_def << " ; "
                  << "distance (PS strategy) = " << distance_ss << " ; "
                  << "comp. distance (default strategy) = "
                  << comparable_distance_def << " ; "
                  << "comp. distance (PS strategy) = " << comparable_distance
                  << std::endl;
#endif

        distance_def = bg::distance(geometry,segment,strategy);

        BOOST_CHECK_CLOSE(distance_def, expected_distance, 0.0001);

        distance_ss = bg::distance(geometry,segment,strategy);

        BOOST_CHECK_CLOSE(distance_ss, expected_distance, 0.0001);

#ifdef GEOMETRY_TEST_DEBUG
        comparable_distance_def = bg::comparable_distance(geometry, segment);

        BOOST_CHECK_CLOSE(comparable_distance_def,
                          expected_comparable_distance, 0.001);
#endif

        comparable_distance =
            bg::comparable_distance(geometry, segment, strategy);

        BOOST_CHECK_CLOSE(comparable_distance,
                          expected_comparable_distance, 0.001);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "distance[reversed args] (def. startegy) = "
                  << distance_def << " ; "
                  << "distance[reversed args] (PS startegy) = "
                  << distance_ss << " ; "
                  << "comp. distance[reversed args] (def. strategy) = "
                  << comparable_distance << " ; "
                  << "comp. distance[reversed args] (PS strategy) = "
                  << comparable_distance << std::endl;
        std::cout << std::endl;
#endif
    }
};

template<typename Geometry>
struct test_distance_of_segment_and_geometry<Geometry,3>
    : public test_distance_of_segment_and_geometry<Geometry,0>
{
    typedef test_distance_of_segment_and_geometry<Geometry,0> base;

    typedef typename bg::ring_type<Geometry>::type ring_type;

    template <typename Segment, typename Strategy>
    void operator()(Segment const& segment,
                    std::string const& wkt,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
        Geometry geometry = from_wkt<Geometry>(wkt);
        operator()(segment,
                   geometry,
                   expected_distance,
                   expected_comparable_distance,
                   strategy);
    }


    template <typename Segment, typename Strategy>
    void operator()(Segment const& segment,
                    Geometry const& geometry,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy) const
    {
        base::operator()(segment, geometry, expected_distance,
                         expected_comparable_distance, strategy);
        if ( bg::num_interior_rings(geometry) == 0 ) {
            std::cout << "... testing also exterior ring ..." << std::endl;
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








template
<
    typename Geometry1, typename Geometry2,
    int id1 = bg::geometry_id<Geometry1>::value,
    int id2 = bg::geometry_id<Geometry2>::value
>
struct test_distance_of_geometries
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
                    Geometry2 const& geometry2,
                    double expected_distance,
                    double expected_comparable_distance,
                    Strategy const& strategy,
                    bool test_reversed = false) const
    {

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << bg::wkt(geometry1) << " - "
                  << bg::wkt(geometry2) << std::endl;
#endif

        typename bg::default_distance_result<Geometry1>::type distance_def
            = bg::distance(geometry1, geometry2);

        BOOST_CHECK_CLOSE(distance_def, expected_distance, 0.0001);

        typename bg::default_distance_result<Geometry1>::type distance_ss
            = bg::distance(geometry1, geometry2, strategy);

        BOOST_CHECK_CLOSE(distance_ss, expected_distance, 0.0001);

#ifdef GEOMETRY_TEST_DEBUG
        typename bg::default_distance_result<Geometry1>::type comparable_distance_def
            = bg::comparable_distance(geometry1, geometry2);

        BOOST_CHECK_CLOSE(comparable_distance_def,
                          expected_comparable_distance, 0.001);
#endif

        typename bg::default_distance_result<Geometry1>::type comparable_distance
            = bg::comparable_distance(geometry1, geometry2, strategy);

        BOOST_CHECK_CLOSE(comparable_distance,
                          expected_comparable_distance, 0.001);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "distance (default strategy) = " << distance_def << " ; " 
                  << "distance (PS strategy) = " << distance_ss << " ; " 
                  << "comp. distance (default strategy) = "
                  << comparable_distance_def << " ; "
                  << "comp. distance (PS strategy) = "
                  << comparable_distance << std::endl;

        if ( !test_reversed )
        {
            std::cout << std::endl;
        }
#endif

        if ( test_reversed )
        {
            distance_def = bg::distance(geometry2, geometry1);

            BOOST_CHECK_CLOSE(distance_def, expected_distance, 0.0001);

            distance_ss = bg::distance(geometry2, geometry1, strategy);

            BOOST_CHECK_CLOSE(distance_ss, expected_distance, 0.0001);

#ifdef GEOMETRY_TEST_DEBUG
            comparable_distance_def =
                bg::comparable_distance(geometry2, geometry1);

            BOOST_CHECK_CLOSE(comparable_distance_def,
                              expected_comparable_distance, 0.001);
#endif

            comparable_distance =
                bg::comparable_distance(geometry2, geometry1, strategy);

            BOOST_CHECK_CLOSE(comparable_distance, expected_comparable_distance,
                              0.001);

#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "distance[reversed args] (def. startegy) = "
                      << distance_def << " ; "
                      << "distance[reversed args] (PS startegy) = "
                      << distance_ss << " ; "
                      << "comp. distance[reversed args] (def. strategy) = "
                      << comparable_distance_def << " ; "
                      << "comp. distance[reversed args] (PS strategy) = "
                      << comparable_distance << std::endl;
            std::cout << std::endl;
#endif
        }
    }
};




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
}



#endif // BOOST_GEOMETRY_TEST_DISTANCE1_HPP
