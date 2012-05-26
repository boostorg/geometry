// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_STRATEGIES_BUFFER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_STRATEGIES_BUFFER_HPP


// Buffer strategies

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/geometry/extensions/strategies/buffer_side.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

/*

   A Buffer-join strategy gets 4 input points.
   On the two consecutive segments s1 and s2 (joining at vertex v):

   The lines from parallel at s1, s2 (at buffer-distance) end/start
   in two points perpendicular to the segments: p1 and p2.
   These parallel lines interesct in point ip

             (s2)
              |
              |
              ^
              |
        (p2)  |(v)
        *     +----<--- (s1)

        x(ip) *(p1)


    So, in clockwise order:
        v : vertex point
        p1: perpendicular on left side of segment1<1> (perp1)
        ip: intersection point
        p2: perpendicular on left side of segment2<0> (perp2)
*/



// TODO: merge join_miter with join_round, lot of duplicate code

template
<
    typename PointIn,
    typename PointOut
>
struct join_miter
{
    typedef typename strategy::side::services::default_strategy<typename cs_tag<PointIn>::type>::type side;
    typedef typename coordinate_type<PointIn>::type coordinate_type;


    template <typename RangeOut>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                RangeOut& range_out) const
    {
        coordinate_type zero = 0;
        int signum = buffer_distance > zero ? 1
                   : buffer_distance < zero ? -1
                   : 0;

        if (side::apply(perp1, ip, perp2) == signum)
        {
        }
        else
        {
            PointIn p = ip;

            // Normalize it and give it X*dist.
            coordinate_type dx = get<0>(ip) - get<0>(vertex);
            coordinate_type dy = get<1>(ip) - get<1>(vertex);

            coordinate_type length = sqrt(dx * dx + dy * dy);

            // TODO: make max-mitre-limit flexible
            coordinate_type ten = 10.0;
            coordinate_type zero_seven = 0.7;

            coordinate_type max = ten * geometry::math::abs(buffer_distance);

            if (length > max)
            {

                coordinate_type prop = zero_seven * buffer_distance;
                prop /= length;
                set<0>(p, get<0>(vertex) + dx * prop);
                set<1>(p, get<1>(vertex) + dy * prop);

#ifdef BOOST_GEOMETRY_DEBUG_BUFFER
                std::cout << length << std::endl;
#endif
            }

            range_out.push_back(perp1);
            range_out.push_back(p);
            range_out.push_back(perp2);
        }

    }
};


template
<
    typename PointIn,
    typename PointOut
>
struct join_bevel
{
    typedef typename coordinate_type<PointIn>::type coordinate_type;

    template <typename RangeOut>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                RangeOut& range_out) const
    {
    }
};


template
<
    typename PointIn,
    typename PointOut
>
struct join_round
{
    inline join_round(int max_level = 4)
        : m_max_level(max_level)
    {}

    typedef typename strategy::side::services::default_strategy<typename cs_tag<PointIn>::type>::type side;
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

#ifdef BOOST_GEOMETRY_BUFFER_USE_MIDPOINTS
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

        coordinate_type length2 = sqrt(v_x * v_x + v_y * v_y);

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
#endif

    template <typename RangeOut>
    inline void generate_points(PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                promoted_type const& buffer_distance,
                RangeOut& range_out) const
    {
        promoted_type dx1 = get<0>(perp1) - get<0>(vertex);
        promoted_type dy1 = get<1>(perp1) - get<1>(vertex);
        promoted_type dx2 = get<0>(perp2) - get<0>(vertex);
        promoted_type dy2 = get<1>(perp2) - get<1>(vertex);

        dx1 /= buffer_distance;
        dy1 /= buffer_distance;
        dx2 /= buffer_distance;
        dy2 /= buffer_distance;

        promoted_type angle_diff = acos(dx1 * dx2 + dy1 * dy2);

        // Default might be 100 steps for a full circle (2 pi)
        promoted_type const steps_per_circle = 100.0;
		promoted_type two = 2.0;
        int n = boost::numeric_cast<int>(steps_per_circle * angle_diff 
                    / (two * geometry::math::pi<promoted_type>()));

		if (n > 1000)
		{
			std::cout << dx1 << ", " << dy1 << " .. " << dx2 << ", " << dy2 << std::endl;
			std::cout << angle_diff << " -> " << n << std::endl;
			n = 1000;
		}
		else if (n <= 1)
		{
			return;
		}

        promoted_type const angle1 = atan2(dy1, dx1);
        promoted_type diff = angle_diff / promoted_type(n);
        promoted_type a = angle1 - diff;

        for (int i = 0; i < n - 1; i++, a -= diff)
        {
            PointIn p;
            set<0>(p, get<0>(vertex) + buffer_distance * cos(a));
            set<1>(p, get<1>(vertex) + buffer_distance * sin(a));
            range_out.push_back(p);
        }
    }

    template <typename RangeOut>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                RangeOut& range_out) const
    {
		if (geometry::equals(perp1, perp2))
		{
			//std::cout << "Corner for equal points " << geometry::wkt(ip) << " " << geometry::wkt(perp1) << std::endl;
			return;
		}

        coordinate_type zero = 0;
        int signum = buffer_distance > zero ? 1
                   : buffer_distance < zero ? -1
                   : 0;

        if (side::apply(perp1, ip, perp2) == signum)
        {
        }
        else
        {
            // Generate 'vectors'
            coordinate_type vix = (get<0>(ip) - get<0>(vertex));
            coordinate_type viy = (get<1>(ip) - get<1>(vertex));

            coordinate_type length_i = sqrt(vix * vix + viy * viy);


            coordinate_type const bd = geometry::math::abs(buffer_distance);
            coordinate_type prop = bd / length_i;

            PointIn bp;
            set<0>(bp, get<0>(vertex) + vix * prop);
            set<1>(bp, get<1>(vertex) + viy * prop);

            range_out.push_back(perp1);

#ifdef BOOST_GEOMETRY_BUFFER_USE_MIDPOINTS
            if (m_max_level <= 1)
            {
                if (m_max_level == 1)
                {
                    range_out.push_back(bp);
                }
            }
            else
            {
                mid_points(vertex, perp1, bp, bd, range_out);
                range_out.push_back(bp);
                mid_points(vertex, bp, perp2, bd, range_out);
            }
#else
            generate_points(vertex, perp1, perp2, bd, range_out);
#endif

            range_out.push_back(perp2);
        }
    }
};



template
<
    typename CoordinateType
>
class distance_assymetric
{
public :
    distance_assymetric(CoordinateType const& left,
                CoordinateType const& right)
        : m_left(left)
        , m_right(right)
    {}

    template <typename Point>
    inline CoordinateType apply(Point const& , Point const& ,
                buffer_side_selector side)  const
    {
        return side == buffer_side_left ? m_left : m_right;
    }
    
    inline int factor() const
    {
        return m_left < 0 ? -1 : 1;
    }

private :
    CoordinateType m_left;
    CoordinateType m_right;
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_STRATEGIES_BUFFER_HPP
