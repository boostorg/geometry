// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_ASYMMETRIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_ASYMMETRIC_HPP

#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{


template<typename CoordinateType>
class distance_asymmetric
{
public :
    distance_asymmetric(CoordinateType const& left,
                CoordinateType const& right)
        : m_left(left)
        , m_right(right)
    {}

    template <typename Point>
    inline CoordinateType apply(Point const& , Point const& ,
                buffer_side_selector side)  const
    {
        CoordinateType result = side == buffer_side_left ? m_left : m_right;
        return negative() ? math::abs(result) : result;
    }

    inline int factor() const
    {
        return m_left < 0 ? -1 : 1;
    }

    inline bool negative() const
    {
        return m_left < 0 && m_right < 0;
    }

    inline CoordinateType simplify_distance() const
    {
        CoordinateType const left = geometry::math::abs(m_left);
        CoordinateType const right = geometry::math::abs(m_right);
        return (left < right ? left : right) / 1000.0;
    }


private :
    CoordinateType m_left;
    CoordinateType m_right;
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_ASYMMETRIC_HPP
