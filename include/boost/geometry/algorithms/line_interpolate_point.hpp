// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018, 2019 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_LINE_INTERPOLATE_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_LINE_INTERPOLATE_POINT_HPP

#include <iterator>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/value_type.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/line_interpolate_point.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace interpolate_point
{

struct convert_and_push_back
{
    template <typename Range, typename Point>
    inline void apply(Point const& p, Range& range)
    {
        typename boost::range_value<Range>::type p2;
        geometry::detail::conversion::convert_point_to_point(p, p2);
        range::push_back(range, p2);
    }
};

struct convert_and_assign
{
    template <typename Point1, typename Point2>
    inline void apply(Point1 const& p1, Point2& p2)
    {
        geometry::detail::conversion::convert_point_to_point(p1, p2);
    }

};


/*!
\brief Internal, calculates interpolation point of a linestring using iterator pairs and
    specified strategy
*/
template <typename Policy>
struct range
{
    template
    <
        typename Range,
        typename Distance,
        typename PointLike,
        typename Strategy
    >
    static inline void apply(Range const& range,
                             Distance const& max_distance,
                             PointLike & pointlike,
                             Strategy const& strategy)
    {
        Policy policy;

        typedef typename boost::range_iterator<Range const>::type iterator_t;
        typedef typename boost::range_value<Range const>::type point_t;

 //       typedef typename select_most_precise
 //           <
 //               typename default_length_result<Range>::type,
 //               double
 //           >::type calc_t;

        typedef typename Strategy::template result_type<point_t> result_type;
//        typedef boost::container::vector<result_type> vector_t;
//        typedef typename boost::range_iterator<vector_t const>::type viterator_t;

        iterator_t it = boost::begin(range);
        iterator_t end = boost::end(range);

        if (it == end) // empty(range)
        {
            return;
        }
        if (max_distance <= 0) //non positive distance
        {
            policy.apply(*it, pointlike);
            return;
        }


/*        if (fraction == 1)
        {
            policy.apply(*(end-1), pointlike);
            return;
        }
*/
        // compute lengths of segments and their sum
/*        calc_t tot_len = 0;
        vector_t lengths;
        iterator_t prev = it++;
        do {
            result_type result = strategy.compute(*prev, *it);
            lengths.push_back(result);
            tot_len += result.distance;
            prev = it++;
        } while (it != end);

        //compute interpolation point(s)
        calc_t repeated_fraction = fraction;
        calc_t prev_fraction = 0;
        calc_t cur_fraction = 0;
        unsigned int i = 1;
        viterator_t vit = boost::begin(lengths);
        it = boost::begin(range);
        prev = it++;
        bool single_point = false;

        do {
            calc_t seg_fraction = vit->distance / tot_len;

            cur_fraction = (it + 1 == end) ? 1 : prev_fraction + seg_fraction;

            while (cur_fraction >= repeated_fraction && !single_point)
            {
                point_t p;

                strategy.apply(*prev, *it,
                               (repeated_fraction - prev_fraction) / seg_fraction,
                               p, *vit);
                single_point = boost::is_same<Policy, convert_and_assign>::value;
                policy.apply(p, pointlike);

                repeated_fraction = ++i * fraction;
            }

            prev_fraction = cur_fraction;
            prev = it++;
            vit++;

        } while (it != end && !single_point);
    }
*/
        iterator_t prev = it++;
        Distance repeated_distance = max_distance;
        Distance prev_distance = 0;
        Distance current_distance = 0;
        point_t start_p = *prev;
        bool single_point = false;

        do {
            result_type res = strategy.compute(*prev, *it);
            //seg_distance += res.distance;

//            current_distance = (it + 1 == end) ? 1 : prev_distance + res.distance;
            current_distance = prev_distance + res.distance;

            while (current_distance >= repeated_distance && !single_point)
            {
                point_t p;
                strategy.apply(start_p, *it,
                               //repeated_distance - prev_distance,
                               (repeated_distance - prev_distance)
                               /(current_distance - prev_distance),
                               p,
                               current_distance - prev_distance,
                               res);
                single_point = boost::is_same<Policy, convert_and_assign>::value;
                policy.apply(p, pointlike);
                start_p = p;
                prev_distance = repeated_distance;
                repeated_distance += max_distance;
            }

            prev_distance = current_distance;
            prev = it++;
            start_p = *prev;

        } while (it != end && !single_point);

        // case when max_distance is larger than linestring's length
        // return the last point in range (range is not empty)
        if (repeated_distance == max_distance)
        {
            policy.apply(*(end-1), pointlike);
        }
    }
};

template <typename Policy>
struct segment
{
    template <typename Segment, typename Distance, typename Pointlike, typename Strategy>
    static inline void apply(Segment const& segment,
                             Distance const& max_distance,
                             Pointlike & point,
                             Strategy const& strategy)
    {
        range<Policy>().apply(segment_view<Segment>(segment),
                              max_distance, point, strategy);
    }
};

}} // namespace detail::length
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Geometry,
    typename Pointlike,
    typename Tag1 = typename tag<Geometry>::type,
    typename Tag2 = typename tag<Pointlike>::type
>
struct line_interpolate_point
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


template <typename Geometry, typename Pointlike>
struct line_interpolate_point<Geometry, Pointlike, linestring_tag, point_tag>
    : detail::interpolate_point::range
        <
            detail::interpolate_point::convert_and_assign
        >
{};

