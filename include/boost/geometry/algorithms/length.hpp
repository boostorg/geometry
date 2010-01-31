// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_LENGTH_HPP
#define BOOST_GEOMETRY_ALGORITHMS_LENGTH_HPP

#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>


#include <boost/geometry/core/cs.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/detail/calculate_null.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/length_result.hpp>

/*!
\defgroup length length: calculate length of a linear geometry
The length algorithm is implemented for the linestring and the multi_linestring
geometry and results in the length of the linestring. If the points of
a linestring have coordinates expressed in kilometers,
the length of the line is expressed in kilometers as well.
\par Example:
Example showing length calculation
\dontinclude doxygen_1.cpp
\skip example_length_linestring_iterators1
\line {
\until }
*/

namespace boost { namespace geometry
{



#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace length {


template<typename Segment, typename Strategy>
struct segment_length
{
    static inline typename length_result<Segment>::type apply(
            Segment const& segment, Strategy const& strategy)
    {
        typedef typename point_type<Segment>::type point_type;
        point_type p1, p2;
        assign_point_from_index<0>(segment, p1);
        assign_point_from_index<1>(segment, p2);
        return strategy.apply(p1, p2);
    }
};

/*!
\brief Internal, calculates length of a linestring using iterator pairs and
    specified strategy
\note for_each could be used here, now that point_type is changed by boost
    range iterator
*/
template<typename Range, typename Strategy>
struct range_length
{
    typedef typename length_result<Range>::type return_type;

    static inline return_type apply(
            Range const& range, Strategy const& strategy)
    {
        return_type sum = return_type();

        typedef typename boost::range_const_iterator<Range>::type iterator_type;
        iterator_type it = boost::begin(range);
        if (it != boost::end(range))
        {
            iterator_type previous = it++;
            while(it != boost::end(range))
            {
                // Add point-point distance using the return type belonging
                // to strategy
                sum += strategy.apply(*previous, *it);
                previous = it++;
            }
        }

        return sum;
    }
};

}} // namespace detail::length
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Tag, typename Geometry, typename Strategy>
struct length : detail::calculate_null
    <
        typename length_result<Geometry>::type,
        Geometry,
        Strategy
    >
{};


template <typename Geometry, typename Strategy>
struct length<linestring_tag, Geometry, Strategy>
    : detail::length::range_length<Geometry, Strategy>
{};


// RING: length is currently 0; it might be argued that it is the "perimeter"


template <typename Geometry, typename Strategy>
struct length<segment_tag, Geometry, Strategy>
    : detail::length::segment_length<Geometry, Strategy>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate length of a geometry
    \ingroup length
    \details The function length returns the length of a geometry, using the
        default distance-calculation-strategy
    \param geometry the geometry, being a geometry::linestring, vector,
        iterator pair, or any other boost compatible range
    \return the length
    Example showing length calculation on a vector
    \dontinclude doxygen_1.cpp
    \skip example_length_linestring_iterators2
    \line {
    \until }
 */
template<typename Geometry>
inline typename length_result<Geometry>::type length(
        Geometry const& geometry)
{
    concept::check<const Geometry>();

    typedef typename point_type<Geometry>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance
        <
            cs_tag,
            cs_tag,
            point_type,
            point_type
        >::type strategy_type;

    return dispatch::length
        <
            typename tag<Geometry>::type,
            Geometry,
            strategy_type
        >::apply(geometry, strategy_type());
}


/*!
    \brief Calculate length of a geometry
    \ingroup length
    \details The function length returns the length of a geometry,
        using specified strategy
    \param geometry the geometry, being a geometry::linestring, vector,
        iterator pair, or any other boost compatible range
    \param strategy strategy to be used for distance calculations.
    \return the length
    \par Example:
    Example showing length calculation using iterators
        and the Vincenty strategy
    \dontinclude doxygen_1.cpp
    \skip example_length_linestring_iterators3
    \line {
    \until }
 */
template<typename Geometry, typename Strategy>
inline typename length_result<Geometry>::type length(
        Geometry const& geometry, Strategy const& strategy)
{
    concept::check<const Geometry>();

    return dispatch::length
        <
            typename tag<Geometry>::type,
            Geometry,
            Strategy
        >::apply(geometry, strategy);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_LENGTH_HPP
