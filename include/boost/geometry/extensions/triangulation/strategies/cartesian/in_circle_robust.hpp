// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_IN_CIRCLE_ROBUST_HPP
#define BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_IN_CIRCLE_ROBUST_HPP

#include<boost/geometry/extensions/triangulation/strategies/cartesian/detail/precise_math.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace in_circle
{

template <typename CalculationType = double, int robustness = 2>
class in_circle_robust
{
public:
    template <typename P1, typename P2, typename P3, typename P>
    static inline int apply(P1 const& p1, P2 const& p2, P3 const& p3, P const& p)
    {
        std::array<CalculationType, 2> pa {
            { boost::geometry::get<0>(p1), boost::geometry::get<1>(p1) }};
        std::array<CalculationType, 2> pb {
            { boost::geometry::get<0>(p2), boost::geometry::get<1>(p2) }};
        std::array<CalculationType, 2> pc {
            { boost::geometry::get<0>(p3), boost::geometry::get<1>(p3) }};
        std::array<CalculationType, 2> pd {
            { boost::geometry::get<0>(p), boost::geometry::get<1>(p) }};
        CalculationType det =
            boost::geometry::detail::precise_math::incircle
                <
                    CalculationType,
                    robustness
                >(pa, pb, pc, pd);
        return det > 0 ? 1
                       : det < 0 ? -1 : 0;
    }

};

} // namespace in_circle

} // namespace strategy

}} // namespace boost::geometry::strategy

#endif // BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_IN_CIRCLE_ROBUST_HPP
