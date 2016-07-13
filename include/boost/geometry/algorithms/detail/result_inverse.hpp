// Boost.Geometry

// Copyright (c) 2015-2016 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESULT_INVERSE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESULT_INVERSE_HPP


#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/algorithms/detail/flattening.hpp>


namespace boost { namespace geometry { namespace formula
{

template <typename T>
struct result_inverse
{
    result_inverse()
        : distance(0)
        , azimuth(0)
        , reverse_azimuth(0)
        , reduced_length(0)
        , geodesic_scale(1)
    {}

    T distance;
    T azimuth;
    T reverse_azimuth;
    T reduced_length;
    T geodesic_scale;
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESULT_INVERSE_HPP
