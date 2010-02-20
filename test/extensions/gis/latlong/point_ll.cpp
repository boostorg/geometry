// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/extensions/gis/io/wkt/stream_wkt.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/latlong/point_ll.hpp>


#include <test_common/test_point.hpp>

template <typename P>
void test_all()
{
    typedef typename boost::geometry::coordinate_type<P>::type type;

    P p1(boost::geometry::latitude<type>(boost::geometry::dms<boost::geometry::south, type>(12, 2, 36)),
         boost::geometry::longitude<type>(boost::geometry::dms<boost::geometry::west, type>(77, 1, 42)));

    // Check decimal/degree conversion
    BOOST_CHECK_CLOSE(boost::geometry::get<0>(p1), type(-77.0283), 0.001);
    BOOST_CHECK_CLOSE(boost::geometry::get<1>(p1), type(-12.0433), 0.001);

    // Check degree/radian conversion
    boost::geometry::point_ll<type, boost::geometry::cs::geographic<boost::geometry::radian> > p2;
    boost::geometry::transform(p1, p2);

    BOOST_CHECK_CLOSE(boost::geometry::get<0>(p2), type(-1.3444), 0.001);
    BOOST_CHECK_CLOSE(boost::geometry::get<1>(p2), type(-0.210196), 0.001);

    // Check degree/radian conversion back
    P p3;
    boost::geometry::transform(p2, p3);
    BOOST_CHECK_CLOSE(boost::geometry::get<0>(p3), type(-77.0283), 0.001);
    BOOST_CHECK_CLOSE(boost::geometry::get<1>(p3), type(-12.0433), 0.001);


    // Check decimal/degree conversion back
    int d;
    int m;
    double s;
    bool positive;
    char cardinal;

    boost::geometry::dms<boost::geometry::cd_lat, type> d1(boost::geometry::get<0>(p3));
    d1.get_dms(d, m, s, positive, cardinal);

    BOOST_CHECK(d == 77);
    BOOST_CHECK(m == 1);
    BOOST_CHECK_CLOSE(s, double(42), 0.1);
    BOOST_CHECK(positive == false);
    BOOST_CHECK(cardinal == 'S');

    // Check dd conversion as string, back. We cannot do that always because of the precision.
    // Only double gives correct results
    //std::string st = d1.get_dms();
    //std::cout << st << std::endl;
    //BOOST_CHECK(st == "77 1'42\" S");
}

int test_main(int, char* [])
{
    test_all<boost::geometry::point_ll<float> >();
    test_all<boost::geometry::point_ll<double> >();

    return 0;
}
