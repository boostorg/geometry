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
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/buffer_side.hpp>


#define BOOST_GEOMETRY_BUFFER_NO_HELPER_POINTS


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


#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
template
<
    typename PointIn, typename Mapper
>
struct join_mapper
{
    Mapper const& m_mapper;
    join_mapper(Mapper const& mapper)
        : m_mapper(mapper)
    {}

    template <typename Ring>
    inline void map(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2) const
    {
        Ring corner;
        corner.push_back(vertex);
        corner.push_back(perp1);
        corner.push_back(ip);
        corner.push_back(perp2);
        corner.push_back(vertex);

        const_cast<Mapper&>(m_mapper).map(corner,
            "opacity:0.4;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:1");
    }
};
#endif



#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
// Forget this, it will go
template<typename PointIn, typename PointOut, typename Mapper>
struct join_miter : public join_mapper<PointIn, Mapper>
{
    join_miter(Mapper const& mapper) : join_mapper(mapper) {}
#else


template
<
    typename PointIn,
    typename PointOut
>
struct join_miter
{

#endif
    typedef typename strategy_side<typename cs_tag<PointIn>::type>::type side;
    typedef typename coordinate_type<PointIn>::type coordinate_type;


    template <typename Ring>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                Ring& buffered) const
    {
        coordinate_type zero = 0;
        int signum = buffer_distance > zero
            ? 1
            : buffer_distance < zero
                ? -1
                : 0;

        if (side::apply(perp1, ip, perp2) == signum)
        {

#ifdef BOOST_GEOMETRY_BUFFER_NO_HELPER_POINTS
            // Because perp1 crosses perp2 at IP, it is not necessary to
            // include IP
            buffered.push_back(ip);
#else
            // If it is concave (corner to left), add helperline
            // The helper-line IS essential for buffering holes. Without,
            // holes might be generated, while they should NOT be there.
            // DOES NOT WORK ALWAYS buffered.push_back(ip);
            // We might consider to make it optional (because more efficient)
            buffered.push_back(perp1);
            buffered.push_back(perp2);
#endif
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

            buffered.push_back(p);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            map<Ring>(ip, vertex, perp1, perp2);
#endif
        }


    }
};


#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
// Forget this, it will go
template<typename PointIn, typename PointOut, typename Mapper>
struct join_bevel : public join_mapper<PointIn, Mapper>
{
    join_bevel(Mapper const& mapper) : join_mapper(mapper) {}
#else


template
<
    typename PointIn,
    typename PointOut
>
struct join_bevel
{
#endif


    typedef typename coordinate_type<PointIn>::type coordinate_type;

    template <typename Ring>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                Ring& buffered) const
    {
        buffered.push_back(perp1);
        buffered.push_back(perp2);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            map<Ring>(ip, vertex, perp1, perp2);
#endif
    }
};


#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
// Forget this, it will go
template<typename PointIn, typename PointOut, typename Mapper>
struct join_round : public join_mapper<PointIn, Mapper>
{
    join_round(Mapper const& mapper, int max_level = 4)
        : join_mapper(mapper)
        , m_max_level(max_level)
    {}
#else


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
#endif

    typedef typename strategy_side<typename cs_tag<PointIn>::type>::type side;
    typedef typename coordinate_type<PointOut>::type coordinate_type;
    int m_max_level;


    template <typename Ring>
    inline void mid_points(PointIn const& vertex,
                PointIn const& p1, PointIn const& p2,
                coordinate_type const& buffer_distance,
                Ring& buffered,
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
            mid_points(vertex, p1, mid_point, buffer_distance, buffered, level + 1);
        }
        buffered.push_back(mid_point);
        if (level < m_max_level)
        {
            mid_points(vertex, mid_point, p2, buffer_distance, buffered, level + 1);
        }

    }


    template <typename Ring>
    inline void apply(PointIn const& ip, PointIn const& vertex,
                PointIn const& perp1, PointIn const& perp2,
                coordinate_type const& buffer_distance,
                Ring& buffered) const
    {
        coordinate_type zero = 0;
        int signum = buffer_distance > zero
            ? 1
            : buffer_distance < zero
                ? -1
                : 0;

        if (side::apply(perp1, ip, perp2) == signum)
        {
#ifdef BOOST_GEOMETRY_BUFFER_NO_HELPER_POINTS
            buffered.push_back(ip);
#else
            // If it is concave (corner to left), add helperline
            buffered.push_back(perp1);
            buffered.push_back(perp2);
#endif
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
                mid_points(vertex, perp1, bp, bd, buffered);
                mid_points(vertex, bp, perp2, bd, buffered);
                buffered.push_back(perp2);
            }

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            map<Ring>(bp, vertex, perp1, perp2);
#endif
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

private :
    CoordinateType m_left;
    CoordinateType m_right;
};


}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_HPP
