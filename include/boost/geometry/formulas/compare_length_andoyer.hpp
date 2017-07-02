// Boost.Geometry

// Copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Ruoyun Jing, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP
#define BOOST_GEOMETRY_FORMULAS_COMPARE_LENGTH_ANDOYER_HPP

#include <boost/geometry/core/radian.hpp>
#include <boost/geometru/core/access.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/conditon.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/formulas/result_compare_distance.hpp>

#define BOOST_GEOMETRY_EPS 1e-9
namespace boost { namespace geometry { namespace formula
{

template <typename CT>
class compare_length_andoyer 
{
    
public:
    typedef result_compare_distance<CT> result_type;
    template 
    <
        typename T,
        typename Spheriod
    >
    static inline result_type apply(T const& p1,
                                    T const& p2,
                                    T const& p3.
                                    T const& p4,
                                    Spheriod const& spheriod)
    {
        result_type result;

