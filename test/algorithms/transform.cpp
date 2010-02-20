// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <sstream>


#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/util/write_dsv.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

#include <test_common/test_point.hpp>

template <typename P1, typename P2>
void test_transform_point(double value)
{
    P1 p1;
    boost::geometry::set<0>(p1, 1);
    boost::geometry::set<1>(p1, 2);
    P2 p2;
    boost::geometry::transform(p1, p2);

    BOOST_CHECK_CLOSE(value * boost::geometry::get<0>(p1), double(boost::geometry::get<0>(p2)), 0.001);
    BOOST_CHECK_CLOSE(value * boost::geometry::get<1>(p1), double(boost::geometry::get<1>(p2)), 0.001);
}

template <typename P1, typename P2>
void test_transform_linestring()
{
    boost::geometry::linestring<P1> line1;
    line1.push_back(boost::geometry::make<P1>(1, 1));
    line1.push_back(boost::geometry::make<P1>(2, 2));
    boost::geometry::linestring<P2> line2;
    boost::geometry::transform(line1, line2);
    BOOST_CHECK_EQUAL(line1.size(), line2.size());

    std::ostringstream out1, out2;
    out1 << boost::geometry::wkt(line1);
    out2 << boost::geometry::wkt(line2);
    BOOST_CHECK_EQUAL(out1.str(), out1.str());
}


template <typename P1, typename P2>
void test_all(double value = 1.0)
{
    test_transform_point<P1, P2>(value);
    test_transform_linestring<P1, P2>();
}

template <typename T, typename DegreeOrRadian>
void test_transformations(double phi, double theta, double r)
{
    using namespace boost::geometry;

    typedef point<T, 3, cs::cartesian> cartesian_type;
    cartesian_type p;

    // 1: using spherical coordinates
    {
        typedef point<T, 3, cs::spherical<DegreeOrRadian> >  spherical_type;
        spherical_type sph1;
        assign(sph1, phi, theta, r);
        transform(sph1, p);

        spherical_type sph2;
        transform(p, sph2);

        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(sph1)), double(boost::geometry::get<0>(sph2)), 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(sph1)), double(boost::geometry::get<1>(sph2)), 0.001);

        //std::cout << dsv(p) << std::endl;
        //std::cout << dsv(sph2) << std::endl;
    }

    // 2: using spherical coordinates on unit sphere
    {
        typedef point<T, 2, cs::spherical<DegreeOrRadian> >  spherical_type;
        spherical_type sph1, sph2;
        assign(sph1, phi, theta);
        transform(sph1, p);
        transform(p, sph2);

        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(sph1)), double(boost::geometry::get<0>(sph2)), 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(sph1)), double(boost::geometry::get<1>(sph2)), 0.001);

        //std::cout << dsv(sph1) << " " << dsv(p) << " " << dsv(sph2) << std::endl;
    }
}

int test_main(int, char* [])
{
    using namespace boost::geometry;
    using namespace boost::geometry::cs;
    using namespace boost::geometry::math;


    typedef boost::geometry::point_xy<double > P;
    test_all<P, P>();
    test_all<boost::geometry::point_xy<int>, boost::geometry::point_xy<float> >();

    test_all<boost::geometry::point<double, 2, spherical<degree> >,
        boost::geometry::point<double, 2, spherical<radian> > >(d2r);
    test_all<boost::geometry::point<double, 2, spherical<radian> >,
        boost::geometry::point<double, 2, spherical<degree> > >(r2d);

    test_all<boost::geometry::point<int, 2, spherical<degree> >,
        boost::geometry::point<float, 2, spherical<radian> > >(d2r);

    test_transformations<float, degree>(4, 52, 1);
    test_transformations<double, degree>(4, 52, 1);

    test_transformations<float, radian>(3 * d2r, 51 * d2r, 1);
    test_transformations<double, radian>(3 * d2r, 51 * d2r, 1);


    return 0;
}
