// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_STRATEGIES_CARTESIAN_CENTROID_AVERAGE_HPP
#define BOOST_GEOMETRY_MULTI_STRATEGIES_CARTESIAN_CENTROID_AVERAGE_HPP


#include <boost/numeric/conversion/cast.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/strategies/centroid.hpp>
#include <boost/geometry/util/copy.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace centroid
{


template
<
    typename PointCentroid,
    typename Point = PointCentroid
>
class centroid_average
{
private :

    /*! subclass to keep state */
    class sum
    {
        friend class centroid_average;
        int count;
        PointCentroid centroid;

    public :
        inline sum()
            : count(0)
        {
            assign_zero(centroid);
        }
    };

public :
    typedef sum state_type;
    typedef PointCentroid centroid_point_type;
    typedef Point point_type;

    static inline void apply(Point const& p, sum& state)
    {
        add_point(state.centroid, p);
        state.count++;
    }

    static inline void result(sum const& state, PointCentroid& centroid)
    {
        centroid = state.centroid;
        divide_value(centroid, state.count);
    }

};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


namespace services
{

template <typename Point, typename Geometry>
struct default_strategy<cartesian_tag, multi_point_tag, 2, Point, Geometry>
{
    typedef centroid_average
        <
            Point,
            typename point_type<Geometry>::type
        > type;
};

} // namespace services

#endif


}} // namespace strategy::centroid


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_STRATEGIES_CARTESIAN_CENTROID_AVERAGE_HPP
