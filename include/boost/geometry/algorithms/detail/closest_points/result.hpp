// Boost.Geometry

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_RESULT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_RESULT_HPP

//#include <boost/geometry/formulas/point_segment_distance.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace closest_points
{

template <typename CT>
class result
{
public :

    result()
        : distance(0)
        , lon1(0)
        , lat1(0)
        , lon2(0)
        , lat2(0)
    {}

    result(CT init)
        : distance(init)
        , lon1(0)
        , lat1(0)
        , lon2(0)
        , lat2(0)
    {}

    CT distance;
    CT lon1;
    CT lat1;
    CT lon2;
    CT lat2;

    template <typename T>
    void operator=(T const& res)
    {
        distance = res.distance;
        lon1 = res.lon1;
        lat1 = res.lat1;
        lon2 = res.lon2;
        lat2 = res.lat2;
    }

    bool operator<(const result& rhs) const
    {
        return distance < rhs.distance;
    }

    bool operator==(const result& rhs) const
    {
        return distance == rhs.distance;
    }

    void swap_points()
    {
        std::swap(lon1, lon2);
        std::swap(lat1, lat2);
    }

    void mirror()
    {
        lat1 *= -1;
        lat2 *= -1;
    }

    template <typename Point>
    void set_unique_point(Point const& point)
    {
        distance = 0;
        lon1 = get_as_radian<0>(point);
        lat1 = get_as_radian<1>(point);
        lon2 = lon1;
        lat2 = lat1;
    }
};

}}

#endif // DOXYGEN_NO_DETAIL

}} //namespace boost::geometry::detail::closest_points

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_RESULT_HPP
