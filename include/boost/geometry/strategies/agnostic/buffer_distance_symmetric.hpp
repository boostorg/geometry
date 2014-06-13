// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_SYMMETRIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_SYMMETRIC_HPP

#include <boost/geometry/strategies/buffer.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{


template<typename CoordinateType>
class distance_symmetric
{
public :
    inline distance_symmetric(CoordinateType const& distance)
        : m_distance(distance)
    {}

    template <typename Point>
    inline CoordinateType apply(Point const& , Point const& ,
                buffer_side_selector side)  const
    {
        return m_distance;
    }

    inline int factor() const
    {
        return 1;
    }

    inline bool negative() const
    {
        return m_distance < 0;
    }

private :
    CoordinateType m_distance;
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_SYMMETRIC_HPP
