// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_SYMMETRIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_SYMMETRIC_HPP

#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{


/*!
\brief Let the buffer algorithm create buffers with same distances
\ingroup strategies
\details This strategy can be used as DistanceStrategy for the buffer algorithm.
    It can be applied for all geometries. It uses one distance for left and
    for right.
    The distance can also be negative: it creates a deflated polygon.

\qbk{
[heading Example]
[buffer_distance_symmetric]
[heading Output]
[$img/strategies/buffer_distance_symmetric.png]
[heading See also]
\* [link geometry.reference.algorithms.buffer.buffer_7_with_strategies buffer (with strategies)]
}
 */
template<typename CoordinateType>
class distance_symmetric
{
public :
    //! Constructs the strategy, a distance must be specified
    explicit inline distance_symmetric(CoordinateType const& distance)
        : m_distance(distance)
    {}

    //! Returns the distance-value
    template <typename Point>
    inline CoordinateType apply(Point const& , Point const& ,
                buffer_side_selector )  const
    {
        return m_distance;
    }

    //! Returns 1 (used internally)
    inline int factor() const
    {
        return 1;
    }

    //! Returns true if distance is negative
    inline bool negative() const
    {
        return m_distance < 0;
    }

    //! Returns the distance at which the input is simplified before the buffer process
    inline CoordinateType simplify_distance() const
    {
        return geometry::math::abs(m_distance) / 1000.0;
    }


private :
    CoordinateType m_distance;
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_AGNOSTIC_BUFFER_DISTANCE_SYMMETRIC_HPP
