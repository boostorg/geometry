// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html


#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_envelope_on_spheroid
#endif

#include <boost/test/included/unit_test.hpp>

#include <limits>
#include <iostream>
#include <string>

#include <geometry_test_common.hpp>
#include <from_wkt.hpp>

#include <boost/type_traits/is_same.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/util/condition.hpp>

#include <boost/geometry/io/dsv/write.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>

#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/reverse.hpp>


typedef bg::cs::spherical_equatorial<bg::radian> se_rad_type;
typedef bg::cs::spherical_equatorial<bg::degree> se_deg_type;

typedef bg::model::point<double, 2, se_rad_type> rad_point_type;
typedef bg::model::point<double, 2, se_deg_type> deg_point_type;

typedef bg::model::multi_point<rad_point_type> rad_multipoint_type;
typedef bg::model::multi_point<deg_point_type> deg_multipoint_type;

typedef bg::model::segment<rad_point_type> rad_segment_type;
typedef bg::model::segment<deg_point_type> deg_segment_type;

typedef bg::model::box<rad_point_type> rad_box_type;
typedef bg::model::box<deg_point_type> deg_box_type;

typedef bg::model::linestring<rad_point_type> rad_linestring_type;
typedef bg::model::linestring<deg_point_type> deg_linestring_type;

typedef bg::model::multi_linestring
    <
        rad_linestring_type
    > rad_multilinestring_type;

typedef bg::model::multi_linestring
    <
        deg_linestring_type
    > deg_multilinestring_type;


typedef bg::model::ring<rad_point_type> rad_cw_ring_type;
typedef bg::model::ring<deg_point_type> deg_cw_ring_type;

typedef bg::model::ring<rad_point_type, false> rad_ccw_ring_type;
typedef bg::model::ring<deg_point_type, false> deg_ccw_ring_type;


template <typename Units>
char const* units2string()
{
    if (BOOST_GEOMETRY_CONDITION((boost::is_same<Units, bg::degree>::value)))
    {
        return "degrees";
    }
    return "radians";
}

template <typename Units>
struct other_system_info
{
    typedef bg::degree units;
    typedef bg::cs::spherical_equatorial<units> type;

    template <typename T>
    static inline T convert(T const& value)
    {
        return value * bg::math::r2d<T>();
    }
};

template <>
struct other_system_info<bg::degree>
{
    typedef bg::radian units;
    typedef bg::cs::spherical_equatorial<units> type;

    template <typename T>
    static inline T convert(T const& value)
    {
        return value * bg::math::d2r<T>();
    }
};


class equals_with_tolerance
{
private:
    double m_tolerance;

    template <typename T>
    static inline T const& get_max(T const& a, T const& b, T const& c)
    {
        return (std::max)((std::max)(a, b), c);
    }

    template <typename T>
    static inline bool check_close(T const& a, T const& b, double tol)
    {
        return (a == b)
            || (std::abs(a - b) <= tol * get_max(std::abs(a), std::abs(b), 1.0));
    }

public:
    equals_with_tolerance(double tolerance) : m_tolerance(tolerance) {}

    template <typename T>
    inline bool operator()(T const& value1, T const& value2) const
    {
        return check_close(value1, value2, m_tolerance);
    }
};


template <typename Box1, typename Box2>
inline bool box_equals(Box1 const& box1, Box2 const& box2, double tol)
{
    equals_with_tolerance equals(tol);

    return equals(bg::get<0, 0>(box1), bg::get<0, 0>(box2))
        && equals(bg::get<0, 1>(box1), bg::get<0, 1>(box2))
        && equals(bg::get<1, 0>(box1), bg::get<1, 0>(box2))
        && equals(bg::get<1, 1>(box1), bg::get<1, 1>(box2));
}


template <typename MBR>
class envelope_on_spheroid_basic_tester
{
private:
    template <typename Geometry, typename Box>
    static inline void check_message(bool same_boxes,
                                     std::string const& case_id,
                                     std::string const& units_str,
                                     Geometry const& geometry,
                                     Box const& expected,
                                     Box const& detected)
    {
        bool const is_box = boost::is_same
            <
                typename bg::tag<Geometry>::type, bg::box_tag
            >::value;

        bool const is_segment = boost::is_same
            <
                typename bg::tag<Geometry>::type, bg::segment_tag
            >::value;

        std::ostringstream stream;
        stream << "case ID: " << case_id << ", "
               << "MBR units: " << units_str << "; "
               << "geometry: ";

        if (BOOST_GEOMETRY_CONDITION(is_box))
        {
            stream << "BOX" << bg::dsv(geometry);
        }
        else if (BOOST_GEOMETRY_CONDITION(is_segment))
        {
            stream << "SEGMENT" << bg::dsv(geometry);
        }
        else
        {
            stream << bg::wkt(geometry);
        }

        stream << "; " << "expected: " << bg::dsv(expected)
               << ", " << "detected: " << bg::dsv(detected);

        BOOST_CHECK_MESSAGE(same_boxes, stream.str());
    }

