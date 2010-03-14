// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DISSOLVE_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DISSOLVE_HPP


#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/algorithms/dissolve.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace dissolve
{

template <typename Multi, typename MultiOut>
struct dissolve_multi
{
	template <typename OutputIterator>
    static inline OutputIterator apply(Multi const& multi, OutputIterator out)
    {
		typedef typename boost::range_value<Multi>::type polygon_type;
		typedef typename boost::range_value<MultiOut>::type output_polygon_type;
		typedef typename boost::range_iterator<Multi const>::type iterator_type;
		
		// Step 1: dissolve all polygons in the multi-polygon, independantly
		MultiOut step1;
        for (iterator_type it = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
			dissolve_ring_or_polygon
				<
					polygon_type,
					output_polygon_type
				>::apply(*it, std::back_inserter(step1));
		}

		// Step 2: remove mutual overlap
		// TODO: solve quadratic behaviour by alternating division in x/y axis
		// per division there are 3 cases: cases above the line, cases below the line, cases crossing the line
		// recursively handle those 3 cases and union them.
		MultiOut step2;
        for (iterator_type it = boost::begin(step1);
            it != boost::end(step1);
            ++it)
        {
			if (step2.empty())
			{
				step2.push_back(*it);
			}
			else
			{
				MultiOut unioned;
				for (iterator_type it2 = boost::begin(step2);
					it2 != boost::end(step2);
					++it2)
				{
					geometry::union_inserter
						<
							output_polygon_type
						>(*it2, *it, std::back_inserter(unioned));
				}
				step2.swap(unioned);
			}
		}

		// Step 3: output
		*out++ = step2;
		return out;
	}
};


}} // namespace detail::dissolve
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template<typename Multi, typename MultiOut>
struct dissolve<multi_polygon_tag, multi_polygon_tag, Multi, MultiOut>
    : detail::dissolve::dissolve_multi<Multi, MultiOut>
{};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DISSOLVE_HPP
