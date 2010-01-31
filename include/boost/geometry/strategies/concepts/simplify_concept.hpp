// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CONCEPTS_SIMPLIFY_CONCEPT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CONCEPTS_SIMPLIFY_CONCEPT_HPP

#include <vector>
#include <iterator>

#include <boost/concept_check.hpp>

#include <boost/geometry/strategies/concepts/distance_concept.hpp>


namespace boost { namespace geometry { namespace concept {


/*!
    \brief Checks strategy for simplify
    \ingroup simplify
*/
template <typename Strategy>
struct SimplifyStrategy
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    private :

        // 1) must define distance_strategy_type,
        //    defining point-segment distance strategy (to be checked)
        typedef typename Strategy::distance_strategy_type ds_type;

        BOOST_CONCEPT_ASSERT
            (
                (concept::PointSegmentDistanceStrategy<ds_type>)
            );

        struct apply_checker
        {
            static void check()
            {
                Strategy *str;
                std::vector<typename ds_type::point_type> const* v1;
                std::vector<typename ds_type::point_type> * v2;

                // 2) must implement method apply with arguments
                //    - Range
                //    - OutputIterator
                //    - floating point value
                str->apply(*v1, std::back_inserter(*v2), 1.0);

                boost::ignore_unused_variable_warning(str);
            }
        };

    public :
        BOOST_CONCEPT_USAGE(SimplifyStrategy)
        {
            apply_checker::check();

        }
#endif
};



}}} // namespace boost::geometry::concept

#endif // BOOST_GEOMETRY_STRATEGIES_CONCEPTS_SIMPLIFY_CONCEPT_HPP