    template <typename Box, typename Geometry>
    static inline void base_test(std::string const& case_id,
        Geometry const& geometry,
        double lon_min, double lat_min,
        double lon_max, double lat_max,
        double tolerance)
    {
        typedef typename bg::coordinate_system<Box>::type::units box_units_type;

        std::string const units_str = units2string<box_units_type>();

        Box detected;
        bg::envelope(geometry, detected);

        Box expected;
        bg::assign_values(expected, lon_min, lat_min, lon_max, lat_max);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        bool const is_box = boost::is_same
            <
                typename bg::tag<Geometry>::type, bg::box_tag
            >::value;

        bool const is_segment = boost::is_same
            <
                typename bg::tag<Geometry>::type, bg::segment_tag
            >::value;

        std::cout << "geometry: ";
        if (BOOST_GEOMETRY_CONDITION(is_box))
        {
            std::cout << "BOX" << bg::dsv(geometry);
        }
        else if(BOOST_GEOMETRY_CONDITION(is_segment))
        {
            std::cout << "SEGMENT" << bg::dsv(geometry);
        }
        else
        {
            std::cout << bg::wkt(geometry);
        }

        std::cout << std::endl
                  << "MBR units: " << units_str
                  << std::endl
                  << "expected: " << bg::dsv(expected)
                  << std::endl
                  << "detected: " << bg::dsv(detected)
                  << std::endl << std::endl;
#endif

        check_message(box_equals(detected, expected, tolerance),
                      case_id, units_str,
                      geometry, expected, detected);
    }

public:
    template <typename Geometry>
    static inline void apply(std::string const& case_id,
        Geometry const& geometry,
        double lon_min, double lat_min,
        double lon_max, double lat_max,
        double tolerance)
    {
        typedef other_system_info
            <
                typename bg::coordinate_system<MBR>::type::units
            > other;

        typedef bg::model::box
            <
                bg::model::point
                    <
                        typename bg::coordinate_type<MBR>::type,
                        2,
                        typename other::type
                    >
            > other_mbr_type;

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << std::endl << std::endl;
        std::cout << "case ID: " << case_id << std::endl << std::endl;
#endif

        base_test<MBR>(case_id, geometry,
                       lon_min, lat_min, lon_max, lat_max,
                       tolerance);

        base_test<other_mbr_type>(case_id, geometry,
                                  other::convert(lon_min),
                                  other::convert(lat_min),
                                  other::convert(lon_max),
                                  other::convert(lat_max),
                                  tolerance);
    }
};


// test the reverse of a geometry if it is either linear or ring
template <typename Geometry, typename Tag = typename bg::tag<Geometry>::type>
struct test_reverse_geometry
{
    static bool const is_linear =
        boost::is_same<Tag, bg::segment_tag>::value
        || boost::is_same<Tag, bg::linestring_tag>::value
        || boost::is_same<Tag, bg::multi_linestring_tag>::value;

    // currently disable rings
    static bool const is_ring = false;
    //    static bool const is_ring = boost::is_same<Tag, bg::ring_tag>::value;

    typedef typename boost::mpl::if_c
        <
            is_linear || is_ring,
            boost::true_type,
            boost::false_type
        >::type type;

    static bool const value = type::value;
};

template
<
    typename Geometry,
    typename MBR,
    typename Tag = typename bg::tag<Geometry>::type,
    bool TestReverse = test_reverse_geometry<Geometry>::value
>
struct test_envelope_on_spheroid
{
    static inline void apply(std::string const& case_id,
        Geometry const& geometry,
        double lon_min1, double lat_min1,
        double lon_max1, double lat_max1,
        double lon_min2, double lat_min2,
        double lon_max2, double lat_max2,
        double tolerance = std::numeric_limits<double>::epsilon())
    {
        envelope_on_spheroid_basic_tester
            <
                MBR
            >::apply(case_id, geometry,
                     lon_min1, lat_min1, lon_max1, lat_max1,
                     tolerance);

        if (BOOST_GEOMETRY_CONDITION(TestReverse))
        {
            std::string reversed_case_id = case_id + "-reverse";

            Geometry reversed_geometry = geometry;
            bg::reverse(reversed_geometry);
            envelope_on_spheroid_basic_tester
                <
                    MBR
                >::apply(reversed_case_id, reversed_geometry,
                         lon_min2, lat_min2, lon_max2, lat_max2,
                         tolerance);        
        }

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << "=================="
                  << std::endl << std::endl;
#endif
    }

    static inline void apply(std::string const& case_id,
        Geometry const& geometry,
        double lon_min, double lat_min,
        double lon_max, double lat_max,
        double tolerance = std::numeric_limits<double>::epsilon())
    {
        apply(case_id, geometry,
              lon_min, lat_min, lon_max, lat_max,
              lon_min, lat_min, lon_max, lat_max,
              tolerance);
    }
};


// special tester for rings
template <typename Geometry, typename MBR, bool TestReverse>
struct test_envelope_on_spheroid<Geometry, MBR, bg::ring_tag, TestReverse>
{
    static inline void apply(std::string const& case_id,
        Geometry const& geometry,
        double lon_min1, double lat_min1,
        double lon_max1, double lat_max1,
        double lon_min2, double lat_min2,
        double lon_max2, double lat_max2,
        double tolerance = std::numeric_limits<double>::epsilon())
    {
        envelope_on_spheroid_basic_tester
            <
                MBR
            >::apply(case_id, geometry,
                     lon_min1, lat_min1, lon_max1, lat_max1,
                     tolerance);

        std::string ccw_case_id = case_id + "-2ccw";

        deg_ccw_ring_type ccw_ring;            
        bg::convert(geometry, ccw_ring);
            
        envelope_on_spheroid_basic_tester
            <
                MBR
            >::apply(ccw_case_id, ccw_ring,
                     lon_min2, lat_min2, lon_max2, lat_max2,
                     tolerance);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << "=================="
                  << std::endl << std::endl;
#endif
    }

    static inline void apply(std::string const& case_id,
        Geometry const& geometry,
        double lon_min, double lat_min,
        double lon_max, double lat_max,
        double tolerance = std::numeric_limits<double>::epsilon())
    {
        apply(case_id, geometry,
              lon_min, lat_min, lon_max, lat_max,
              lon_min, lat_min, lon_max, lat_max,
              tolerance);
    }
};


