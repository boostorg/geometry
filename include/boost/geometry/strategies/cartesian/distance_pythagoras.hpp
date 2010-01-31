// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISTANCE_PYTHAGORAS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISTANCE_PYTHAGORAS_HPP


#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/core/access.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/distance_result.hpp>

#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/util/copy.hpp>



namespace boost { namespace geometry {

namespace strategy { namespace distance {

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Point1, typename Point2, size_t I, typename T>
struct compute_pythagoras
{
    static inline T apply(Point1 const& p1, Point2 const& p2)
    {
        T const c1 = boost::numeric_cast<T>(get<I-1>(p2));
        T const c2 = boost::numeric_cast<T>(get<I-1>(p1));
        T const d = c1 - c2;
        return d * d + compute_pythagoras<Point1, Point2, I-1, T>::apply(p1, p2);
    }
};

template <typename Point1, typename Point2, typename T>
struct compute_pythagoras<Point1, Point2, 0, T>
{
    static inline T apply(Point1 const&, Point2 const&)
    {
        return boost::numeric_cast<T>(0);
    }
};

}
#endif // DOXYGEN_NO_DETAIL

/*!
    \brief Strategy for distance point to point: pythagoras
    \ingroup distance
    \tparam Point1 first point type
    \tparam Point2 optional, second point type, defaults to first point type
    \par Concepts for Point1 and Point2:
    - specialized point_traits class
*/
template
<
    typename Point1,
    typename Point2 = Point1,
    typename CalculationType = void
>
struct pythagoras
{
    typedef typename select_calculation_type
        <
            Point1,
            Point2,
            CalculationType
        >::type calculation_type;

    typedef Point1 first_point_type;
    typedef Point2 second_point_type;
    typedef cartesian_distance<calculation_type> return_type;

    inline return_type apply(Point1 const& p1, Point2 const& p2) const
    {
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<Point1>) );
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<Point2>) );

        // Calculate distance using Pythagoras
        // (Leave comment above for Doxygen)

        assert_dimension_equal<Point1, Point2>();

        return return_type(detail::compute_pythagoras
            <
                Point1, Point2,
                dimension<Point1>::value,
                calculation_type
            >::apply(p1, p2));
    }
};


}} // namespace strategy::distance


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename Point1, typename Point2>
struct strategy_distance<cartesian_tag, cartesian_tag, Point1, Point2>
{
    typedef strategy::distance::pythagoras<Point1, Point2> type;
};


#endif


template <typename Point1, typename Point2>
struct strategy_tag<strategy::distance::pythagoras<Point1, Point2> >
{
    typedef strategy_tag_distance_point_point type;
};



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISTANCE_PYTHAGORAS_HPP
