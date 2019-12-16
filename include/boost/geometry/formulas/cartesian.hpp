// Boost.Geometry

// Copyright (c) 2019 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_CARTESIAN_HPP
#define BOOST_GEOMETRY_FORMULAS_CARTESIAN_HPP

#include <boost/concept_check.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/util/select_coordinate_type.hpp>

// Helper geometry (projected point on line)
#include <boost/geometry/geometries/point.hpp>

namespace boost { namespace geometry { namespace formula
{

template <
    typename CalculationType,
    bool EnableClosestPoint = false
>
class comparable_cartesian_point_segment_distance
{

public :

    struct result_type
    {
        result_type()
            : distance(0)
            , x1(0)
            , y1(0)
            , x2(0)
            , y2(0)
        {}

        CalculationType distance;
        CalculationType x1;
        CalculationType y1;
        CalculationType x2;
        CalculationType y2;
    };

    template <typename Point, typename PointOfSegment, typename Strategy>
    static inline result_type
    apply(Point const& p,
          PointOfSegment const& p1,
          PointOfSegment const& p2,
          Strategy const& comparable_distance_strategy)
    {
        result_type result;

        result.x1 = get<0>(p);
        result.y1 = get<1>(p);

        // A projected point of points in Integer coordinates must be able to be
        // represented in FP.
        typedef model::point
            <
                CalculationType,
                dimension<PointOfSegment>::value,
                typename coordinate_system<PointOfSegment>::type
            > fp_point_type;

        // For convenience
        typedef fp_point_type fp_vector_type;

        /*
            Algorithm [p: (px,py), p1: (x1,y1), p2: (x2,y2)]
            VECTOR v(x2 - x1, y2 - y1)
            VECTOR w(px - x1, py - y1)
            c1 = w . v
            c2 = v . v
            b = c1 / c2
            RETURN POINT(x1 + b * vx, y1 + b * vy)
        */

        // v is multiplied below with a (possibly) FP-value, so should be in FP
        // For consistency we define w also in FP
        fp_vector_type v, w, projected;

        geometry::convert(p2, v);
        geometry::convert(p, w);
        geometry::convert(p1, projected);
        subtract_point(v, projected);
        subtract_point(w, projected);

        CalculationType const zero = CalculationType();
        CalculationType const c1 = dot_product(w, v);
        if (c1 <= zero)
        {
            result.distance = comparable_distance_strategy.apply(p, p1);
            result.x2 = get<0>(p1);
            result.y2 = get<1>(p1);
            return result;
        }
        CalculationType const c2 = dot_product(v, v);
        if (c2 <= c1)
        {
            result.distance = comparable_distance_strategy.apply(p, p2);
            result.x2 = get<0>(p2);
            result.y2 = get<1>(p2);
            return result;
        }

        // See above, c1 > 0 AND c2 > c1 so: c2 != 0
        CalculationType const b = c1 / c2;

        multiply_value(v, b);
        add_point(projected, v);

        result.distance = comparable_distance_strategy.apply(p, projected);
        result.x2 = get<0>(p2);
        result.y2 = get<1>(p2);
        return result;
    }
};

} } } // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_CARTESIAN_HPP