template <typename Geometry, typename Pointlike>
struct line_interpolate_point<Geometry, Pointlike, linestring_tag, multi_point_tag>
    : detail::interpolate_point::range
        <
            detail::interpolate_point::convert_and_push_back
        >
{};

template <typename Geometry, typename Pointlike>
struct line_interpolate_point<Geometry, Pointlike, segment_tag, point_tag>
    : detail::interpolate_point::segment
        <
            detail::interpolate_point::convert_and_assign
        >
{};

template <typename Geometry, typename Pointlike>
struct line_interpolate_point<Geometry, Pointlike, segment_tag, multi_point_tag>
    : detail::interpolate_point::segment
        <
            detail::interpolate_point::convert_and_push_back
        >
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy {

struct line_interpolate_point
{
    template
    <
        typename Geometry,
        typename Distance,
        typename Pointlike,
        typename Strategy
    >
    static inline void apply(Geometry const& geometry,
                             Distance const& max_distance,
                             Pointlike & pointlike,
                             Strategy const& strategy)
    {
        dispatch::line_interpolate_point<Geometry, Pointlike>::apply(geometry,
                                                                     max_distance,
                                                                     pointlike,
                                                                     strategy);
    }

    template <typename Geometry, typename Distance, typename Pointlike>
    static inline void apply(Geometry const& geometry,
                             Distance const& max_distance,
                             Pointlike & pointlike,
                             default_strategy)
    {        
        typedef typename strategy::line_interpolate_point::services::default_strategy
            <
                typename cs_tag<Geometry>::type
            >::type strategy_type;

        dispatch::line_interpolate_point<Geometry, Pointlike>::apply(geometry,
                                                                    max_distance,
                                                                    pointlike,
                                                                    strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant {

template <typename Geometry>
struct line_interpolate_point
{
    template <typename Distance, typename Pointlike, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             Distance const& max_distance,
                             Pointlike & pointlike,
                             Strategy const& strategy)
    {
        return resolve_strategy::line_interpolate_point::apply(geometry,
                                                               max_distance,
                                                               pointlike,
                                                               strategy);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct line_interpolate_point<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Pointlike, typename Strategy>
    struct visitor: boost::static_visitor<void>
    {
        Pointlike const& m_pointlike;
        Strategy const& m_strategy;

        visitor(Pointlike const& pointlike, Strategy const& strategy)
            : m_pointlike(pointlike)
            , m_strategy(strategy)
        {}

        template <typename Geometry, typename Distance>
        void operator()(Geometry const& geometry, Distance const& max_distance) const
        {
            line_interpolate_point<Geometry>::apply(geometry, max_distance,
                                                    m_pointlike, m_strategy);
        }
    };

    template <typename Distance, typename Pointlike, typename Strategy>
    static inline void
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
          double const& max_distance,
          Pointlike & pointlike,
          Strategy const& strategy)
    {
        boost::apply_visitor(
            visitor<Pointlike, Strategy>(pointlike, strategy),
            geometry,
            max_distance
        );
    }
};

} // namespace resolve_variant

/*!
\brief 	Returns one or more points interpolated along a LineString \brief_strategy
\ingroup line_interpolate_point
\tparam Geometry Any type fulfilling a LineString concept
\tparam Distance A numerical distance measure
\tparam Pointlike Any type fulfilling Point or Multipoint concept
\tparam Strategy A type fulfilling a LineInterpolatePointStrategy concept
\param geometry Input geometry
\param max_distance Distance threshold (in units depending on coordinate system)
representing the spacing between the points
\param pointlike Output: either a Point (exactly one point will be constructed) or
a MultiPoint (depending on the max_distance one or more points will be constructed)
\param strategy Line_interpolate_point strategy to be used for interpolation of
points

\qbk{[include reference/algorithms/line_interpolate_point.qbk]}

\qbk{distinguish,with strategy}

\qbk{
[heading Available Strategies]
\* [link geometry.reference.strategies.strategy_line_interpolate_point_cartesian Cartesian]
\* [link geometry.reference.strategies.strategy_line_interpolate_point_spherical Spherical]
\* [link geometry.reference.strategies.strategy_line_interpolate_point_geographic Geographic]

[heading Example]
[line_interpolate_point_strategy]
[line_interpolate_point_strategy_output]
}
 */
template
<
    typename Geometry,
    typename Distance,
    typename Pointlike,
    typename Strategy
>
inline void line_interpolate_point(Geometry const& geometry,
                                   Distance const& max_distance,
                                   Pointlike & pointlike,
                                   Strategy const& strategy)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, max_distance, pointlike, strategy);
}


/*!
\brief 	Returns one or more points interpolated along a LineString.
\ingroup line_interpolate_point
\tparam Geometry Any type fulfilling a LineString concept
\tparam Distance A numerical distance measure
\tparam Pointlike Any type fulfilling Point or Multipoint concept
\param geometry Input geometry
\param max_distance Distance threshold (in units depending on coordinate system)
representing the spacing between the points
\param pointlike Output: either a Point (exactly one point will be constructed) or
a MultiPoint (depending on the max_distance one or more points will be constructed)

\qbk{[include reference/algorithms/line_interpolate_point.qbk]

[heading Example]
[line_interpolate_point]
[line_interpolate_point_output]
}
 */
template<typename Geometry, typename Distance, typename Pointlike>
inline void line_interpolate_point(Geometry const& geometry,
                                   Distance const& max_distance,
                                   Pointlike & pointlike)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, max_distance, pointlike, default_strategy());
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_LINE_INTERPOLATE_POINT_HPP
