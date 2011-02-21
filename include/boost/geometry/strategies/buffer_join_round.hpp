// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_BUFFER_JOIN_ROUND_HPP
#define BOOST_GEOMETRY_STRATEGIES_BUFFER_JOIN_ROUND_HPP



#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/buffer_side.hpp>


#define BOOST_GEOMETRY_BUFFER_NO_HELPER_POINTS


namespace boost { namespace geometry
{




namespace strategy { namespace buffer
{




template<typename PointOut>
struct join_round2
{
    typedef PointOut vector_type;

    template <typename Vector, typename Point1, typename Point2>
    static inline Vector create_vector(Point1 const& p1, Point2 const& p2)
    {
        Vector v;
        geometry::convert(p1, v);
        subtract_point(v, p2);
        return v;
    }

    inline join_round2(int max_level = 4)
        : m_max_level(max_level)
    {}

    typedef typename coordinate_type<PointOut>::type coordinate_type;
    int m_max_level;


    template <typename OutputIterator, typename Point, typename PointP, typename Point1, typename Point2>
    inline void mid_points(Point const& vertex, PointP const& perpendicular,
                Point1 const& p1, Point2 const& p2,
                coordinate_type const& buffer_distance,
                coordinate_type const& max_distance,
                OutputIterator out,
                int level = 1) const
    {
        // Generate 'vectors'
        coordinate_type vp1_x = get<0>(p1) - get<0>(vertex);
        coordinate_type vp1_y = get<1>(p1) - get<1>(vertex);

        coordinate_type vp2_x = (get<0>(p2) - get<0>(vertex));
        coordinate_type vp2_y = (get<1>(p2) - get<1>(vertex));

        // Average them to generate vector in between
        coordinate_type two = 2;
        coordinate_type v_x = (vp1_x + vp2_x) / two;
        coordinate_type v_y = (vp1_y + vp2_y) / two;

        coordinate_type between_length = sqrt(v_x * v_x + v_y * v_y);

        coordinate_type const positive_buffer_distance = geometry::math::abs(buffer_distance);
        coordinate_type prop = positive_buffer_distance / between_length;

        PointOut mid_point;
        set<0>(mid_point, get<0>(vertex) + v_x * prop);
        set<1>(mid_point, get<1>(vertex) + v_y * prop);

        if (buffer_distance > max_distance)
        {
            // Calculate point projected on original perpendicular segment,
            // using vector maths
            vector_type v = create_vector<vector_type>(perpendicular, vertex);
            vector_type w = create_vector<vector_type>(mid_point, vertex);

            coordinate_type c1 = dot_product(w, v);
            if (c1 > 0)
            {
                coordinate_type c2 = dot_product(v, v);
                if (c2 > c1)
                {
                    coordinate_type b = c1 / c2;

                    PointOut projected_point;

                    multiply_value(v, b);
                    geometry::convert(vertex, projected_point);
                    add_point(projected_point, v);

                    coordinate_type projected_distance = geometry::distance(projected_point, mid_point);

                    if (projected_distance > max_distance)
                    {
                        // Do not generate from here on.
                        return;
                    }
                }
            }
        }

        if (level < m_max_level)
        {
            mid_points(vertex, perpendicular, p1, mid_point, positive_buffer_distance, max_distance, out, level + 1);
        }
        *out++ = mid_point;
        if (level < m_max_level)
        {
            mid_points(vertex, perpendicular, mid_point, p2, positive_buffer_distance, max_distance, out, level + 1);
        }
    }


    template <typename OutputIterator, typename Point, typename Point2>
    inline OutputIterator apply(Point const& vertex,
                Point2 const& perpendicular,
                Point2 const& p1, Point2 const& p2,
                coordinate_type const& buffer_distance,
                coordinate_type const& max_distance,
                OutputIterator out) const
    {
        mid_points(vertex, perpendicular, p1, p2, buffer_distance, max_distance, out);
        return out;
    }
};


template<typename PointOut>
struct join_none
{
    template <typename OutputIterator, typename Point, typename Point2,
        typename DistanceType>
    inline OutputIterator apply(Point const& ,
                Point2 const& ,
                Point2 const& , Point2 const& ,
                DistanceType const& ,
                DistanceType const& ,
                OutputIterator out) const
    {
        return out;
    }
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_JOIN_ROUND_HPP
