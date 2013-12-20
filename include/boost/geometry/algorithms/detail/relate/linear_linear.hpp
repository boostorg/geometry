// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP

#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp> // later change to equal/point_point.hpp
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/geometry/algorithms/detail/relate/turns.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

enum field { interior = 0, boundary = 1, exterior = 2 };

// With DE9IM only Dimension < 10 is supported
class result
{
public:
    result()
    {
        set('F');
    }

    result(const char * str)
    {
        ::memcpy(array, str, 9);
    }

    template <field F1, field F2>
    char get() const
    {
        return array[F1 * 3 + F2];
    }

    template <field F1, field F2>
    void set(char v)
    {
        array[F1 * 3 + F2] = v;
    }

    template <field F1, field F2>
    void update_dimension(char v)
    {
        char * c = array + F1 * 3 + F2;
        if ( v > *c || *c > '9')
            *c = v;
    }

    template <field F1, field F2>
    void update(char v)
    {
        char * c = array + F1 * 3 + F2;
        if ( *c > '9')
            *c = v;
    }

    void set(char v)
    {
        ::memset(array, v, 9);
    }

private:
    char array[9];
};

template <typename Point1, typename Point2>
struct point_point
{
    static inline result apply(Point1 const& point1, Point2 const& point2)
    {
        bool equal = detail::equals::equals_point_point(point1, point2);

        if ( equal )
            return result("0FFFFFFFT");
        else
            return result("FF0FFF0FT");
    }
};

template <typename Point, typename Geometry>
struct point_geometry
{
    static inline result apply(Point const& point, Geometry const& geometry)
    {
        int pig = detail::within::point_in_geometry(point, geometry);

        // TODO: ? - if geometry has interior and/or boundary
        // e.g. isn't 1-point linestring or linear ring

        if ( pig < 0 ) // not within
            return result("FF0FFF??T");
        else if ( pig == 0 )
            return result("F0FFFF??T");
        else // pig > 0 - within
            return result("0FFFFF??T");
    }
};

// transposed result of point_geometry
template <typename Geometry, typename Point>
struct geometry_point
{
    static inline result apply(Geometry const& geometry, Point const& point)
    {
        int pig = detail::within::point_in_geometry(point, geometry);

        // TODO: ? - if geometry has interior and/or boundary
        // e.g. isn't 1-point linestring or linear ring

        if ( pig < 0 ) // not within
            return result("FF?FF?0FT");
        else if ( pig == 0 )
            return result("FF?0F?FFT");
        else // pig > 0 - within
            return result("0F?FF?FFT");
    }
};

template<typename P>
struct distance_info
{
    typedef typename strategy::distance::services::return_type
        <
            typename strategy::distance::services::comparable_type
                <
                    typename strategy::distance::services::default_strategy
                        <
                            point_tag,
                            P
                        >::type
                >::type,
            P, P
        >::type distance_type;

    inline distance_info()
        : distance(distance_type())
    {}

    distance_type distance; // distance-measurement from segment.first to IP
};

template <typename P>
struct turn_operation_with_distance : public overlay::turn_operation
{
    distance_info<P> enriched;
};

template <typename Geometry1, typename Geometry2>
struct get_turns
{
    typedef typename geometry::point_type<Geometry1>::type point1_type;

    typedef overlay::turn_info
        <
            point1_type,
            turn_operation_with_distance<point1_type>
        > turn_info;

    template <typename Turns>
    static inline void apply(Turns & turns, Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        typedef //overlay::assign_null_policy
                overlay::calculate_distance_policy assign_policy;

        detail::get_turns::no_interrupt_policy interrupt_policy;

        boost::geometry::get_turns
                <
                    detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value,
                    detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value,
                    assign_policy
                >(geometry1, geometry2, detail::no_rescale_policy(), turns, interrupt_policy);
    }
};

// currently works only for linestrings
template <typename Geometry1, typename Geometry2>
struct linear_linear
{
    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;

    template <typename Range>
    static inline typename boost::range_value<Range>::type const&
    front(Range const& rng)
    {
        BOOST_ASSERT(!boost::empty(rng));
        return *boost::begin(rng);
    }

    template <typename Range>
    static inline typename boost::range_value<Range>::type const&
    back(Range const& rng)
    {
        BOOST_ASSERT(!boost::empty(rng));
        return *(--boost::end(rng));
    }

    static inline result apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        // TODO: currently this only works for linestrings
        std::size_t s1 = boost::size(geometry1);
        std::size_t s2 = boost::size(geometry2);
        if ( s1 == 0 || s2 == 0 )
        {
            // throw on empty output?
            return result("FFFFFFFFF");
        }
        else if ( s1 == 1 && s2 == 1 )
        {
            return point_point<point1_type, point2_type>::apply(front(geometry1), front(geometry2));
        }
        else if ( s1 == 1 /*&& s2 > 1*/ )
        {
            return point_geometry<point1_type, Geometry2>::apply(front(geometry1), geometry2);
        }
        else if ( s2 == 1 /*&& s1 > 1*/  )
        {
            return geometry_point<Geometry2, point2_type>::apply(geometry1, front(geometry2));
        }

