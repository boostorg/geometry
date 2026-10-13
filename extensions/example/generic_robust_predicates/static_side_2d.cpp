// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#define BOOST_GEOMETRY_NO_BOOST_TEST

#include <iostream>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>

#include <boost/geometry/extensions/triangulation/strategies/cartesian/side_robust.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expressions.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_a.hpp>


namespace bg = boost::geometry;
using point = bg::model::point<double, 2, bg::cs::cartesian>;

template <typename CalculationType>
struct side_robust_with_static_filter
{
private:
    using ct = CalculationType;
    using expression = bg::detail::generic_robust_predicates::orient2d;
    using filter = bg::detail::generic_robust_predicates::stage_a_static
            <
                expression,
                ct
            >;
    filter m_filter;
public:
    side_robust_with_static_filter(ct x_max, ct y_max, ct x_min, ct y_min)
        : m_filter(x_max, y_max, x_max, y_max, x_max, y_max,
                   x_min, y_min, x_min, y_min, x_min, y_min) {};

    template
    <
        typename P1,
        typename P2,
        typename P  
    >
    inline int apply(P1 const& p1, P2 const& p2, P const& p) const
    {
        int sign = m_filter.apply(bg::get<0>(p1),
                                  bg::get<1>(p1),
                                  bg::get<0>(p2),
                                  bg::get<1>(p2),
                                  bg::get<0>(p),
                                  bg::get<1>(p));
        if (sign != bg::detail::generic_robust_predicates::sign_uncertain)
        {
            return sign;
        }
        else
        {
            //fallback if filter fails.
            return bg::strategy::side::side_robust<double>::apply(p1, p2, p);
        }
    }
};

int main()
{
    point p1(0.0, 0.0);
    point p2(1.0, 1.0);
    point p (0.0, 1.0);
    side_robust_with_static_filter<double> static_strategy(2.0, 2.0, 1.0, 1.0);
    std::cout << "Side value: " << static_strategy.apply(p1, p2, p) << "\n"; 
    return 0;
}