BOOST_AUTO_TEST_CASE( envelope_point )
{
    typedef deg_point_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    tester::apply("p01",
                  from_wkt<G>("POINT(10 10)"),
                  10, 10, 10, 10);

    tester::apply("p02",
                  from_wkt<G>("POINT(370 10)"),
                  10, 10, 10, 10);

#ifdef BOOST_GEOMETRY_NORMALIZE_LATITUDE
    tester::apply("p03",
                  from_wkt<G>("POINT(370 -350)"),
                  10, 10, 10, 10);
#endif

    // north and south poles
    tester::apply("p04",
                  from_wkt<G>("POINT(0 90)"),
                  0, 90, 0, 90);

    tester::apply("p04a",
                  from_wkt<G>("POINT(10 90)"),
                  0, 90, 0, 90);

    tester::apply("p04b",
                  from_wkt<G>("POINT(270 90)"),
                  0, 90, 0, 90);

#ifdef BOOST_GEOMETRY_NORMALIZE_LATITUDE
    tester::apply("p04c",
                  from_wkt<G>("POINT(270 450)"),
                  0, 90, 0, 90);
#endif

    tester::apply("p04d",
                  from_wkt<G>("POINT(190 90)"),
                  0, 90, 0, 90);

    tester::apply("p04e",
                  from_wkt<G>("POINT(-100 90)"),
                  0, 90, 0, 90);

    tester::apply("p05",
                  from_wkt<G>("POINT(0 -90)"),
                  0, -90, 0, -90);

    tester::apply("p05a",
                  from_wkt<G>("POINT(10 -90)"),
                  0, -90, 0, -90);

    tester::apply("p05b",
                  from_wkt<G>("POINT(270 -90)"),
                  0, -90, 0, -90);

#ifdef BOOST_GEOMETRY_NORMALIZE_LATITUDE
    tester::apply("p05c",
                  from_wkt<G>("POINT(270 -450)"),
                  0, -90, 0, -90);
#endif

    tester::apply("p05d",
                  from_wkt<G>("POINT(190 -90)"),
                  0, -90, 0, -90);

    tester::apply("p05e",
                  from_wkt<G>("POINT(-100 -90)"),
                  0, -90, 0, -90);

    tester::apply("p05f",
                  from_wkt<G>("POINT(-100 -90)"),
                  0, -90, 0, -90);
}


BOOST_AUTO_TEST_CASE( envelope_segment )
{
    typedef deg_segment_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    tester::apply("s01",
                  from_wkt<G>("SEGMENT(10 10,40 40)"),
                  10, 10, 40, 40);

    tester::apply("s02",
                  from_wkt<G>("SEGMENT(10 10,40 10)"),
                  10, 10, 40, 10.34527004614999);

    tester::apply("s02a",
                  from_wkt<G>("SEGMENT(40 10,10 10)"),
                  10, 10, 40, 10.34527004614999);

    tester::apply("s03",
                  from_wkt<G>("SEGMENT(160 10,-170 10)"),
                  160, 10, 190, 10.34527004614999);

    tester::apply("s03a",
                  from_wkt<G>("SEGMENT(-170 10,160 10)"),
                  160, 10, 190, 10.34527004614999);

    tester::apply("s03b",
                  from_wkt<G>("SEGMENT(-170 -10,160 -10)"),
                  160,  -10.34527004614999, 190, -10);

    tester::apply("s04",
                  from_wkt<G>("SEGMENT(-40 45,140 60)"),
                  -40, 45, 140, 90);

    tester::apply("s04a",
                  from_wkt<G>("SEGMENT(-40 45,140 25)"),
                  -40, 25, 140, 90);

    // segment ending at the north pole
    tester::apply("s05",
                  from_wkt<G>("SEGMENT(40 45,80 90)"),
                  40, 45, 40, 90);

    // segment starting at the north pole
    tester::apply("s05a",
                  from_wkt<G>("SEGMENT(80 90,40 45)"),
                  40, 45, 40, 90);

    // segment ending at the north pole
    tester::apply("s06",
                  from_wkt<G>("SEGMENT(-40 45,80 90)"),
                  -40, 45, -40, 90);

    // segment starting at the north pole
    tester::apply("s06a",
                  from_wkt<G>("SEGMENT(70 90,-40 45)"),
                  -40, 45, -40, 90);

    // segment ending at the north pole
    tester::apply("s07",
                  from_wkt<G>("SEGMENT(40 -45,80 90)"),
                  40, -45, 40, 90);

    // segment passing through the south pole
    tester::apply("s08",
                  from_wkt<G>("SEGMENT(-170 -45,10 -30)"),
                  -170, -90, 10, -30);

    tester::apply("s09",
                  from_wkt<G>("SEGMENT(1 -45,179 30)"),
                  1, -85.28884376852972, 179, 30);

    tester::apply("s09a",
                  from_wkt<G>("SEGMENT(2 -45,181 30)"),
                  2, -87.63659983704828, 181, 30);

    // very long segment
    tester::apply("s10",
                  from_wkt<G>("SEGMENT(0 -45,181 30)"),
                  -179, -88.07047433509489, 0, 30,
                  2.0 * std::numeric_limits<double>::epsilon());

    tester::apply("s11",
                  from_wkt<G>("SEGMENT(260 30,20 45)"),
                  -100, 30, 20, 57.93195594009233);

    tester::apply("s11a",
                  from_wkt<G>("SEGMENT(260 45,20 30)"),
                  -100, 30, 20, 49.43962741846035);

    // segment degenerating to the north pole
    tester::apply("s12",
                  from_wkt<G>("SEGMENT(10 90,20 90)"),
                  0, 90, 0, 90);

    // segment degenerating to the south pole
    tester::apply("s13",
                  from_wkt<G>("SEGMENT(10 -90,20 -90)"),
                  0, -90, 0, -90);

    tester::apply("s14",
                  from_wkt<G>("SEGMENT(20 20,10 30)"),
                  10, 20, 20, 30);//48.87458730907602);

    tester::apply("s15",
                  from_wkt<G>("SEGMENT(50 45,185 45)"),
                  50, 45, 185, 69.05897952775615);

    // segment that lies on the equator
    tester::apply("s16",
                  from_wkt<G>("SEGMENT(0 0,50 0)"),
                  0, 0, 50, 0);

    // segment that lies on the equator
    tester::apply("s16a",
                  from_wkt<G>("SEGMENT(-50 0,50 0)"),
                  -50, 0, 50, 0);

    // segment that lies on the equator and touches antimeridian
    tester::apply("s16b",
                  from_wkt<G>("SEGMENT(50 0,180 0)"),
                  50, 0, 180, 0);

    // segment that lies on the equator and crosses antimeridian
    tester::apply("s16c",
                  from_wkt<G>("SEGMENT(-170 0,160 0)"),
                  160, 0, 190, 0);

    tester::apply("s17",
                  from_wkt<G>("SEGMENT(140 10, -140 80)"),
                  140, 10, 220, 80);

    tester::apply("s17-r",
                  from_wkt<G>("SEGMENT(-140 80, 140 10)"),
                  140, 10, 220, 80);

    tester::apply("s18",
                  from_wkt<G>("SEGMENT(20 10, 100 80)"),
                  20, 10, 100, 80);

    tester::apply("s18-r",
                  from_wkt<G>("SEGMENT(100 80, 20 10)"),
                  20, 10, 100, 80);

    // segment connecting the north and south pole
    //
    // this should be forbidden actually, as it is not well-defined
    // with this test we demonstrate that the algorithm still returns
    // something meaningful
    tester::apply("s99",
                  from_wkt<G>("SEGMENT(10 90,20 -90)"),
                  0, -90, 0, 90);
}