        // TODO: handle also linestrings with points_num == 2 and equals(front, back) - treat like point?

        result res("FFFFFFFFF");

        // TODO: implement generic function forking also for multilinestrings, also use it in point_in_geometry
        bool has_boundary1 = ! detail::equals::equals_point_point(front(geometry1), back(geometry1));
        bool has_boundary2 = ! detail::equals::equals_point_point(front(geometry2), back(geometry2));

        int pig_front = detail::within::point_in_geometry(front(geometry1), geometry2);

        if ( has_boundary1 )
        {
            int pig_back = detail::within::point_in_geometry(back(geometry1), geometry2);

            if ( pig_front > 0 || pig_back > 0 )
                res.template set<boundary, interior>('0');
            if ( pig_front == 0 || pig_back == 0 )
                res.template set<boundary, boundary>('0');
            if ( pig_front < 0 || pig_back < 0 )
                res.template set<boundary, exterior>('0');
        }
        else
        {
            if ( pig_front > 0 )
                res.template set<interior, interior>('0');
            else if ( pig_front == 0 )
                res.template set<interior, boundary>('0');
            else if ( pig_front < 0 )
                res.template set<interior, exterior>('0');
        }

        // get and analyse turns

        std::deque<typename get_turns<Geometry1, Geometry2>::turn_info> turns;

        get_turns<Geometry1, Geometry2>::apply(turns, geometry2, geometry2);

        // TODO: turns must be analysed this way only if it's possible to go out and in on the same point
        // for linear geometries union or intersection operation was detected
        std::sort(turns.begin(), turns.end(), relate::turns::less_seg_dist_op<relate::turns::operation_order_uibc>());

        analyse_turns(res, turns.begin(), turns.end(), geometry1, geometry2, has_boundary1, has_boundary2);

        return res;
    }

    template <typename TurnIt>
    static inline void analyse_turns(result & res,
                                     TurnIt first, TurnIt last,
                                     Geometry1 const& geometry1, Geometry2 const& geometry2,
                                     bool has_boundary1, bool has_boundary2)
    {
        for ( TurnIt it = first ; it != last ; ++it )
        {
            switch ( it->method )
            {
                case overlay::method_crosses:
                    handle_crosses(res);
                    break;
                case overlay::method_touch:
                case overlay::method_touch_interior:
                    handle_terminal_point(res, *it, geometry1, geometry2, has_boundary1, has_boundary2);
                    handle_touch(res, *it, geometry1, geometry2, has_boundary1, has_boundary2);
                    break;
                case overlay::method_equal:
                case overlay::method_collinear:
                    // probably there is no need to check it for collinear
                    // and for equal only one linestring shoudl be checked, not both
                    handle_terminal_point(res, *it, geometry1, geometry2, has_boundary1, has_boundary2);
                    handle_col_eq(res);
                    break;
                case overlay::method_none :
                case overlay::method_disjoint :
                case overlay::method_error :
                    break;
            }
        }
    }

    static inline void handle_crosses(result & res)
    {
        res.template update_dimension<interior, interior>('0');
        res.template update_dimension<interior, exterior>('1');
        res.template update_dimension<exterior, interior>('1');
    }

    static inline void handle_col_eq(result & res)
    {
        res.template update_dimension<interior, interior>('1');
    }

    template <typename Turn>
    static inline void handle_touch(result & res,
                                    Turn const& turn,
                                    Geometry1 const& geometry1, Geometry2 const& geometry2,
                                    bool has_boundary1, bool has_boundary2)
    {
        if ( turn.both(overlay::operation_continue)
          || turn.both(overlay::operation_blocked) )
        {
            res.template update_dimension<interior, interior>('1');
        }
        else if ( turn.has(overlay::operation_union) || turn.has(overlay::operation_intersection) )
        {
            res.template update_dimension<interior, exterior>('1');
            res.template update_dimension<exterior, interior>('1');

            // boundaries shoudl probably be also handled here
            // boundaries of G1 was already set
        }

        // TODO: THE REST OF CONDITIONS + CHECK FOR OUT-IN
    }

    template <typename Turn>
    static inline void handle_terminal_point(result & res,
                                             Turn const& turn,
                                             Geometry1 const& geometry1, Geometry2 const& geometry2,
                                             bool has_boundary1, bool has_boundary2)
    {
        bool pt_on_boundary1 = has_boundary1 && equals_terminal_point(turn.point, geometry1);
        bool pt_on_boundary2 = has_boundary2 && equals_terminal_point(turn.point, geometry2);

        if ( pt_on_boundary1 && pt_on_boundary2 )
            res.template update_dimension<boundary, boundary>('0');
        else if ( pt_on_boundary1 )
            res.template update_dimension<boundary, interior>('0');
        else if ( pt_on_boundary2 )
            res.template update_dimension<interior, boundary>('0');
    }

    // TODO: replace with generic point_in_boundary working also for multilinestrings
    template <typename Point, typename Geometry>
    static inline bool equals_terminal_point(Point const& point, Geometry const& geometry)
    {
        return detail::equals::equals_point_point(point, front(geometry))
            || detail::equals::equals_point_point(point, back(geometry));
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LINEAR_LINEAR_HPP
