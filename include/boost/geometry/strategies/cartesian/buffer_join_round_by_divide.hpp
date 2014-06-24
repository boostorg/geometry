// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_ROUND_BY_DIVIDE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_ROUND_BY_DIVIDE_HPP

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_BUFFER_WARN
#include <boost/geometry/io/wkt/wkt.hpp>
#endif

namespace boost { namespace geometry
{


namespace strategy { namespace buffer
{


template
<
    typename PointIn,
    typename PointOut
>
class join_round_by_divide
{
private :
    geometry::equal_to<PointIn> equals;

public :

    inline join_round_by_divide(int max_level = 4)
        : m_max_level(max_level)
    {}

    typedef typename coordinate_type<PointOut>::type coordinate_type;

    typedef typename geometry::select_most_precise
        <
            typename geometry::select_most_precise
                <
                    typename geometry::coordinate_type<PointIn>::type,
                    typename geometry::coordinate_type<PointOut>::type
                >::type,
            double
        >::type promoted_type;


    int m_max_level;

    template <typename RangeOut>
    inline void mid_points(PointIn const& vertex,
                PointIn const& p1, PointIn const& p2,
                coordinate_type const& buffer_distance,
                RangeOut& range_out,
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

        coordinate_type length2 = geometry::math::sqrt(v_x * v_x + v_y * v_y);

        coordinate_type prop = buffer_distance / length2;

        PointIn mid_point;
        set<0>(mid_point, get<0>(vertex) + v_x * prop);
        set<1>(mid_point, get<1>(vertex) + v_y * prop);

        if (level < m_max_level)
        {
            mid_points(vertex, p1, mid_point, buffer_distance, range_out, level + 1);
        }
        range_out.push_back(mid_point);
        if (level < m_max_level)
        {
            mid_points(vertex, mid_point, p2, buffer_distance, range_out, level + 1);
        }
    }

    template <typename RangeOut>
    inline bool apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                RangeOut& range_out) const
    {
        if (equals(perp1, perp2))
        {
#ifdef BOOST_GEOMETRY_DEBUG_BUFFER_WARN
            std::cout << "Corner for equal points " << geometry::wkt(ip) << " " << geometry::wkt(perp1) << std::endl;
#endif
            return false;
        }

        // Generate 'vectors'
        coordinate_type vix = (get<0>(ip) - get<0>(vertex));
        coordinate_type viy = (get<1>(ip) - get<1>(vertex));

        coordinate_type length_i =
            geometry::math::sqrt(vix * vix + viy * viy);

        coordinate_type const bd = geometry::math::abs(buffer_distance);
        coordinate_type prop = bd / length_i;

        PointIn bp;
        set<0>(bp, get<0>(vertex) + vix * prop);
        set<1>(bp, get<1>(vertex) + viy * prop);

        range_out.push_back(perp1);

        if (m_max_level > 1)
        {
            mid_points(vertex, perp1, bp, bd, range_out);
            range_out.push_back(bp);
            mid_points(vertex, bp, perp2, bd, range_out);
        }
        else if (m_max_level == 1)
        {
            range_out.push_back(bp);
        }

        range_out.push_back(perp2);
        return true;
    }
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_JOIN_ROUND_BY_DIVIDE_HPP