BOOST_AUTO_TEST_CASE( envelope_multipoint )
{
    typedef deg_multipoint_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    tester::apply("mp01",
                  from_wkt<G>("MULTIPOINT(0 0,10 10)"),
                  0, 0, 10, 10);

    tester::apply("mp02",
                  from_wkt<G>("MULTIPOINT(0 10,10 0)"),
                  0, 0, 10, 10);

    tester::apply("mp03",
                  from_wkt<G>("MULTIPOINT(-10 20,0 10,10 0)"),
                  -10, 0, 10, 20);

    tester::apply("mp04",
                  from_wkt<G>("MULTIPOINT(-10 20,0 10,10 -15)"),
                  -10, -15, 10, 20);

    tester::apply("mp05",
                  from_wkt<G>("MULTIPOINT(-85 10,85 -20)"),
                  -85, -20, 85, 10);

    tester::apply("mp06",
                  from_wkt<G>("MULTIPOINT(-95 10,85 -20)"),
                  -95, -20, 85, 10);

    tester::apply("mp07",
                  from_wkt<G>("MULTIPOINT(-96 10,85 -20)"),
                  85, -20, -96+360, 10);

    tester::apply("mp08",
                  from_wkt<G>("MULTIPOINT(175 15,-175 -20)"),
                  175, -20, -175+360, 15);

    tester::apply("mp09",
                  from_wkt<G>("MULTIPOINT(170 15,170 20,-175 10,-90 10,10 10)"),
                  170, 10, 10+360, 20);

    // this should fail
    tester::apply("mp09a",
                  from_wkt<G>("MULTIPOINT(10 10,170 15,170 20,-175 10,-90 10)"),
                  170, 10, 10+360, 20);


    tester::apply("mp10",
                  from_wkt<G>("MULTIPOINT(10 10,20 90,30 -90)"),
                  10, -90, 10, 90);

    // this should fail
    tester::apply("mp11",
                  from_wkt<G>("MULTIPOINT(179 90,-179 -90,10 10)"),
                  10, -90, 10, 90);

    tester::apply("mp11a",
                  from_wkt<G>("MULTIPOINT(10 10,179 90,-179 -90)"),
                  10, -90, 10, 90);

    // this should fail
    tester::apply("mp11b",
                  from_wkt<G>("MULTIPOINT(179 90,-179 -90,-90 0)"),
                  -90, -90, -90, 90);

    tester::apply("mp11c",
                  from_wkt<G>("MULTIPOINT(-90 0,179 90,-179 -90,-90 0)"),
                  -90, -90, -90, 90);

    tester::apply("mp12",
                  from_wkt<G>("MULTIPOINT(170 -30,175 60,-178 10)"),
                  170, -30, -178+360, 60);

    tester::apply("mp13",
                  from_wkt<G>("MULTIPOINT(-170 -30,-175 40,178 50)"),
                  178, -30, -170+360, 50);

    tester::apply("mp13a",
                  from_wkt<G>("MULTIPOINT(-170 -30,178 50)"),
                  178, -30, -170+360, 50);

    tester::apply("mp13b",
                  from_wkt<G>("MULTIPOINT(-170 -30,178 50,-175 40)"),
                  178, -30, -170+360, 50);

    tester::apply("mp15",
                  from_wkt<G>("MULTIPOINT(10 -20)"),
                  10, -20, 10, -20);

    tester::apply("mp16",
                  from_wkt<G>("MULTIPOINT(0 90,10 90)"),
                  0, 90, 0, 90);

    tester::apply("mp17",
                  from_wkt<G>("MULTIPOINT(179 80,-179 -80,10 10)"),
                  10, -80, -179+360, 80);

    tester::apply("mp17a",
                  from_wkt<G>("MULTIPOINT(10 10,179 80,-179 -80)"),
                  10, -80, -179+360, 80);

    tester::apply("mp17b",
                  from_wkt<G>("MULTIPOINT(179 80,-179 -80,-90 0)"),
                  179, -80, -90+360, 80);

    tester::apply("mp17c",
                  from_wkt<G>("MULTIPOINT(-90 0,179 80,-179 -80,-90 0)"),
                  179, -80, -90+360, 80);

    tester::apply("mp18",
                  from_wkt<G>("MULTIPOINT(-170 45,20 25,40 40)"),
                  20, 25, 190, 45);

#ifdef BOOST_GEOMETRY_NORMALIZE_LATITUDE
    tester::apply("mp18a",
                  from_wkt<G>("MULTIPOINT(10 135,20 25,40 40)"),
                  20, 25, 190, 45);
#endif

    tester::apply("mp19",
                  from_wkt<G>("MULTIPOINT(350 45,20 25,40 40)"),
                  -10, 25, 40, 45);

#ifdef BOOST_GEOMETRY_NORMALIZE_LATITUDE
    tester::apply("mp19a",
                  from_wkt<G>("MULTIPOINT(170 135,20 25,40 40)"),
                  -10, 25, 40, 45);
#endif
}


