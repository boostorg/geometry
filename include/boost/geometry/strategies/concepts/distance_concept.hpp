// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CONCEPTS_DISTANCE_CONCEPT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CONCEPTS_DISTANCE_CONCEPT_HPP

#include <vector>
#include <iterator>

#include <boost/concept_check.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/segment.hpp>


namespace boost { namespace geometry { namespace concept
{


/*!
    \brief Checks strategy for point-segment-distance
    \ingroup distance
*/
template <typename Strategy>
struct PointDistanceStrategy
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    private :

        // 1) must define first_point_type
        typedef typename Strategy::first_point_type ptype1;
        BOOST_CONCEPT_ASSERT
            (
                (concept::ConstPoint<ptype1>)
            );

        // 2) must define second_point_type
        typedef typename Strategy::second_point_type ptype2;
        BOOST_CONCEPT_ASSERT
            (
                (concept::ConstPoint<ptype2>)
            );

        // 3) must define return_type
        typedef typename Strategy::return_type rtype;

        // 4) must implement apply with arguments
        struct apply_checker
        {
            static void check()
            {
                Strategy* str;
                ptype1 *p1;
                ptype2 *p2;
                rtype r = str->apply(*p1, *p2);

                boost::ignore_unused_variable_warning(str);
                boost::ignore_unused_variable_warning(r);
            }
        };

        // 5) must define meta-function "similar_type"
        typedef typename strategy::distance::services::similar_type
            <
                Strategy, ptype2, ptype1
            >::type stype;

        // 6) must define meta-function "comparable_type"
        typedef typename strategy::distance::services::comparable_type
            <
                Strategy
            >::type ctype;

        // 6) must define meta-function "tag"
        typedef typename strategy::distance::services::tag
            <
                Strategy
            >::type tag;


        // 7) must define (meta)struct "get_similar" with apply
        // 8) must define (meta)struct "get_comparable" with apply
        // 9) must define (meta)struct "result_from_distance" with apply
        struct services_checker
        {
            static void check()
            {
                Strategy* str;
                stype s = strategy::distance::services::get_similar
                    <
                        Strategy,
                        ptype2, ptype1
                    >::apply(*str);
                ctype c = strategy::distance::services::get_comparable
                    <
                        Strategy
                    >::apply(*str);
                rtype r = strategy::distance::services::result_from_distance
                    <
                        Strategy
                    >::apply(*str, 1.0);

                boost::ignore_unused_variable_warning(s);
                boost::ignore_unused_variable_warning(c);
            }
        };


    public :
        BOOST_CONCEPT_USAGE(PointDistanceStrategy)
        {
            apply_checker::check();
            services_checker::check();
        }
#endif
};


/*!
    \brief Checks strategy for point-segment-distance
    \ingroup strategy_concepts
*/
template <typename Strategy>
struct PointSegmentDistanceStrategy
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    private :

        // 1) must define point_type
        typedef typename Strategy::point_type ptype;
        BOOST_CONCEPT_ASSERT
            (
                (concept::ConstPoint<ptype>)
            );

        // 2) must define segment_point_type
        typedef typename Strategy::segment_point_type sptype;
        BOOST_CONCEPT_ASSERT
            (
                (concept::ConstPoint<sptype>)
            );

        // 3) must define return_type
        typedef typename Strategy::return_type rtype;

        // 4) must define underlying point-distance-strategy
        typedef typename Strategy::point_strategy_type stype;
        BOOST_CONCEPT_ASSERT
            (
                (concept::PointDistanceStrategy<stype>)
            );


        // 5) must implement method apply with arguments
        struct apply_checker
        {
            static void check()
            {
                Strategy *str;
                ptype *p;
                sptype *sp1;
                sptype *sp2;

                rtype r = str->apply(*p, *sp1, *sp2);

                boost::ignore_unused_variable_warning(str);
                boost::ignore_unused_variable_warning(r);
            }
        };

    public :
        BOOST_CONCEPT_USAGE(PointSegmentDistanceStrategy)
        {
            apply_checker::check();
        }
#endif
};


}}} // namespace boost::geometry::concept


#endif // BOOST_GEOMETRY_STRATEGIES_CONCEPTS_DISTANCE_CONCEPT_HPP
