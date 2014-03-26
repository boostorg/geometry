// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_TOUCHES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_TOUCHES_HPP


#include <deque>

#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/num_geometries.hpp>
#include <boost/geometry/algorithms/detail/sub_range.hpp>
#include <boost/geometry/policies/robustness/no_rescale_policy.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace touches
{

struct areal_interrupt_policy
{
    static bool const enabled = true;
    bool found_touch;
    bool found_not_touch;

    // dummy variable required by self_get_turn_points::get_turns
    static bool const has_intersections = false;

    inline bool result()
    {
        return found_touch && !found_not_touch;
    }

    inline areal_interrupt_policy()
        : found_touch(false), found_not_touch(false)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        // if already rejected (temp workaround?)
        if ( found_not_touch )
            return true;

        typedef typename boost::range_iterator<Range const>::type iterator;
        for ( iterator it = boost::begin(range) ; it != boost::end(range) ; ++it )
        {
            if ( it->has(overlay::operation_intersection) )
            {
                found_not_touch = true;
                return true;
            }

            switch(it->method)
            {
                case overlay::method_crosses:
                    found_not_touch = true;
                    return true;
                case overlay::method_equal:
                    // Segment spatially equal means: at the right side
                    // the polygon internally overlaps. So return false.
                    found_not_touch = true;
                    return true;
                case overlay::method_touch:
                case overlay::method_touch_interior:
                case overlay::method_collinear:
                    if ( ok_for_touch(*it) )
                    {
                        found_touch = true;
                    }
                    else
                    {
                        found_not_touch = true;
                        return true;
                    }
                    break;
                case overlay::method_none :
                case overlay::method_disjoint :
                case overlay::method_error :
                    break;
            }
        }

        return false;
    }

    template <typename Turn>
    inline bool ok_for_touch(Turn const& turn)
    {
        return turn.both(overlay::operation_union)
            || turn.both(overlay::operation_blocked)
            || turn.combination(overlay::operation_union, overlay::operation_blocked)
            ;
    }
};

template <typename Linear>
struct linear_areal_interrupt_policy
{
    static bool const enabled = true;
    bool found_touch;
    bool found_not_touch;

    Linear const& linear;

    // dummy variable required by self_get_turn_points::get_turns
    static bool const has_intersections = false;

    inline bool result()
    {
        return found_touch && !found_not_touch;
    }

    inline linear_areal_interrupt_policy(Linear const& l)
        : found_touch(false), found_not_touch(false), linear(l)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        // if already rejected (temp workaround?)
        if ( found_not_touch )
            return true;

        typedef typename boost::range_iterator<Range const>::type iterator;
        for ( iterator it = boost::begin(range) ; it != boost::end(range) ; ++it )
        {
            if ( it->operations[0].operation == overlay::operation_intersection )
            {
                if ( it->operations[1].operation == overlay::operation_union &&
                     is_turn_on_last_point(*it) )
                {
                    found_touch = true;
                    continue;
                }
                else
                {
                    found_not_touch = true;
                    return true;
                }
            }

            switch(it->method)
            {
                case overlay::method_crosses:
                    found_not_touch = true;
                    return true;
                case overlay::method_equal:
                case overlay::method_touch:
                case overlay::method_touch_interior:
                case overlay::method_collinear:
                    if ( ok_for_touch(*it) )
                    {
                        found_touch = true;
                    }
                    else
                    {
                        found_not_touch = true;
                        return true;
                    }
                    break;
                case overlay::method_none :
                case overlay::method_disjoint :
                case overlay::method_error :
                    break;
            }
        }

        return false;
    }

    template <typename Turn>
    inline bool ok_for_touch(Turn const& turn)
    {
        return turn.both(overlay::operation_union)
            || turn.both(overlay::operation_blocked)
            || turn.combination(overlay::operation_union, overlay::operation_blocked)

            || turn.both(overlay::operation_continue)
            || turn.combination(overlay::operation_union, overlay::operation_intersection)
            ;
    }

    template <typename Turn>
    inline bool is_turn_on_last_point(Turn const& turn)
    {
        typename sub_range_return_type<Linear const>::type
            g = sub_range(linear, turn.operations[0].seg_id);

        std::size_t s = boost::size(g);

        if ( s == 0 )
            return false; // shouldn't return here
        else if ( s == 1 )
            return equals::equals_point_point(turn.point, *boost::begin(g)); // nor here
        else
            return equals::equals_point_point(turn.point, *(boost::end(g)-1));
    }
};