BOOST_AUTO_TEST_CASE( envelope_box )
{
    typedef deg_box_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    tester::apply("b01",
                  from_wkt<G>("BOX(10 10,20 20)"),
                  10, 10, 20, 20);

#ifdef BOOST_GEOMETRY_NORMALIZE_LATITUDE
    tester::apply("b02",
                  from_wkt<G>("BOX(10 370,20 20)"),
                  10, 10, 20, 20);
#endif

    // box crosses anti-meridian
    tester::apply("b02a",
                  from_wkt<G>("BOX(170 10,-170 20)"),
                  170, 10, 190, 20);

    tester::apply("b03",
                  from_wkt<G>("BOX(-170 10,170 20)"),
                  -170, 10, 170, 20);

    tester::apply("b04",
                  from_wkt<G>("BOX(10 10,350 20)"),
                  10, 10, 350, 20);

    tester::apply("b04a",
                  from_wkt<G>("BOX(10 10,-10 20)"),
                  10, 10, 350, 20);

    // box is a band
    tester::apply("b05",
                  from_wkt<G>("BOX(0 10,360 20)"),
                  -180, 10, 180, 20);

    tester::apply("b05a",
                  from_wkt<G>("BOX(0 10,0 20)"),
                  0, 10, 0, 20);

    // box is almost a band
    tester::apply("b06",
                  from_wkt<G>("BOX(10 10,5 20)"),
                  10, 10, 365, 20);

    // initial box is a band that crosses itself
    tester::apply("b07",
                  from_wkt<G>("BOX(0 10,540 20)"),
                  -180, 10, 180, 20);

    // initial box is a band that crosses itself
    tester::apply("b08",
                  from_wkt<G>("BOX(0 10,720 20)"),
                  -180, 10, 180, 20);

    tester::apply("b09",
                  from_wkt<G>("BOX(10 10,10 10)"),
                  10, 10, 10, 10);

    tester::apply("b09a",
                  from_wkt<G>("BOX(370 10,370 10)"),
                  10, 10, 10, 10);

    // box contains north and south pole
    tester::apply("b10",
                  from_wkt<G>("BOX(0 -90,0 90)"),
                  0, -90, 0, 90);

    // box contains north and south pole
    tester::apply("b10a",
                  from_wkt<G>("BOX(10 -90,10 90)"),
                  10, -90, 10, 90);

    // box contains north and south pole
    tester::apply("b10b",
                  from_wkt<G>("BOX(0 -90,10 90)"),
                  0, -90, 10, 90);

    // box contains north and south pole
    tester::apply("b11",
                  from_wkt<G>("BOX(0 -90,180 90)"),
                  0, -90, 180, 90);

    // box contains north and south pole
    tester::apply("b11a",
                  from_wkt<G>("BOX(10 -90,190 90)"),
                  10, -90, 190, 90);

    // box contains north and south pole
    tester::apply("b11b",
                  from_wkt<G>("BOX(10 -90,110 90)"),
                  10, -90, 110, 90);

    // box contains north and south pole and is a band
    // (box covers the entire spheroid)
    tester::apply("b12",
                  from_wkt<G>("BOX(0 -90,360 90)"),
                  -180, -90, 180, 90);

    // box contains north and south pole and is a band
    // (box covers the entire spheroid)
    tester::apply("b12a",
                  from_wkt<G>("BOX(10 -90,370 90)"),
                  -180, -90, 180, 90);

    // box contains north and south pole and is a band
    // (box covers the entire spheroid)
    tester::apply("b12b",
                  from_wkt<G>("BOX(-175 -90,185 90)"),
                  -180, -90, 180, 90);

    // box contains north and south pole and is a band
    // (box covers the entire spheroid)
    tester::apply("b12c",
                  from_wkt<G>("BOX(-175 -90,185 90)"),
                  -180, -90, 180, 90);

    // box contains north and south pole and is a band and is self-intersecting
    // (box covers the entire spheroid)
    tester::apply("b12d",
                  from_wkt<G>("BOX(-175 -90,186 90)"),
                  -180, -90, 180, 90);

    // box contains north and south pole and is a band and is self-intersecting
    // (box covers the entire spheroid)
    tester::apply("b12e",
                  from_wkt<G>("BOX(0 -90,540 90)"),
                  -180, -90, 180, 90);

    // box contains north and south pole and is a band and is self-intersecting
    // (box covers the entire spheroid)
    tester::apply("b12f",
                  from_wkt<G>("BOX(10 -90,540 90)"),
                  -180, -90, 180, 90);

    // box is a band
    tester::apply("b13",
                  from_wkt<G>("BOX(180 -10,-180 10)"),
                  -180, -10, 180, 10);

    // box contains north and south pole and is a band
    // (box covers the entire spheroid)
    tester::apply("b13a",
                  from_wkt<G>("BOX(180 -90,-180 90)"),
                  -180, -90, 180, 90);

    tester::apply("b14",
                  from_wkt<G>("BOX(0 10,30 90)"),
                  0, 10, 30, 90);

    tester::apply("b15",
                  from_wkt<G>("BOX(179 10,178 70)"),
                  179, 10, 538, 70);

    // box contains north pole
    tester::apply("b16",
                  from_wkt<G>("BOX(10 40,20 90)"),
                  10, 40, 20, 90);

    tester::apply("b16a",
                  from_wkt<G>("BOX(170 40,-170 90)"),
                  170, 40, 190, 90);

    // box contains south pole
    tester::apply("b17",
                  from_wkt<G>("BOX(10 -90,20 40)"),
                  10, -90, 20, 40);

    tester::apply("b17a",
                  from_wkt<G>("BOX(150 -90,-150 40)"),
                  150, -90, 210, 40);

    // box degenerates to the north pole
    tester::apply("b98",
                  from_wkt<G>("BOX(10 90,20 90)"),
                  0, 90, 0, 90);

    // box degenerates to the south pole
    tester::apply("b99",
                  from_wkt<G>("BOX(10 -90,20 -90)"),
                  0, -90, 0, -90);
}


