// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_BUFFER_HPP
#define BOOST_GEOMETRY_STRATEGIES_BUFFER_HPP


// Buffer strategies

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/strategies/side.hpp>



namespace boost { namespace geometry
{


namespace strategy { namespace buffer
{




/*

   A Buffer-join strategy gets 4 input points.
   On the two consecutive segments s1 and s2 (combined by vertex v):

   There are two points perpendicular to the segments (p1,p2),
   crossing each other in interesction point x.


              (s2)
              |
              |
              ^
              |
        (p2)  |(v)
        *     +----<--- (s1)

        x(i)  *(p1)


    So, in clockwise order:
        v : vertex point
        p1: perpendicular on left side of segment1<1> (perp1)
        i : intersection point (ip)
        p2: perpendicular on left side of segment2<0> (perp2)
*/


template
<
    typename PointIn,
    typename PointOut
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
struct join_miter
{
    typedef typename strategy_side<typename cs_tag<PointIn>::type>::type side;
    typedef typename coordinate_type<PointIn>::type coordinate_type;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    Mapper const& m_mapper;

    join_miter(Mapper const& mapper)
        : m_mapper(mapper)
    {}
#endif

    template <typename Ring>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                double buffer_distance,
                Ring& buffered) const
    {
        int signum = buffer_distance > 0 ? 1 : buffer_distance < 0 ? -1 : 0;

        // If it is concave (corner to left), add helper-line
        if (side::apply(perp1, ip, perp2) == signum)
        {
            buffered.push_back(perp1);
            buffered.push_back(perp2);
            // Note, because perp1 crosses perp2 at IP, it is not necessary to
            // include also IP
        }
        else
        {
            PointIn p = ip;

            // Normalize it and give it X*dist.
            coordinate_type dx = get<0>(ip) - get<0>(vertex);
            coordinate_type dy = get<1>(ip) - get<1>(vertex);

            coordinate_type length = sqrt(dx * dx + dy * dy);
            coordinate_type max = 10 * abs(buffer_distance);

            if (length > max)
            {
                coordinate_type prop = 0.7 * buffer_distance;
                prop /= length;
                set<0>(p, get<0>(vertex) + dx * prop);
                set<1>(p, get<1>(vertex) + dy * prop);

                std::cout << length << std::endl;
            }


            buffered.push_back(p);
        }


        // Map it
        {
            Ring corner;
            corner.push_back(vertex);
            corner.push_back(perp1);
            corner.push_back(ip);
            corner.push_back(perp2);
            corner.push_back(vertex);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            const_cast<Mapper&>(m_mapper).map(corner,
                "opacity:0.4;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:1");
#endif
        }
    }
};


template
<
    typename PointIn,
    typename PointOut
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
struct join_bevel
{

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    Mapper const& m_mapper;

    join_bevel(Mapper const& mapper)
        : m_mapper(mapper)
    {}
#endif

    template <typename Ring>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                double buffer_distance,
                Ring& buffered) const
    {
        buffered.push_back(perp1);
        buffered.push_back(perp2);

        // Map it
        {
            Ring corner;
            corner.push_back(vertex);
            corner.push_back(perp1);
            corner.push_back(perp2);
            corner.push_back(vertex);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            const_cast<Mapper&>(m_mapper).map(corner,
                    "opacity:0.4;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:1");
#endif
        }
    }
};


template
<
    typename PointIn,
    typename PointOut
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
struct join_round
{
    int m_max_level;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    Mapper const& m_mapper;

    join_round(Mapper const& mapper, int max_level = 4)
        : m_mapper(mapper)
        , m_max_level(max_level)
    {}
#endif

    inline join_round(int max_level = 4)
        : m_max_level(max_level)
    {}


    template <typename Ring>
    inline void mid_points(PointIn const& vertex,
                PointIn const& p1, PointIn const& p2,
                double buffer_distance,
                Ring& buffered,
                int level = 1) const
    {
        // Generate 'vectors'
        double vp1_x = get<0>(p1) - get<0>(vertex);
        double vp1_y = get<1>(p1) - get<1>(vertex);

        double vp2_x = (get<0>(p2) - get<0>(vertex));
        double vp2_y = (get<1>(p2) - get<1>(vertex));

        // Average them to generate vector in between
        double two = 2;
        double v_x = (vp1_x + vp2_x) / two;
        double v_y = (vp1_y + vp2_y) / two;

        double length2 = sqrt(v_x * v_x + v_y * v_y);

        double prop = buffer_distance / length2;

        PointIn mid_point;
        set<0>(mid_point, get<0>(vertex) + v_x * prop);
        set<1>(mid_point, get<1>(vertex) + v_y * prop);

        if (level < m_max_level)
        {
            mid_points(vertex, p1, mid_point, buffer_distance, buffered, level + 1);
            mid_points(vertex, mid_point, p2, buffer_distance, buffered, level + 1);
        }

        buffered.push_back(p2);
    }


    template <typename Ring>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                double buffer_distance,
                Ring& buffered) const
    {
        // Generate 'vectors'
        double vpx = get<0>(perp1) - get<0>(vertex);
        double vpy = get<1>(perp1) - get<1>(vertex);

        double vix = (get<0>(ip) - get<0>(vertex));
        double viy = (get<1>(ip) - get<1>(vertex));

        double length_i = sqrt(vix * vix + viy * viy);

        double prop = buffer_distance / length_i;

        PointIn bp;
        set<0>(bp, get<0>(vertex) + vix * prop);
        set<1>(bp, get<1>(vertex) + viy * prop);

        if (m_max_level <= 1)
        {
            buffered.push_back(perp1);
            if (m_max_level == 1)
            {
                buffered.push_back(bp);
            }
            buffered.push_back(perp2);
        }
        else
        {
            buffered.push_back(perp1);
            mid_points(vertex, perp1, bp, buffer_distance, buffered);
            mid_points(vertex, bp, perp2, buffer_distance, buffered);
        }


        // Map it
        {
            Ring corner;
            corner.push_back(vertex);
            corner.push_back(perp1);
            corner.push_back(bp);
            corner.push_back(perp2);
            corner.push_back(vertex);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            const_cast<Mapper&>(m_mapper).map(corner,
                    "opacity:0.4;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:1");
#endif
        }
    }
};

}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_HPP
