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

#include <iostream>
#include <string>

#include <boost/foreach.hpp>

#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/policies/robustness/no_rescale_policy.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/io/svg/svg_mapper.hpp>
#endif

#include <geometry_test_common.hpp>


namespace boost { namespace geometry
{


// Overload with rescale-policy specified (temporary here - to be worked out)
template
<
    typename Geometry1,
    typename Geometry2,
    typename RescalePolicy,
    typename GeometryOut
>
inline bool intersection(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            RescalePolicy const& rescale_policy,
            GeometryOut& geometry_out)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    typedef strategy_intersection
        <
            typename cs_tag<Geometry1>::type,
            Geometry1,
            Geometry2,
            typename geometry::point_type<Geometry1>::type,
            RescalePolicy
        > strategy;

    return dispatch::intersection
        <
           Geometry1,
           Geometry2
        >::apply(geometry1, geometry2, rescale_policy, geometry_out, strategy());
}


}} // namespace boost::geometry


template
<
    typename OutputType,
    typename CalculationType,
    typename Geometry1,
    typename Geometry2,
    typename RescalePolicy
>
typename bg::default_area_result<Geometry1>::type test_intersection(std::string const& caseid,
        Geometry1 const& geometry1, Geometry2 const& geometry2,
        RescalePolicy const& rescale_policy,
        std::size_t expected_count = 0, int expected_point_count = 0,
        double expected_length_or_area = 0,
        double percentage = 0.0001,
        bool debug = false)
{
    typedef typename bg::coordinate_type<Geometry1>::type coordinate_type;
    typedef typename bg::point_type<Geometry1>::type point_type;

//    if (debug)
    {
        std::cout << std::endl
            << "case " << caseid
            << " " << typeid(coordinate_type).name()
            << " " << string_from_type<coordinate_type>::name()
            << std::endl;
    }



    std::vector<OutputType> out;
    bg::intersection(geometry1, geometry2, rescale_policy, out);

    typename bg::default_area_result<Geometry1>::type length_or_area = 0;
    int n = 0;
    for (typename std::vector<OutputType>::iterator it = out.begin();
            it != out.end();
            ++it)
    {
        if (expected_point_count > 0)
        {
            n += bg::num_points(*it, true);
        }

        length_or_area += bg::area(*it);

        if (debug)
        {
            std::cout << std::setprecision(20) << bg::wkt(*it) << std::endl;
        }
    }


#if ! defined(BOOST_GEOMETRY_NO_BOOST_TEST)
    if (expected_point_count > 0)
    {
        BOOST_CHECK_MESSAGE(bg::math::abs(n - expected_point_count) < 3,
                "intersection: " << caseid
                << " #points expected: " << expected_point_count
                << " detected: " << n
                << " type: " << (type_for_assert_message<Geometry1, Geometry2>())
                );
    }

    if (expected_count > 0)
    {
        BOOST_CHECK_MESSAGE(out.size() == expected_count,
                "intersection: " << caseid
                << " #outputs expected: " << expected_count
                << " detected: " << out.size()
                << " type: " << (type_for_assert_message<Geometry1, Geometry2>())
                );
    }

    double const detected_length_or_area = boost::numeric_cast<double>(length_or_area);
    BOOST_CHECK_CLOSE(detected_length_or_area, expected_length_or_area, percentage);
#endif


#if defined(TEST_WITH_SVG)
    {
        bool const ccw =
            bg::point_order<Geometry1>::value == bg::counterclockwise
            || bg::point_order<Geometry2>::value == bg::counterclockwise;
        bool const open =
            bg::closure<Geometry1>::value == bg::open
            || bg::closure<Geometry2>::value == bg::open;

        std::ostringstream filename;
        filename << "rescale_policy_intersection_"
            << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << string_from_type<CalculationType>::name()
            << (ccw ? "_ccw" : "")
            << (open ? "_open" : "")
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<point_type> mapper(svg, 500, 500);

        mapper.add(geometry1);
        mapper.add(geometry2);

        mapper.map(geometry1, "fill-opacity:0.5;fill:rgb(153,204,0);"
                    "stroke:rgb(153,204,0);stroke-width:3");
        mapper.map(geometry2, "fill-opacity:0.3;fill:rgb(51,51,153);"
                    "stroke:rgb(51,51,153);stroke-width:3");

        for (typename std::vector<OutputType>::const_iterator it = out.begin();
                it != out.end(); ++it)
        {
            mapper.map(*it, "fill-opacity:0.2;stroke-opacity:0.4;fill:rgb(255,0,0);"
                        "stroke:rgb(255,0,255);stroke-width:8");
        }
    }
#endif

    if (debug)
    {
        std::cout << "end case " << caseid << std::endl;
    }

    return length_or_area;
}

template
<
    typename RescalePolicy,
    typename OutputType,
    typename Geometry1,
    typename Geometry2
>
typename bg::default_area_result<Geometry1>::type test_one(std::string const& caseid,
        std::string const& wkt1, std::string const& wkt2,
        std::size_t expected_count = 0, int expected_point_count = 0,
        double expected_length_or_area = 0,
        double percentage = 0.02,
        bool debug = false)
{
    Geometry1 geometry1;
    bg::read_wkt(wkt1, geometry1);

    Geometry2 geometry2;
    bg::read_wkt(wkt2, geometry2);

    // Reverse if necessary
    bg::correct(geometry1);
    bg::correct(geometry2);

    RescalePolicy rescale_policy
            = bg::get_rescale_policy<RescalePolicy>(geometry1, geometry2);

    return test_intersection<OutputType, void>(caseid,
        geometry1, geometry2,
        rescale_policy,
        expected_count, expected_point_count,
        expected_length_or_area, percentage,
        debug);
}

static std::string simplex_normal[2] =
    {"POLYGON((0 1,2 5,5 3,0 1))",
    "POLYGON((3 0,0 3,4 5,3 0))"};

static std::string simplex_large[2] =
    {"POLYGON((0 1000,2000 5000,5000 3000,0 1000))",
    "POLYGON((3000 0,0 3000,4000 5000,3000 0))"};


template <bool Rescale, typename P>
void test_rescale()
{
    typedef bg::model::polygon<P> polygon;

    typedef typename boost::mpl::if_c
        <
            Rescale,
            typename bg::rescale_policy_type<P>::type ,
            bg::detail::no_rescale_policy
        >::type rescale_policy_type;

    std::string suffix = Rescale ? "rescale" : "no_rescale";

    typedef typename bg::coordinate_type<P>::type coordinate_type;
    if (! boost::is_integral<coordinate_type>::type::value)
    {
        test_one<rescale_policy_type, polygon, polygon, polygon>("simplex_normal_" + suffix,
            simplex_normal[0], simplex_normal[1],
            1, 7, 5.47363293);
    }
    test_one<rescale_policy_type, polygon, polygon, polygon>("simplex_large_" + suffix,
        simplex_large[0], simplex_large[1],
        1, 7, 5473632.93);
}

template <typename T>
void test_all()
{
    typedef bg::model::d2::point_xy<T> point_type;
    test_rescale<true, point_type>();
    test_rescale<false, point_type>();
}


int test_main(int, char* [])
{
    test_all<double>();
    test_all<long double>();
    test_all<int>();
    test_all<boost::long_long_type>();
    //    test_all<short int>(); // compiles but overflows

    return 0;
}