BOOST_AUTO_TEST_CASE( envelope_linestring )
{
    typedef deg_linestring_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    tester::apply("l01",
                  from_wkt<G>("LINESTRING(10 15)"),
                  10, 15, 10, 15);

    tester::apply("l01a",
                  from_wkt<G>("LINESTRING(370 15)"),
                  10, 15, 10, 15);

    tester::apply("l01b",
                  from_wkt<G>("LINESTRING(370 90)"),
                  0, 90, 0, 90);

    tester::apply("l02",
                  from_wkt<G>("LINESTRING(10 10,20 20,10 30)"),
                  10, 10, 20, 30);

    // linestring that circles the entire globe
    tester::apply("l03",
                  from_wkt<G>("LINESTRING(-185 0,-170 25,-50 10,10 10,20 20,100 5,180 15)"),
                  -180, 0, 180, 25.15036418555258,
                  4.0 * std::numeric_limits<double>::epsilon());

    // linestring that crosses the antimeridian but staying close to it
    tester::apply("l04",
                  from_wkt<G>("LINESTRING(-170 10,170 45,160 5,-160 25)"),
                  160, 5, 200, 45);

    // linestring that goes through the north pole (twice)
    tester::apply("l05",
                  from_wkt<G>("LINESTRING(-170 80,10 60,20 80,-160 30)"),
                  -170, 30, 20, 90);

    // linestring that goes through the north pole (three times)
    tester::apply("l05a",
                  from_wkt<G>("LINESTRING(-170 80,10 60,20 80,-160 30,-150 30,30 70)"),
                  -170, 30, 30, 90);

    // linestring that goes through the north pole (four times)
    tester::apply("l05b",
                  from_wkt<G>("LINESTRING(-170 80,10 60,20 80,-160 30,-150 30,30 70,40 85,-140 25)"),
                  -170, 25, 40, 90);

    // linestring that goes through the north pole (five times)
    tester::apply("l05c",
                  from_wkt<G>("LINESTRING(-170 80,10 60,20 80,-160 30,-150 30,30 70,40 85,-140 25,-130 25,50 45)"),
                  -170, 25, 50, 90);

    // linestring that goes through the north pole (five times)
    tester::apply("l05d",
                  from_wkt<G>("LINESTRING(-170 80,10 60,20 80,-160 30,-150 30,30 70,40 85,-140 25,-130 25,50 45,185 45)"),
                  -170, 25, 185, 90);

    // linestring that crosses the antimeridian
    tester::apply("l06",
                  from_wkt<G>("LINESTRING(-160 85,-170 80,170 40,160 80)"),
                  160, 40, 200, 85);

    // linestring that crosses the antimeridian
    tester::apply("l06a",
                  from_wkt<G>("LINESTRING(-130 85,-170 84,170 40,160 80)"),
                  160, 40, 230, 85.02629680862029);

    // linestring that goes through the north pole
    tester::apply("l07",
                  from_wkt<G>("LINESTRING(-160 40,-170 90,-140 40,-50 30)"),
                  -160, 30, -50, 90);

    // linestring that degenerates to the north pole
    tester::apply("l08",
                  from_wkt<G>("LINESTRING(-40 90,-30 90,-140 90,10 90)"),
                  0, 90, 0, 90);

    // linestring with duplicate points
    tester::apply("l09",
                  from_wkt<G>("LINESTRING(-40 20,-40 20,-140 85,-10 5,-10 5)"),
                  -140, 5, -10, 85);

    // linestring with duplicate points
    tester::apply("l09a",
                  from_wkt<G>("LINESTRING(-40 20,320 20,-140 85,-10 5,350 5)"),
                  -140, 5, -10, 85);

    // linestring that lies on the equator
    tester::apply("l10",
                  from_wkt<G>("LINESTRING(0 0,50 0)"),
                  0, 0, 50, 0);

    // linestring that lies on the equator
    tester::apply("l10a",
                  from_wkt<G>("LINESTRING(-50 0,50 0)"),
                  -50, 0, 50, 0);

    // linestring that lies on the equator and touches antimeridian
    tester::apply("l10b",
                  from_wkt<G>("LINESTRING(50 0,180 0)"),
                  50, 0, 180, 0);

    // linestring that lies on the equator and crosses antimeridian
    tester::apply("l10c",
                  from_wkt<G>("LINESTRING(-170 0,160 0)"),
                  160, 0, 190, 0);
}