template <std::size_t Max>
struct turns_count_interrupt_policy
{
    static bool const enabled = true;
    std::size_t turns_count;

    // dummy variable required by self_get_turn_points::get_turns
    static bool const has_intersections = false;

    inline turns_count_interrupt_policy()
        : turns_count(0)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        turns_count += boost::size(range);
        if ( Max < turns_count )
            return true;
        return false;
    }
};


template<typename Geometry>
struct check_each_ring_for_within
{
    bool has_within;
    Geometry const& m_geometry;

    inline check_each_ring_for_within(Geometry const& g)
        : has_within(false)
        , m_geometry(g)
    {}

    template <typename Range>
    inline void apply(Range const& range)
    {
        typename geometry::point_type<Range>::type p;
        geometry::point_on_border(p, range);
        if (geometry::within(p, m_geometry))
        {
            has_within = true;
        }
    }
};

// TODO: currently this function checks if a point of at least one range from g2 is within g1
//       shouldn't it check the opposite?

template <typename FirstGeometry, typename SecondGeometry>
inline bool rings_containing(FirstGeometry const& geometry1,
                SecondGeometry const& geometry2)
{
    check_each_ring_for_within<FirstGeometry> checker(geometry1);
    geometry::detail::for_each_range(geometry2, checker);
    return checker.has_within;
}

template <typename Geometry1, typename Geometry2>
struct areal_areal
{
    static inline
    bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        typedef detail::no_rescale_policy rescale_policy_type;
        typedef typename geometry::point_type<Geometry1>::type point_type;
        typedef detail::overlay::turn_info
            <
                point_type,
                typename segment_ratio_type<point_type, rescale_policy_type>::type
            > turn_info;

        typedef detail::overlay::get_turn_info
            <
                detail::overlay::assign_null_policy
            > policy_type;

        std::deque<turn_info> turns;
        detail::touches::areal_interrupt_policy policy;
        rescale_policy_type rescale_policy;
        boost::geometry::get_turns
                <
                    detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value,
                    detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value,
                    detail::overlay::assign_null_policy
                >(geometry1, geometry2, rescale_policy, turns, policy);

        return policy.result()
            && ! geometry::detail::touches::rings_containing(geometry1, geometry2)
            && ! geometry::detail::touches::rings_containing(geometry2, geometry1);
    }
};

template <typename Geometry1, typename Geometry2>
struct linear_areal
{
    static inline
    bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        typedef detail::no_rescale_policy rescale_policy_type;
        typedef typename geometry::point_type<Geometry1>::type point_type;
        typedef detail::overlay::turn_info
            <
                point_type,
                typename segment_ratio_type<point_type, rescale_policy_type>::type
            > turn_info;

        std::deque<turn_info> turns;
        detail::touches::linear_areal_interrupt_policy<Geometry1> policy(geometry1);
        rescale_policy_type rescale_policy;
        boost::geometry::get_turns
                <
                    detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value,
                    detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value,
                    detail::overlay::assign_null_policy
                >(geometry1, geometry2, rescale_policy, turns, policy);

        return policy.result()
            && ! geometry::detail::touches::rings_containing(geometry2, geometry1);
    }
};

template <typename Point, typename Geometry>
struct point_geometry
{
    static inline
    bool apply(Point const& point, Geometry const& geometry)
    {
        return detail::within::point_in_geometry(point, geometry) == 0;
    }
};

template <typename Geometry1, typename Geometry2>
struct linestring_linestring
{
    static inline
    bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        std::size_t s1 = boost::size(geometry1);
        std::size_t s2 = boost::size(geometry2);
        // TODO: throw on empty input?
        if ( s1 == 0 || s2 == 0 )
            return false;

        typedef detail::no_rescale_policy rescale_policy_type;
        typedef typename geometry::point_type<Geometry1>::type point_type;
        typedef detail::overlay::turn_info
            <
                point_type,
                typename segment_ratio_type<point_type, rescale_policy_type>::type
            > turn_info;

