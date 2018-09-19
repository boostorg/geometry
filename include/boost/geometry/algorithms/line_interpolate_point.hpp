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
#include <boost/geometry/strategies/line_interpolate_point.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace interpolate_point
{


template<typename Segment>
struct segment
{
    template <typename Point, typename Strategy>
    static inline void apply(Segment const& segment,
                             double const fraction,
                             Point & point,
                             Strategy const& strategy)
    {
        boost::ignore_unused(strategy);
        typedef typename point_type<Segment>::type point_type;
        point_type p1, p2;
        geometry::detail::assign_point_from_index<0>(segment, p1);
        geometry::detail::assign_point_from_index<1>(segment, p2);
        strategy.apply(p1, p2, fraction, point);
    }
};

/*!
\brief Internal, calculates interpolation point of a linestring using iterator pairs and
    specified strategy
\note for_each could be used here, now that point_type is changed by boost
    range iterator
*/
template<typename Range, closure_selector Closure>
struct range
{
    template <typename Point, typename Strategy>
    static inline void apply(Range const& range,
                             double const fraction,
                             Point & point,
                             Strategy const& strategy)
    {
        typedef typename boost::range_iterator<Range const>::type iterator_t;
        typedef typename boost::range_value<Range const>::type point_t;

        iterator_t it = boost::begin(range);
        iterator_t end = boost::end(range);

        if (it == end) // empty(range)
        {
            return;
        }
            
        //push_back_policy<MutRng> policy(rng_out);

        double tot_len = geometry::length(range);
        double prev_fraction = 0;

        iterator_t prev = it;
        for ( ++it ; it != end; prev = it++)
        {
            point_t const& p0 = *prev;
            point_t const& p1 = *it;

            double seg_fraction = strategy.get_distance_pp_strategy().apply(p0, p1) / tot_len;
            double cur_fraction = prev_fraction + seg_fraction;
            if (cur_fraction >= fraction)
            {
                double new_fraction = (fraction - prev_fraction) / seg_fraction;
                strategy.apply(p0, p1, new_fraction, point);
                break;

            }
            prev_fraction = cur_fraction;
        }
    }
};


}} // namespace detail::length
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct line_interpolate_point
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};


template <typename Geometry>
struct line_interpolate_point<Geometry, linestring_tag>
    : detail::interpolate_point::range<Geometry, closed>
{};


template <typename Geometry>
struct line_interpolate_point<Geometry, segment_tag>
    : detail::interpolate_point::segment<Geometry>
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
    template <typename Geometry, typename Point, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             double const fraction,
                             Point & point,
                             Strategy const& strategy)
    {
        dispatch::line_interpolate_point<Geometry>::apply(geometry,
                                                          fraction,
                                                          point,
                                                          strategy);
    }

    template <typename Geometry, typename Point>
    static inline void apply(Geometry const& geometry,
                             double const fraction,
                             Point & point,
                             default_strategy)
    {        
        typedef typename strategy::line_interpolate_point::services::default_strategy
            <
                typename cs_tag<Geometry>::type
            >::type strategy_type;

        dispatch::line_interpolate_point<Geometry>::apply(geometry,
                                                          fraction,
                                                          point,
                                                          strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant {

template <typename Geometry>
struct line_interpolate_point
{
    template <typename Point, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             double const fraction,
                             Point & point,
                             Strategy const& strategy)
    {
        return resolve_strategy::line_interpolate_point::apply(geometry,
                                                               fraction,
                                                               point,
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
template<typename Geometry, typename Point>
inline void line_interpolate_point(Geometry const& geometry,
                                   double const fraction,
                                   Point & point)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, fraction, point, default_strategy());
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
template<typename Geometry, typename Point, typename Strategy>
inline void line_interpolate_point(Geometry const& geometry,
                                    double const fraction,
                                    Point & point,
                                    Strategy const& strategy)
{
    concepts::check<Geometry const>();

    // detail::throw_on_empty_input(geometry);

    return resolve_variant::line_interpolate_point<Geometry>
                          ::apply(geometry, fraction, point, strategy);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_LINE_INTERPOLATE_POINT_HPP