BOOST_AUTO_TEST_CASE( envelope_multilinestring )
{
    typedef deg_multilinestring_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    tester::apply("ml01",
                  from_wkt<G>("MULTILINESTRING((),(),(10 15),())"),
                  10, 15, 10, 15);

    tester::apply("ml02",
                  from_wkt<G>("MULTILINESTRING((-170 40,-100 80,10 40),(-10 25,10 35,100 45),(50 30,150 45,-160 30))"),
                  -180, 25, 180, 80);

    tester::apply("ml03",
                  from_wkt<G>("MULTILINESTRING((-150 40,-100 80,10 40),(-10 25,10 35,100 45),(50 30,150 45,-160 30))"),
                  -150, 25, 200, 80);

    tester::apply("ml04",
                  from_wkt<G>("MULTILINESTRING((-150 40,-100 80),(10 35,100 80))"),
                  -150, 35, 100, 80.07385383411011);

    tester::apply("ml04a",
                  from_wkt<G>("MULTILINESTRING((-150 40,-100 80),(10 35,100 80),(170 25,-160 80))"),
                  10, 25, 260, 80.07385383411011);

    tester::apply("ml05",
                  from_wkt<G>("MULTILINESTRING((-140 40,-100 80),(10 35,100 80))"),
                  -140, 35, 100, 80.07385383411011);

    tester::apply("ml05a",
                  from_wkt<G>("MULTILINESTRING((-140 40,-100 80),(10 35,100 80),(170 25,-160 80))"),
                  10, 25, 260, 80.07385383411011);
}