        typedef detail::overlay::get_turn_info
            <
                detail::overlay::assign_null_policy
            > policy_type;

        std::deque<turn_info> turns;
        turns_count_interrupt_policy<2> policy;
        rescale_policy_type rescale_policy;
        boost::geometry::get_turns
                <
                    detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value,
                    detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value,
                    detail::overlay::assign_null_policy
                >(geometry1, geometry2, rescale_policy, turns, policy);

        if ( 2 < policy.turns_count )
            return false;
        else if ( 2 == policy.turns_count )
        {
            return ( detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry1)-1))
                  && detail::equals::equals_point_point(turns[1].point, *(boost::end(geometry2)-1)) )
                || ( detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry2)-1))
                  && detail::equals::equals_point_point(turns[1].point, *(boost::end(geometry1)-1)) );
        }
        else if ( 1 == policy.turns_count )
        {
            return detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry1)-1))
                || detail::equals::equals_point_point(turns[0].point, *(boost::end(geometry2)-1));
        }
        else
        {
            return detail::within::point_in_geometry(*boost::begin(geometry1), geometry2) >= 0
                || detail::within::point_in_geometry(*boost::begin(geometry2), geometry1) >= 0;
        }
    }
};

}}
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

// TODO: Since CastedTags are used is Reverse needed?

template
<
    typename Geometry1, typename Geometry2,
    typename Tag1 = typename tag<Geometry1>::type,
    typename Tag2 = typename tag<Geometry2>::type,
    typename CastedTag1 = typename tag_cast<Tag1, point_tag, linear_tag, areal_tag>::type,
    typename CastedTag2 = typename tag_cast<Tag2, point_tag, linear_tag, areal_tag>::type,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct touches : not_implemented<Tag1, Tag2>
{};

// If reversal is needed, perform it
template
<
    typename Geometry1, typename Geometry2,
    typename Tag1, typename Tag2,
    typename CastedTag1, typename CastedTag2
>
struct touches<Geometry1, Geometry2, Tag1, Tag2, CastedTag1, CastedTag2, true>
    : touches<Geometry2, Geometry1, Tag2, Tag1, CastedTag2, CastedTag1, false>
{
    static inline bool apply(Geometry1 const& g1, Geometry2 const& g2)
    {
        return touches<Geometry2, Geometry1>::apply(g2, g1);
    }
};

// touches(Pt, Pt), touches(Pt, MPt), touches(MPt, MPt)
template <typename Geometry1, typename Geometry2, typename Tag1, typename Tag2>
struct touches<Geometry1, Geometry2, Tag1, Tag2, point_tag, point_tag, false>
    : detail::touches::point_geometry<Geometry1, Geometry2>
{
    static inline bool apply(Geometry1 const& , Geometry2 const& )
    {
        return false;
    }
};

// touches(Point, Linear), touches(Point, Areal)
template <typename Point, typename Geometry, typename Tag2, typename CastedTag2>
struct touches<Point, Geometry, point_tag, Tag2, point_tag, CastedTag2, false>
    : detail::touches::point_geometry<Point, Geometry>
{};

// TODO: support touches(MPt, Linear/Areal)

// touches(Linestring, Linestring)
template <typename Linestring1, typename Linestring2>
struct touches<Linestring1, Linestring2, linestring_tag, linestring_tag, linear_tag, linear_tag, false>
    : detail::touches::linestring_linestring<Linestring1, Linestring2>
{};

// TODO: support touches(MLs, MLs) and touches(Ls, MLs)

// touches(Linear, Areal)
template <typename Linear, typename Areal, typename Tag1, typename Tag2>
struct touches<Linear, Areal, Tag1, Tag2, linear_tag, areal_tag, false>
    : detail::touches::linear_areal<Linear, Areal>
{};
// e.g. for touches(Poly, MLs)
template <typename Areal, typename Linear, typename Tag1, typename Tag2>
struct touches<Areal, Linear, Tag1, Tag2, areal_tag, linear_tag, false>
{
    static inline bool apply(Areal const& areal, Linear const& linear)
    {
        return detail::touches::linear_areal<Linear, Areal>::apply(linear, areal);
    }
};

