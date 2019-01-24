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
    template <typename Range, typename PointType, typename Strategy>
    static inline void apply(Range const& range,
                             double const& fraction,
                             PointType & p_or_mp,
                             Strategy const& strategy)
    {
        Policy policy;

        typedef typename boost::range_iterator<Range const>::type iterator_t;
        typedef typename boost::range_value<Range const>::type point_t;

        typedef typename select_most_precise
            <
                typename default_length_result<Range>::type,
                double
            >::type calc_t;

        typedef typename Strategy::template result_type<point_t> result_type;
        typedef boost::container::vector<result_type> vector_t;
        typedef typename boost::range_iterator<vector_t const>::type viterator_t;

        iterator_t it = boost::begin(range);
        iterator_t end = boost::end(range);

        if (it == end) // empty(range)
        {
            return;
        }
        if (fraction == 0)
        {
            policy.apply(*it, p_or_mp);
            return;
        }
        if (fraction == 1)
        {
            policy.apply(*(end-1), p_or_mp);
            return;
        }
            
        // compute lengths of segments and their sum
        calc_t tot_len = 0;
        vector_t lengths;
        iterator_t prev = it++;
        do {
            result_type len = strategy.compute(*prev, *it);
            lengths.push_back(len);
            tot_len += len.distance;
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
                policy.apply(p, p_or_mp);

                repeated_fraction = ++i * fraction;
            }

            prev_fraction = cur_fraction;
            prev = it++;
            vit++;

        } while (it != end && !single_point);
    }
};

template <typename Policy>
struct segment
{
    template <typename Segment, typename PointType, typename Strategy>
    static inline void apply(Segment const& segment,
                             double const& fraction,
                             PointType & point,
                             Strategy const& strategy)
    {
        range<Policy>().apply(segment_view<Segment>(segment),
                              fraction, point, strategy);
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
    typename PointType,
    typename Tag1 = typename tag<Geometry>::type,
    typename Tag2 = typename tag<PointType>::type
>
struct line_interpolate_point
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, linestring_tag, point_tag>
    : detail::interpolate_point::range
        <
            detail::interpolate_point::convert_and_assign
        >
{};

template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, linestring_tag, multi_point_tag>
    : detail::interpolate_point::range
        <
            detail::interpolate_point::convert_and_push_back
        >
{};

template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, segment_tag, point_tag>
    : detail::interpolate_point::segment
        <
            detail::interpolate_point::convert_and_assign
        >
{};

template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, segment_tag, multi_point_tag>
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
    template <typename Geometry, typename PointType, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             double const& fraction,
                             PointType & mp,
                             Strategy const& strategy)
    {
        dispatch::line_interpolate_point<Geometry, PointType>::apply(geometry,
                                                                     fraction,
                                                                     mp,
                                                                     strategy);
    }

    template <typename Geometry, typename PointType>
    static inline void apply(Geometry const& geometry,
                             double const& fraction,
                             PointType & mp,
                             default_strategy)
    {        
        typedef typename strategy::line_interpolate_point::services::default_strategy
            <
                typename cs_tag<Geometry>::type
            >::type strategy_type;

        dispatch::line_interpolate_point<Geometry, PointType>::apply(geometry,
                                                                    fraction,
                                                                    mp,
                                                                    strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant {

template <typename Geometry>
struct line_interpolate_point
{
    template <typename PointType, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             double const& fraction,
                             PointType & mp,
                             Strategy const& strategy)
    {
        return resolve_strategy::line_interpolate_point::apply(geometry,
                                                               fraction,
                                                               mp,
                                                               strategy);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct line_interpolate_point<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename PointType, typename Strategy>
    struct visitor: boost::static_visitor<void>
    {
        PointType const& m_mp;
        Strategy const& m_strategy;

        visitor(PointType const& mp, Strategy const& strategy)
            : m_mp(mp)
            , m_strategy(strategy)
        {}

        template <typename Geometry>
        void operator()(Geometry const& geometry, double const& fraction) const
        {
            line_interpolate_point<Geometry>::apply(geometry, fraction,
                                                    m_mp, m_strategy);
        }
    };

    template <typename PointType, typename Strategy>
    static inline void
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
          double const& fraction,
          PointType & mp,
          Strategy const& strategy)
    {
        boost::apply_visitor(
            visitor<PointType, Strategy>(mp, strategy),
            geometry,
            fraction
        );
    }
};

} // namespace resolve_variant

/*!
\brief 	Returns one or more points interpolated along a LineString \brief_strategy
\ingroup line_interpolate_point
\tparam Geometry Any type fulfilling a LineString concept
\tparam PointType Any type fulfilling Point or Multipoint concept
\tparam Strategy A type fulfilling a LineInterpolatePointStrategy concept
\param geometry Input geometry
\param fraction Number between 0 and 1 representing the spacing between the
points as a fraction of total LineString length
\param mp Output: either a Point (exactly one point will be constructed) or
a MultiPoint (depending on the fraction one or more points will be constructed)
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
template<typename Geometry, typename PointType, typename Strategy>
inline void line_interpolate_point(Geometry const& geometry,
                                   double const& fraction,
                                   PointType & mp,
                                   Strategy const& strategy)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, fraction, mp, strategy);
}


/*!
\brief 	Returns one or more points interpolated along a LineString.
\ingroup line_interpolate_point
\tparam Geometry Any type fulfilling a LineString concept
\tparam PointType Any type fulfilling Point or Multipoint concept
\param geometry Input geometry
\param fraction Number between 0 and 1 representing the spacing between the
points as a fraction of total LineString length
\param mp Output: either a Point (exactly one point will be constructed) or
a MultiPoint (depending on the fraction one or more points will be constructed)

\qbk{[include reference/algorithms/line_interpolate_point.qbk]

[heading Example]
[line_interpolate_point]
[line_interpolate_point_output]
}
 */
template<typename Geometry, typename PointType>
inline void line_interpolate_point(Geometry const& geometry,
                                   double const& fraction,
                                   PointType & mp)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, fraction, mp, default_strategy());
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_LINE_INTERPOLATE_POINT_HPP