#if 0
// unit test for rings de-activated for now (current implementation
// for area on the spherical equatorial coordinate system is not complete)
// TODO: re-activate once implementation is done
BOOST_AUTO_TEST_CASE( envelope_cw_ring )
{
    typedef deg_cw_ring_type G;
    typedef test_envelope_on_spheroid<G, deg_box_type> tester;

    double const eps = std::numeric_limits<double>::epsilon();

    tester::apply("r01cw",
                  from_wkt<G>("POLYGON((0 10,0 45,50 10,0 10))"),
                  0, 10, 50, 45);

    // ring that contains both the north and south poles in its interior
    tester::apply("r01cw-r",
                  from_wkt<G>("POLYGON((0 10,50 10,0 45,0 10))"),
                  -180, -90, 180, 90);

    // ring that contains the north pole in its interior
    tester::apply("r02cw",
                  from_wkt<G>("POLYGON((0 0,-50 0,-170 0,180 0,100 0,0 0))"),
                  -180, 0, 180, 90);
    //                  -180, -90, 180, 0);

    // ring that contains the south pole in its interior
    tester::apply("r02cw-r",
                  from_wkt<G>("POLYGON((0 0,100 0,180 0,-170 0,-50 0,0 0))"),
                  -180, -90, 180, 0);
                  //                  -180, 0, 180, 90);

    // ring that contains the north pole in its interior
    tester::apply("r03cw",
                  from_wkt<G>("POLYGON((0 -10,-50 -10,-170 -10,180 -10,100 -10,0 -10))"),
                  -180, -19.42540014068282, 180, 90);
                  //                  -180, -90, 180, -10);

    // ring that contains both the south and north poles in its interior
    tester::apply("r03cw-r",
                  from_wkt<G>("POLYGON((0 -10,100 -10,180 -10,-170 -10,-50 -10,0 -10))"),
                  -180, -90, 180, -10);
    //                  -180, -19.42540014068282, 180, 90);

    // ring that has the north pole as vertex and contains the south pole
    tester::apply("r04cw",
                  from_wkt<G>("POLYGON((0 0,-50 90,-50 0,0 0))"),
                  -180, -90, 180, 90);

    // ring that has the north pole as vertex
    tester::apply("r04cw-r",
                  from_wkt<G>("POLYGON((0 0,-50 0,-50 90,0 0))"),
                  -50, 0, 0, 90);

    // ring that crosses antimeridian but does not contain any pole
    tester::apply("r05cw",
                  from_wkt<G>("POLYGON((-140 0,140 10,-140 80,-140 0))"),
                  140, 0, 220, 80);

    // ring that crosses antimeridian multiple times but does not
    // contain any pole
    tester::apply("r06cw",
                  from_wkt<G>("POLYGON((-140 10,140 10,140 80,-140 80,-140 10))"),
                  140, 10, 220, 82.30737118075496,
                  2 * eps);

    // ring that crosses antimeridian multiple times but does not
    // contain any pole
    tester::apply("r07cw",
                  from_wkt<G>("POLYGON((-140 10,140 10,180 80,-140 10))"),
                  140, 10, 220, 80);

    // ring that goes through the north pole
    tester::apply("r08cw",
                  from_wkt<G>("POLYGON((0 0,-50 0,-50 90,0 0))"),
                  -50, 0, 0, 90);

    // ring that goes through the south pole and contains the north pole
    tester::apply("r09cw",
                  from_wkt<G>("POLYGON((0 0,0 -90,50 0,0 0))"),
                  -180, -90, 180, 90);

    // ring that goes through the south pole
    tester::apply("r09cw-r",
                  from_wkt<G>("POLYGON((0 0,50 0,50 -90,0 0))"),
                  0, -90, 50, 0);

    // ring that goes through both south and north pole
    tester::apply("r10cw",
                  from_wkt<G>("POLYGON((50 0,50 -90,40 0,40 90,50 0))"),
                  40, -90, 50, 90);

    // ring that goes through both south and north pole and crosses antimeridian
    tester::apply("r11cw",
                  from_wkt<G>("POLYGON((-170 0,-170 -90,160 0,160 90,-170 0))"),
                  160, -90, 190, 90);

    // ring with edge that goes through the north pole and contains
    // south pole (the bounding box is the entire globe)
    tester::apply("r12cw",
                  from_wkt<G>("POLYGON((-50 40,130 10,170 80,-50 40))"),
                  -180, -90, 180, 90);
                  //                  -180, 10, 180, 90);

    // ring with edge that goes through the north pole
    tester::apply("r12cw-r",
                  from_wkt<G>("POLYGON((-50 40,170 80,130 10,-50 40))"),
                  -180, 10, 180, 90);
    //                  -180, -90, 180, 90);

    // ring that represents a spherical cap near the north pole
    tester::apply("r13cw",
                  from_wkt<G>("POLYGON((100 45,0 45,-100 45,-100 90,100 45))"),
                  -100, 45, 100, 90);

    // ring that represents the complement of a spherical cap
    // near the north pole
    tester::apply("r13cw-r",
                  from_wkt<G>("POLYGON((-100 45,0 45,100 45,100 90,-100 45))"),
                  -180, -90, 180, 90);

    // ring that represents the complement of a spherical cap
    // that touches the south pole
    tester::apply("r14cw",
                  from_wkt<G>("POLYGON((-100 45,0 45,100 45,100 -90,-100 45))"),
                  -100, -90, 100, 57.26759279038765);

    // ring that represents a spherical cap that touches the south pole
    tester::apply("r14cw-r",
                  from_wkt<G>("POLYGON((100 45,0 45,-100 45,-100 -90,100 45))"),
                  -180, -90, 180, 90);

    // ring with edge that goes through the south pole
    tester::apply("r15cw",
                  from_wkt<G>("POLYGON((-50 -40,130 -10,170 -80,-50 -40))"),
                  -180, -90, 180, -10);

    // ring with edge that goes through the south pole and contains
    // north pole (the bounding box is the entire globe)
    tester::apply("r15cw-r",
                  from_wkt<G>("POLYGON((-50 -40,170 -80,130 -10,-50 -40))"),
                  -180, -90, 180, 90);

    // ring that does not contain any pole and lies in the lower hemisphere
    tester::apply("r16",
                  from_wkt<G>("POLYGON((-50 -80,-50 -40,-30 -40,-30 -80,-50 -80))"),
                  -50, -80.14892388341609, -30, -40);

    // ring that lies in the lower hemisphere and contains both poles
    tester::apply("r16-r",
                  from_wkt<G>("POLYGON((-50 -80,-30 -80,-30 -40,-50 -40,-50 -80))"),
                  -180, -90, 180, 90);

    // ring that goes through the south pole and contains the north pole
    tester::apply("r17cw",
                  from_wkt<G>("POLYGON((50 0,50 -90,100 0,50 0))"),
                  -180, -90, 180, 90);

    // ring that goes through the south pole
    tester::apply("r17cw-r",
                  from_wkt<G>("POLYGON((50 0,100 0,100 -90,50 0))"),
                  50, -90, 100, 0);

    // ring that goes through the south pole and contains the north pole
    tester::apply("r18cw",
                  from_wkt<G>("POLYGON((50 0,50 -90,460 0,50 0))"),
                  -180, -90, 180, 90);

    // ring that goes through the south pole
    tester::apply("r18cw-r",
                  from_wkt<G>("POLYGON((50 0,460 0,100 -90,50 0))"),
                  50, -90, 100, 0);

    // ring that goes through the south pole and contains the north pole
    tester::apply("r19cw",
                  from_wkt<G>("POLYGON((50 0,50 -90,-260 0,50 0))"),
                  -180, -90, 180, 90);

    // ring that goes through the south pole
    tester::apply("r19cw-r",
                  from_wkt<G>("POLYGON((50 0,-260 0,100 -90,50 0))"),
                  50, -90, 100, 0);

    // ring that goes through both poles
    tester::apply("r20cw",
                  from_wkt<G>("POLYGON((10 0,10 90,20 0,20 -90,10 0))"),
                  10, -90, 20, 90); // SUCCEEDS FOR WRONG REASON

    // ring that goes through both poles
    tester::apply("r20cw-r",
                  from_wkt<G>("POLYGON((10 0,10 -90,20 0,20 90,10 0))"),
        -180, -90, 180, 90); // FAILS NOW

    // ring that goes through both poles and its boundary forms
    // a great circle
    tester::apply("r21cw",
                  from_wkt<G>("POLYGON((-10 0,-10 90,170 0,170 -90,-10 0))"),
                  -10, -90, 170, 90); // SUCCEEDS FOR WRONG REASON

    // ring that goes through both poles and its boundary forms
    // a great circle
    tester::apply("r21cw-r",
                  from_wkt<G>("POLYGON((-10 0,-10 -90,170 0,170 90,-10 0))"),
                  170, -90, 350, 90); // FAILS NOW
}
#endif