// touches(Areal, Areal)
template <typename Areal1, typename Areal2, typename Tag1, typename Tag2>
struct touches<Areal1, Areal2, Tag1, Tag2, areal_tag, areal_tag, false>
    : detail::touches::areal_areal<Areal1, Areal2>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_variant {

template <typename Geometry1, typename Geometry2>
struct touches
{
    static bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        concept::check<Geometry1 const>();
        concept::check<Geometry2 const>();

        return dispatch::touches<Geometry1, Geometry2>
                       ::apply(geometry1, geometry2);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Geometry2>
struct touches<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Geometry2>
{
    struct visitor: boost::static_visitor<bool>
    {
        Geometry2 const& m_geometry2;

        visitor(Geometry2 const& geometry2): m_geometry2(geometry2) {}

        template <typename Geometry1>
        bool operator()(Geometry1 const& geometry1) const
        {
            return touches<Geometry1, Geometry2>::apply(geometry1, m_geometry2);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry1,
          Geometry2 const& geometry2)
    {
        return boost::apply_visitor(visitor(geometry2), geometry1);
    }
};

template <typename Geometry1, BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct touches<Geometry1, boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor: boost::static_visitor<bool>
    {
        Geometry1 const& m_geometry1;

        visitor(Geometry1 const& geometry1): m_geometry1(geometry1) {}

        template <typename Geometry2>
        bool operator()(Geometry2 const& geometry2) const
        {
            return touches<Geometry1, Geometry2>::apply(m_geometry1, geometry2);
        }
    };

    static inline bool
    apply(Geometry1 const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry2)
    {
        return boost::apply_visitor(visitor(geometry1), geometry2);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T1),
          BOOST_VARIANT_ENUM_PARAMS(typename T2)>
struct touches<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
               boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)> >
{
    struct visitor: boost::static_visitor<bool>
    {
        template <typename Geometry1, typename Geometry2>
        bool operator()(Geometry1 const& geometry1,
                        Geometry2 const& geometry2) const
        {
            return touches<Geometry1, Geometry2>::apply(geometry1, geometry2);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)> const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)> const& geometry2)
    {
        return boost::apply_visitor(visitor(), geometry1, geometry2);
    }
};

template <typename Geometry>
struct self_touches
{
    static bool apply(Geometry const& geometry)
    {
        concept::check<Geometry const>();

        typedef detail::no_rescale_policy rescale_policy_type;
        typedef typename geometry::point_type<Geometry>::type point_type;
        typedef detail::overlay::turn_info
            <
                point_type,
                typename segment_ratio_type<point_type, rescale_policy_type>::type
            > turn_info;

        typedef detail::overlay::get_turn_info
        <
            detail::overlay::assign_null_policy
        > policy_type;

        std::deque<turn_info> turns;
        detail::touches::areal_interrupt_policy policy;
        rescale_policy_type rescale_policy;
        detail::self_get_turn_points::get_turns
        <
            policy_type
        >::apply(geometry, rescale_policy, turns, policy);

        return policy.result();
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct self_touches<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor: boost::static_visitor<bool>
    {
        template <typename Geometry>
        bool operator()(Geometry const& geometry) const
        {
            return self_touches<Geometry>::apply(geometry);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry)
    {
        return boost::apply_visitor(visitor(), geometry);
    }
};

} // namespace resolve_variant


/*!
\brief \brief_check{has at least one touching point (self-tangency)}
\note This function can be called for one geometry (self-tangency) and
    also for two geometries (touch)
\ingroup touches
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_check{is self-touching}

\qbk{distinguish,one geometry}
\qbk{[def __one_parameter__]}
\qbk{[include reference/algorithms/touches.qbk]}
*/
template <typename Geometry>
inline bool touches(Geometry const& geometry)
{
    return resolve_variant::self_touches<Geometry>::apply(geometry);
}


/*!
\brief \brief_check2{have at least one touching point (tangent - non overlapping)}
\ingroup touches
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return \return_check2{touch each other}

\qbk{distinguish,two geometries}
\qbk{[include reference/algorithms/touches.qbk]}
 */
template <typename Geometry1, typename Geometry2>
inline bool touches(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    return resolve_variant::touches<Geometry1, Geometry2>::apply(geometry1, geometry2);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_TOUCHES_HPP
