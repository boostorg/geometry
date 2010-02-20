// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cstddef>
#include <string>


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/convex_hull.hpp>


#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/util/as_range.hpp>
#include <boost/geometry/extensions/gis/io/wkt/stream_wkt.hpp>


template <typename Geometry>
void test_geometry(std::string const& wkt,
                      std::size_t size_original, std::size_t size_hull,
                      double expected_area)
{

    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);

    typedef typename boost::geometry::point_type<Geometry>::type P;
    typename boost::geometry::strategy_side
        <
            typename boost::geometry::cs_tag<P>::type
        >::type side;

    typedef typename boost::geometry::range_type<Geometry>::type range_type;
    typedef typename boost::range_const_iterator<range_type>::type iterator;

    range_type const& range = boost::geometry::as_range<range_type>(geometry);

    iterator it1 = boost::begin(range);
    iterator it3 = it1++;
    iterator it2 = it1++;

    for (;
        it2 != boost::end(range);
        ++it1, ++it2, ++it3)
    {
       // Last/closing point
       if (it1 == boost::end(range))
       {
           it1 = boost::begin(range) + 1;
       }
       int s = side.apply(*it1, *it2, *it3);

       if (s != 1)
       {
          std::cout << "NOT CONVEX!";
       }
       if (s == 0)
       {
          std::cout << " COLLINEAR!";
       }

       std::cout
           << " " << (*it3)
           << " " << boost::geometry::wkt(*it2)
           << " " << boost::geometry::wkt(*it1)
           << " " << s
           << std::endl;
    }

    std::cout << boost::geometry::area(geometry) << " " << boost::geometry::wkt(geometry) << std::endl;
}

template <typename P>
void test_all()
{
    // rectangular, with concavity
    test_geometry<boost::geometry::polygon<P> >(
        "polygon((1 1, 1 4, 3 4, 3 3, 4 3, 4 4, 5 4, 5 1, 1 1))",
                9, 5, 12.0);


   // concavity at start/closing point
   test_geometry<boost::geometry::polygon<P> >(
        "polygon((1 1,0 2,3 3,1 0,1 1))", 9, 5, 12.0);

    // from sample polygon, with concavity
    test_geometry<boost::geometry::polygon<P> >(
        "polygon((2.0 1.3, 2.4 1.7, 2.8 1.8, 3.4 1.2, 3.7 1.6,3.4 2.0, 4.1 3.0"
        ", 5.3 2.6, 5.4 1.2, 4.9 0.8, 2.9 0.7,2.0 1.3))",
                12, 8, 5.245);
}

int test_main(int, char* [])
{
    //test_all<boost::geometry::point_xy<int> >();
    //test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

#if defined(HAVE_CLN)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif
#if defined(HAVE_GMP)
    // GMP compiles but gives errors in results
    // TODO: find out what's going wrong here
    // test_all<boost::geometry::point_xy<boost::numeric_adaptor::gmp_value_type> >();
#endif

    return 0;
}
