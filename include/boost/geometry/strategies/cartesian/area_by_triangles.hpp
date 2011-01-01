// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_AREA_BY_TRIANGLES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_AREA_BY_TRIANGLES_HPP


#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/geometries/point_xy.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace area
{

/*!
\brief Strategy functor for area calculation on point_xy points
\ingroup strategies
\details Calculates area using well-known triangulation algorithm
\tparam PointOfSegment point type of segments
*/
template
<
    typename PointOfSegment,
    typename CalculationType = void
>
class by_triangles
{
public :
    // If user specified a calculation type, use that type,
    //   whatever it is and whatever the point-type is.
    // Else, use the pointtype, but at least double
    typedef typename
        boost::mpl::if_c
        <
            boost::is_void<CalculationType>::type::value,
            typename select_most_precise
            <
                typename coordinate_type<PointOfSegment>::type,
                double
            >::type,
            CalculationType
        >::type return_type;


private :

    class summation
    {
        friend class by_triangles;

        return_type sum;
    public :

        inline summation() : sum(return_type())
        {
            // Strategy supports only 2D areas
            assert_dimension<PointOfSegment, 2>();
        }
        inline return_type area() const
        {
            return_type result = sum;
            result *= 0.5;
            return result;
        }
    };

public :
    typedef summation state_type;
    typedef PointOfSegment segment_point_type;

    static inline void apply(PointOfSegment const& p1,
                PointOfSegment const& p2,
                summation& state)
    {
        // SUM += x2 * y1 - x1 * y2;
        state.sum += get<0>(p2) * get<1>(p1) - get<0>(p1) * get<1>(p2);
    }

    static inline return_type result(summation const& state)
    {
        return state.area();
    }

};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{
    template <typename Point>
    struct default_strategy<cartesian_tag, Point>
    {
        typedef strategy::area::by_triangles<Point> type;
    };

} // namespace services

#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::area



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_AREA_BY_TRIANGLES_HPP
