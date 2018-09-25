// Boost.Geometry

// Copyright (c) 2018, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SEGMENT_INTERPOLATE_POINT_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SEGMENT_INTERPOLATE_POINT_HPP

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/srs/spheroid.hpp>
#include <boost/geometry/strategies/segment_interpolate_point.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace segment_interpolate_point
{


/*!
\brief Interpolate point on a spherical segment.
\ingroup strategies.
\tparam CalculationType \tparam_calculation
 */
template
<
    typename CalculationType = void,
    typename Strategy = distance::haversine<double, CalculationType>
>
class spherical
{
public:

    // point-point strategy getters
    struct distance_pp_strategy
    {
        typedef Strategy type;
    };

    inline typename distance_pp_strategy::type get_distance_pp_strategy() const
    {
        return typename distance_pp_strategy::type();
    }

    template <typename Point, typename T>
    inline void apply(Point const& p0,
                      Point const& p1,
                      T const& fraction,
                      Point & p) const
    {
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                CalculationType
            >::type calc_t;

        //TODO: create direct formula for spherical

        calc_t const c0 = 0;
        calc_t const c1 = 1;
        calc_t const pi = math::pi<calc_t>();

        typedef model::point<calc_t, 3, cs::cartesian> point3d_t;
        point3d_t const xyz0 = formula::sph_to_cart3d<point3d_t>(p0);
        point3d_t const xyz1 = formula::sph_to_cart3d<point3d_t>(p1);
        calc_t const dot01 = geometry::dot_product(xyz0, xyz1);
        calc_t const angle01 = acos(dot01);

        point3d_t axis;
        if (! math::equals(angle01, pi))
        {
            axis = geometry::cross_product(xyz0, xyz1);
            geometry::detail::vec_normalize(axis);
        }
        else // antipodal
        {
            calc_t const half_pi = math::half_pi<calc_t>();
            calc_t const lat = geometry::get_as_radian<1>(p0);

            if (math::equals(lat, half_pi))
            {
                // pointing east, segment lies on prime meridian, going south
                axis = point3d_t(c0, c1, c0);
            }
            else if (math::equals(lat, -half_pi))
            {
                // pointing west, segment lies on prime meridian, going north
                axis = point3d_t(c0, -c1, c0);
            }
            else
            {
                // lon rotated west by pi/2 at equator
                calc_t const lon = geometry::get_as_radian<0>(p0);
                axis = point3d_t(sin(lon), -cos(lon), c0);
            }
        }

        calc_t a = angle01 * fraction;

        // Axis-Angle rotation
        // see: https://en.wikipedia.org/wiki/Axis-angle_representation
        calc_t const cos_a = cos(a);
        calc_t const sin_a = sin(a);
        // cos_a * v
        point3d_t s1 = xyz0;
        geometry::multiply_value(s1, cos_a);
        // sin_a * (n x v)
        point3d_t s2 = geometry::cross_product(axis, xyz0);
        geometry::multiply_value(s2, sin_a);
        // (1 - cos_a)(n.v) * n
        point3d_t s3 = axis;
        geometry::multiply_value(s3, (c1 - cos_a) * geometry::dot_product(axis, xyz0));
        // v_rot = cos_a * v + sin_a * (n x v) + (1 - cos_a)(n.v) * e
        point3d_t v_rot = s1;
        geometry::add_point(v_rot, s2);
        geometry::add_point(v_rot, s3);

        p = formula::cart3d_to_sph<Point>(v_rot);

    }

};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<spherical_equatorial_tag>
{
    typedef strategy::segment_interpolate_point::spherical<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::segment_interpolate_point


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_SEGMENT_INTERPOLATE_POINT_HPP
