// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/geometry/strategies/transform/inverse_transformer.hpp>
#include <boost/geometry/strategies/transform/map_transformer.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>


#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/transform.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>




template <typename P, typename T>
void check_inverse(P const& p, T const& trans)
{
    boost::geometry::strategy::transform::inverse_transformer<P, P> inverse(trans);

    P i;
    boost::geometry::transform(p, i, inverse);

    BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(i)), 1.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(i)), 1.0, 0.001);
}

template <typename P>
void test_all()
{
    P p;
    boost::geometry::assign(p, 1, 1);

    {
        boost::geometry::strategy::transform::translate_transformer<P, P> trans(1, 1);
        P tp;
        boost::geometry::transform(p, tp, trans);

        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(tp)), 2.0, 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(tp)), 2.0, 0.001);

        check_inverse(tp, trans);
    }

    {
        boost::geometry::strategy::transform::scale_transformer<P, P> trans(10, 10);
        P tp;
        boost::geometry::transform(p, tp, trans);

        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(tp)), 10.0, 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(tp)), 10.0, 0.001);

        check_inverse(tp, trans);
    }

    {
        boost::geometry::strategy::transform::rotate_transformer<P, P, boost::geometry::degree> trans(90.0);
        P tp;
        boost::geometry::transform(p, tp, trans);

        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(tp)), 1.0, 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(tp)), -1.0, 0.001);
        check_inverse(tp, trans);
    }

    {
        // Map from 0,0,2,2 to 0,0,500,500
        boost::geometry::strategy::transform::map_transformer<P, P, false> trans
            (
                0.0, 0.0, 2.0, 2.0, 500, 500
            );
        P tp;
        boost::geometry::transform(p, tp, trans);

        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(tp)), 250.0, 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(tp)), 250.0, 0.001);

        check_inverse(tp, trans);
    }
}

int test_main(int, char* [])
{
    //test_all<int[2]>();
    test_all<float[2]>();
    test_all<double[2]>();

    test_all<boost::tuple<float, float> >();

    //test_all<point<int, 2, cs::cartesian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
