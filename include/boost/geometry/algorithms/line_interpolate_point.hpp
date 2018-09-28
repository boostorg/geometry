// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018 Oracle and/or its affiliates.

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
#include <boost/geometry/strategies/segment_interpolate_point.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace interpolate_point
{

template <typename Range, typename Point>
inline void convert_and_push_back(Range & range, Point const& p)
{
    typename boost::range_value<Range>::type p2;
    geometry::detail::conversion::convert_point_to_point(p, p2);
    range::push_back(range, p2);
}

template <bool repeat>
struct segment
{
    template <typename Segment, typename PointType, typename Strategy>
    static inline void apply(Segment const& segment,
                             double const fraction,
                             PointType & point,
                             Strategy const& strategy)
    {
        boost::ignore_unused(strategy);
        typedef typename point_type<Segment>::type point_type;
        point_type p0, p1;
        geometry::detail::assign_point_from_index<0>(segment, p0);
        geometry::detail::assign_point_from_index<1>(segment, p1);
        if (fraction == 0)
        {
            geometry::detail::conversion::convert_point_to_point(p0, point);
            return;
        }
        if (fraction == 1)
        {
            geometry::detail::conversion::convert_point_to_point(p1, point);
            return;
        }
/*
        while (cur_fraction >= frac)
        {
            strategy.apply(p0, p1,
                           (frac - prev_fraction) / seg_fraction,
                           point);

            std::cout << "(" << get<0>(point) << "," << get<1>(point) << ")";
            if (!repeat) break;
            frac += fraction;
        }
*/
        strategy.apply(p0, p1, fraction, point);
    }
};

/*!
\brief Internal, calculates interpolation point of a linestring using iterator pairs and
    specified strategy
\note for_each could be used here, now that point_type is changed by boost
    range iterator
*/
template <bool repeat>
struct range
{
    template <typename Range, typename PointType, typename Strategy>
    static inline void apply(Range const& range,
                             double const fraction,
                             PointType & point,
                             Strategy const& strategy)
    {
        typedef typename boost::range_iterator<Range const>::type iterator_t;
        typedef typename boost::range_value<Range const>::type point_t;

        typedef typename select_most_precise
            <
                typename default_length_result<Range>::type,
                double
            >::type calc_t;

        iterator_t it = boost::begin(range);
        iterator_t end = boost::end(range);

        if (it == end) // empty(range)
        {
            return;
        }
        if (fraction == 0)
        {
            geometry::detail::conversion::convert_point_to_point(*it, point);
            return;
        }
        if (fraction == 1)
        {
            geometry::detail::conversion::convert_point_to_point(*(end-1), point);
            return;
        }
            
        calc_t frac = fraction;
        calc_t tot_len = geometry::length(range);
        calc_t prev_fraction = 0;
        calc_t cur_fraction = 0;

        int num_of_points = 0;

        iterator_t prev = it++;
        do {
            point_t const& p0 = *prev;
            point_t const& p1 = *it;

            calc_t seg_fraction = strategy.get_distance_pp_strategy().apply(p0, p1)
                                / tot_len;
            cur_fraction = prev_fraction + seg_fraction;
/*
            strategy.apply(p0, p1,
                           (fraction - prev_fraction) / seg_fraction,
                           point);

            prev_fraction = cur_fraction;
            prev = it++;
 */
            while (cur_fraction >= frac)
            {
                num_of_points++;
                strategy.apply(p0, p1,
                               (frac - prev_fraction) / seg_fraction,
                               point);

                std::cout << "(" << get<0>(point) << "," << get<1>(point) << ")";
                //if (!repeat) break;
                frac += fraction;
            }
            //if (!repeat) break;
            std::cout << "[]";
            prev_fraction = cur_fraction;
            prev = it++;
        //} while (cur_fraction < fraction && it != end);
        } while (it != end);
        if (fraction * (num_of_points + 1) == 1.0)
        {
            std::cout << "(" << get<0>(*(end-1)) << "," << get<1>(*(end-1)) << ")";
        }
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
    : detail::interpolate_point::range<false>
{};


template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, linestring_tag, multi_point_tag>
    : detail::interpolate_point::range<true>
{};


template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, segment_tag, point_tag>
    : detail::interpolate_point::segment<false>
{};


template <typename Geometry, typename PointType>
struct line_interpolate_point<Geometry, PointType, segment_tag, multi_point_tag>
    : detail::interpolate_point::segment<true>
{};

/*
template <typename MultiLinestring>
struct length<MultiLinestring, multi_linestring_tag> : detail::multi_sum
{
    template <typename Strategy>
    static inline typename default_length_result<MultiLinestring>::type
    apply(MultiLinestring const& multi, Strategy const& strategy)
    {
        return multi_sum::apply
               <
                   typename default_length_result<MultiLinestring>::type,
                   detail::length::range_length
                   <
                       typename boost::range_value<MultiLinestring>::type,
                       closed // no need to close it explicitly
                   >
               >(multi, strategy);

    }
};
*/

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy {

struct line_interpolate_point
{
    template <typename Geometry, typename PointType, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             double const fraction,
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
                             double const fraction,
                             PointType & mp,
                             default_strategy)
    {        
        typedef typename strategy::segment_interpolate_point::services::default_strategy
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
                             double const fraction,
                             PointType & mp,
                             Strategy const& strategy)
    {
        return resolve_strategy::line_interpolate_point::apply(geometry,
                                                               fraction,
                                                               mp,
                                                               strategy);
    }
};

/*
template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct length<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Strategy>
    struct visitor
        : static_visitor<result_type>
    {
        Strategy const& m_strategy;

        visitor(Strategy const& strategy)
            : m_strategy(strategy)
        {}

        template <typename Geometry>
        inline typename default_length_result<Geometry>::type
        operator()(Geometry const& geometry) const
        {
            return length<Geometry>::apply(geometry, m_strategy);
        }
    };

    template <typename Strategy>
    static inline result_type apply(
        variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
        Strategy const& strategy
    )
    {
        return boost::apply_visitor(visitor<Strategy>(strategy), geometry);
    }
};
*/
} // namespace resolve_variant


/*!
\brief \brief_calc{line_interpolate_point}
\ingroup line_interpolate_point
\details \details_calc{line_interpolate_point, line_interpolate_point (Returns a
         point interpolated along a line)}. \details_default_strategy
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\param fraction \param_fraction
\return \return_calc{line_interpolate_point}

\qbk{[include reference/algorithms/line_interpolate_point.qbk]}
\qbk{[line_interpolate_point] [line_interpolate_point_output]}
 */
template<typename Geometry, typename PointType>
inline void line_interpolate_point(Geometry const& geometry,
                                   double const fraction,
                                   PointType & mp)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, fraction, mp, default_strategy());
}


/*!
\brief \brief_calc{line_interpolate_point} \brief_strategy
\ingroup line_interpolate_point
\details \details_calc{line_interpolate_point, line_interpolate_point (Returns a
         point interpolated along a line)} \brief_strategy. \details_strategy_reasons
\tparam Geometry \tparam_geometry
\tparam Strategy \tparam_strategy{distance}
\param geometry \param_geometry
\param fraction \param_fraction
\param strategy \param_strategy{distance}
\return \return_calc{line_interpolate_point}

\qbk{distinguish,with strategy}
\qbk{[include reference/algorithms/line_interpolate_point.qbk]}
\qbk{[line_interpolate_point_with_strategy] [line_interpolate_point_with_strategy_output]}
 */
template<typename Geometry, typename PointType, typename Strategy>
inline void line_interpolate_point(Geometry const& geometry,
                                   double const fraction,
                                   PointType & mp,
                                   Strategy const& strategy)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, fraction, mp, strategy);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_LINE_INTERPOLATE_POINT_HPP
