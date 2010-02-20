// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_POINT_LL_HPP
#define BOOST_GEOMETRY_GEOMETRIES_POINT_LL_HPP

#include <cstddef>
#include <sstream>
#include <string>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/util/copy.hpp>

#include <boost/geometry/extensions/gis/latlong/detail/graticule.hpp>

namespace boost { namespace geometry
{

/*!
    \brief Point using spherical coordinates \a lat and \a lon, on Earth
    \ingroup Geometry
    \details The point_ll class implements a point with lat and lon functions.
    It can be constructed using latitude and longitude classes. The latlong
    class can be defined in degrees or in radians. There is a conversion method
    from degree to radian, and from radian to degree.
    \tparam T coordinate type, double (the default) or float
        (it might be int as well)
    \tparam C coordinate system, optional, should includes degree/radian
        indication, defaults to geographic<degree>
    \tparam D dimensions, optional, defaults to 2
    \note There is NO constructor with two values to avoid
        exchanging lat and long
    \note Construction with latitude and longitude can be done in both orders,
        so lat/long and long/lat
    \par Example:
    Example showing how the point_ll class can be constructed. Note that it
        can also be constructed using
    decimal degrees (43.123).
    \dontinclude doxygen_1.cpp
    \skip example_point_ll_construct
    \line {
    \until }
*/
template
<
    typename T = double,
    typename C = cs::geographic<degree>,
    std::size_t D = 2
>
class point_ll : public point<T, D, C>
{
public:

    /// Default constructor, does not initialize anything
    inline point_ll() : point<T, D, C>() {}

    /// Constructor with longitude/latitude
    inline point_ll(longitude<T> const& lo, latitude<T> const& la)
        : point<T, D, C>(lo, la) {}

    /// Constructor with latitude/longitude
    inline point_ll(latitude<T> const& la, longitude<T> const& lo)
        : point<T, D, C>(lo, la) {}

    /// Get longitude
    inline T const& lon() const { return this->template get<0>(); }
    /// Get latitude
    inline T const& lat() const { return this->template get<1>(); }

    /// Set longitude
    inline void lon(T const& v) { this->template set<0>(v); }
    /// Set latitude
    inline void lat(T const& v) { this->template set<1>(v); }

    /// Set longitude using dms class
    inline void lon(dms<east, T> const& v)
    {
        this->template set<0>(v.as_value());
    }
    inline void lon(dms<west, T> const& v)
    {
        this->template set<0>(v.as_value());
    }

    inline void lat(dms<north, T> const& v)
    {
        this->template set<1>(v.as_value());
    }
    inline void lat(dms<south, T> const& v)
    {
        this->template set<1>(v.as_value());
    }
};

// Adapt the point_ll to the concept
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename CoordinateType,
    typename CoordinateSystem,
    std::size_t DimensionCount
>
struct tag<point_ll<CoordinateType, CoordinateSystem, DimensionCount> >
{
    typedef point_tag type;
};

template
<
    typename CoordinateType,
    typename CoordinateSystem,
    std::size_t DimensionCount
>
struct coordinate_type
    <
        point_ll<CoordinateType, CoordinateSystem, DimensionCount>
    >
{
    typedef CoordinateType type;
};

template
<
    typename CoordinateType,
    typename CoordinateSystem,
    std::size_t DimensionCount
>
struct coordinate_system
    <
        point_ll<CoordinateType, CoordinateSystem, DimensionCount>
    >
{
    typedef CoordinateSystem type;
};

template
<
    typename CoordinateType,
    typename CoordinateSystem,
    std::size_t DimensionCount
>
struct dimension
    <
        point_ll<CoordinateType, CoordinateSystem, DimensionCount>
    >
    : boost::mpl::int_<DimensionCount>
{};

template
<
    typename CoordinateType,
    typename CoordinateSystem,
    std::size_t DimensionCount,
    std::size_t Dimension
>
struct access
    <
        point_ll<CoordinateType, CoordinateSystem, DimensionCount>, Dimension
    >
{
    static inline CoordinateType get(
        point_ll<CoordinateType, CoordinateSystem, DimensionCount> const& p)
    {
        return p.template get<Dimension>();
    }

    static inline void set(
        point_ll<CoordinateType, CoordinateSystem, DimensionCount>& p,
        CoordinateType const& value)
    {
        p.template set<Dimension>(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_POINT_LL_HPP
