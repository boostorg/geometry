// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_MAPPING_SSF_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_MAPPING_SSF_HPP


#include <boost/core/ignore_unused.hpp>

#include <boost/geometry/core/radius.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>

#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/strategies/spherical/ssf.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace side
{


// An enumeration type defining types of mapping of geographical
// latitude to spherical latitude.
// See: http://en.wikipedia.org/wiki/Great_ellipse
//      http://en.wikipedia.org/wiki/Latitude#Auxiliary_latitudes
enum mapping_type { mapping_geodetic, mapping_reduced, mapping_geocentric };


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Spheroid, mapping_type Mapping, typename CalculationType>
struct mapper
{
    explicit inline mapper(Spheroid const& /*spheroid*/) {}

    static inline CalculationType const& map_lat(CalculationType const& lat)
    {
        return lat;
    }
};

template <typename Spheroid, typename CalculationType>
struct mapper<Spheroid, mapping_reduced, CalculationType>
{
    explicit inline mapper(Spheroid const& spheroid)
    {
        CalculationType const a = geometry::get_radius<0>(spheroid);
        CalculationType const b = geometry::get_radius<2>(spheroid);
        b_div_a = b / a;
    }

    inline CalculationType map_lat(CalculationType const& lat) const
    {
        return ::atan(b_div_a * ::tan(lat));
    }

    CalculationType b_div_a;
};

template <typename Spheroid, typename CalculationType>
struct mapper<Spheroid, mapping_geocentric, CalculationType>
{
    explicit inline mapper(Spheroid const& spheroid)
    {
        CalculationType const a = geometry::get_radius<0>(spheroid);
        CalculationType const b = geometry::get_radius<2>(spheroid);
        sqr_b_div_a = b / a;
        sqr_b_div_a *= sqr_b_div_a;
    }

    inline CalculationType map_lat(CalculationType const& lat) const
    {
        return ::atan(sqr_b_div_a * ::tan(lat));
    }

    CalculationType sqr_b_div_a;
};

}
#endif // DOXYGEN_NO_DETAIL


/*!
\brief Check at which side of a geographical segment a point lies
         left of segment (> 0), right of segment (< 0), on segment (0).
         The check is performed by mapping the geographical coordinates
         to spherical coordinates and using spherical_side_formula.
\ingroup strategies
\tparam Spheroid The reference spheroid model
\tparam Mapping The type of mapping of geographical to spherical latitude
\tparam CalculationType \tparam_calculation
 */
template <typename Spheroid,
          mapping_type Mapping = mapping_geodetic,
          typename CalculationType = void>
class mapping_spherical_side_formula
{

public :
    inline mapping_spherical_side_formula()
    {}

    explicit inline mapping_spherical_side_formula(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename P1, typename P2, typename P>
    inline int apply(P1 const& p1, P2 const& p2, P const& p)
    {
        typedef typename promote_floating_point
            <
                typename select_calculation_type_alt
                    <
                        CalculationType,
                        P1, P2, P
                    >::type
            >::type calc_t;

        side::detail::mapper<Spheroid, mapping_geodetic, calc_t> const
            mapper(m_spheroid);

        // NOTE: Should CalculateType be used as CoordinateType in the following Points?
        //       Coordinates could also be passed directly to some SSF utility.

        return spherical_side_formula<CalculationType>
                ::apply(mapped<P1>(p1, mapper),
                        mapped<P2>(p2, mapper),
                        mapped<P>(p, mapper));
    }

    template <typename ResP, typename P, typename Mapper>
    static inline ResP mapped(P const& src, Mapper const& mapper)
    {
        boost::ignore_unused(mapper);

        ResP dst;
        geometry::set<0>(dst, geometry::get<0>(src));
        geometry::set<1>(dst, mapper.map_lat(geometry::get<1>(src)));
        return dst;
    }

private:
    Spheroid m_spheroid;
};

// The specialization for geodetic latitude which can be used directly
template <typename Spheroid,
          typename CalculationType>
class mapping_spherical_side_formula<Spheroid, mapping_geodetic, CalculationType>
{

public :
    inline mapping_spherical_side_formula() {}
    explicit inline mapping_spherical_side_formula(Spheroid const& /*spheroid*/) {}

    template <typename P1, typename P2, typename P>
    static inline int apply(P1 const& p1, P2 const& p2, P const& p)
    {
        return spherical_side_formula<CalculationType>::apply(p1, p2, p);
    }
};

}} // namespace strategy::side

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_MAPPING_SSF_HPP
