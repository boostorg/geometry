#ifndef BOOST_GEOMETRY_TEST_MYSQL_FROM_WKT_HPP
#define BOOST_GEOMETRY_TEST_MYSQL_FROM_WKT_HPP

#include <boost/geometry/io/wkt/read.hpp>

template <typename Geometry>
Geometry from_wkt(std::string const& wkt)
{
    Geometry res;
    boost::geometry::read_wkt(wkt, res);
    return res;
}

#endif // BOOST_GEOMETRY_TEST_MYSQL_FROM_WKT_HPP
