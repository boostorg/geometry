// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_PARAMETERS_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_PARAMETERS_HPP


#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/formulas/vincenty_inverse.hpp>

#include <boost/mpl/integral_c.hpp>


namespace boost { namespace geometry { namespace strategy
{

template
<
    template <typename, bool, bool, bool, bool, bool> class Formula
>
struct default_order
{};

template<>
struct default_order<formula::andoyer_inverse>
    : boost::mpl::integral_c<unsigned int, 1>
{};

template<>
struct default_order<formula::thomas_inverse>
    : boost::mpl::integral_c<unsigned int, 2>
{};

template<>
struct default_order<formula::vincenty_inverse>
    : boost::mpl::integral_c<unsigned int, 4>
{};

}}} // namespace boost::geometry::strategy


#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_PARAMETERS_HPP
