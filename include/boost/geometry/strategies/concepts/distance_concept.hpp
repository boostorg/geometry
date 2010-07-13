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

#include <boost/type_traits.hpp>

#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/plus.hpp>

#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>

//#define RMP
#ifdef RMP
#include <boost/remove_member_pointer.hpp>
#endif


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

    struct checker
    {
        template <typename ApplyMethod>
        static void check(ApplyMethod const&)
        {

    #ifdef RMP
            typedef boost::function_traits
                <
                    typename boost::remove_pointer
                        <
                            typename boost::remove_member_pointer<ApplyMethod>::type
                        >::type
                > ftraits;
                
            typedef typename boost::remove_const
                <
                    typename boost::remove_reference
                        <
                            typename ftraits::arg1_type
                        >::type
                >::type ptype1;
            typedef typename boost::remove_const
                <
                    typename boost::remove_reference
                        <
                            typename ftraits::arg2_type
                        >::type
                >::type ptype2;
    #else

            typedef typename boost::function_types::parameter_types<ApplyMethod>::type typeseq;

            typedef typename boost::mpl::if_
                <
                    boost::function_types::is_member_function_pointer<ApplyMethod>,
                    boost::mpl::int_<1>,
                    boost::mpl::int_<0>
                >::type base_index;

            typedef typename boost::remove_const
                <
                    typename boost::remove_reference
                        <
                            typename boost::mpl::at
                                <
                                    typeseq, 
                                    base_index
                                >::type
                        >::type
                >::type ptype1;


            typedef typename boost::remove_const
                <
                    typename boost::remove_reference
                        <
                            typename boost::mpl::at
                                <
                                    typeseq, 
                                    typename boost::mpl::plus
                                        <
                                            base_index, 
                                            boost::mpl::int_<1> 
                                        >::type
                                >::type
                        >::type
                >::type ptype2;
    #endif

            BOOST_CONCEPT_ASSERT
                (
                    (concept::ConstPoint<ptype1>)
                );

            BOOST_CONCEPT_ASSERT
                (
                    (concept::ConstPoint<ptype2>)
                );


            // 1) must define meta-function return_type
            typedef typename strategy::distance::services::return_type<Strategy>::type rtype;

            // 2) must define meta-function "similar_type"
            typedef typename strategy::distance::services::similar_type
                <
                    Strategy, ptype2, ptype1
                >::type stype;

            // 3) must define meta-function "comparable_type"
            typedef typename strategy::distance::services::comparable_type
                <
                    Strategy
                >::type ctype;

            // 4) must define meta-function "tag"
            typedef typename strategy::distance::services::tag
                <
                    Strategy
                >::type tag;

            // 5) must implement apply with arguments
            Strategy* str;
            ptype1 *p1;
            ptype2 *p2;
            rtype r = str->apply(*p1, *p2);

            // 6) must define (meta)struct "get_similar" with apply
            stype s = strategy::distance::services::get_similar
                <
                    Strategy,
                    ptype2, ptype1
                >::apply(*str);

            // 7) must define (meta)struct "get_comparable" with apply
            ctype c = strategy::distance::services::get_comparable
                <
                    Strategy
                >::apply(*str);

            // 8) must define (meta)struct "result_from_distance" with apply
            r = strategy::distance::services::result_from_distance
                <
                    Strategy
                >::apply(*str, 1.0);

            boost::ignore_unused_variable_warning(str);
            boost::ignore_unused_variable_warning(s);
            boost::ignore_unused_variable_warning(c);
            boost::ignore_unused_variable_warning(r);
        }
    };



public :
    BOOST_CONCEPT_USAGE(PointDistanceStrategy)
    {
        checker::check(&Strategy::apply);
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

        // 3) must define meta-function return_type
        typedef typename strategy::distance::services::return_type<Strategy>::type rtype;

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
