// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2018-2021.
// Modifications copyright (c) 2018-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_TRANSLATION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_TRANSLATION_HPP

#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/extensions/algebra/geometries/concepts/vector_concept.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
//#include <boost/geometry/geometries/concepts/point_concept.hpp>

namespace boost { namespace geometry
{

// TODO - implement point_operation2 taking two arguments

template <typename Point1, typename Point2, typename Vector>
inline void translation(Point1 const& p1, Point2 const& p2, Vector & v)
{
    concepts::check_concepts_and_equal_dimensions<Point1 const, Point2 const>();
    // TODO - replace the following by check_equal_dimensions
    concepts::check_concepts_and_equal_dimensions<Point1 const, Vector>();

    detail::for_each_dimension<Point1>([&](auto index)
    {
        set<index>(v, get<index>(p2) - get<index>(p1));
    });
}

template <typename Vector, typename Point1, typename Point2>
inline Vector return_translation(Point1 const& p1, Point2 const& p2)
{
    Vector v;
    translation(p1, p2, v);
    return v;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ASSIGN_HPP
