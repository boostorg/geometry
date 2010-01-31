// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_POINT_XY_HPP
#define BOOST_GEOMETRY_GEOMETRIES_POINT_XY_HPP

#include <cstddef>

#include <boost/mpl/int.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace boost { namespace geometry
{

/*!
    \brief 2D point in Cartesian coordinate system
    \ingroup geometries
    \tparam T numeric type, arguments can be, for example, double, float, int
*/
template<typename T, typename C = cs::cartesian>
class point_xy : public point<T, 2, C>
{
public:

    /// Default constructor, does not initialize anything
    inline point_xy() : point<T, 2, C>() {}

    /// Constructor with x/y values
    inline point_xy(T const& x, T const& y) : point<T, 2, C>(x, y) {}

    /// Get x-value
    inline T const& x() const
    { return this->template get<0>(); }

    /// Get y-value
    inline T const& y() const
    { return this->template get<1>(); }

    /// Set x-value
    inline void x(T const& v)
    { this->template set<0>(v); }

    /// Set y-value
    inline void y(T const& v)
    { this->template set<1>(v); }
};

// Adapt the point_xy to the concept
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename CoordinateType, typename CoordinateSystem>
struct tag<point_xy<CoordinateType, CoordinateSystem> >
{
    typedef point_tag type;
};

template<typename CoordinateType, typename CoordinateSystem>
struct coordinate_type<point_xy<CoordinateType, CoordinateSystem> >
{
    typedef CoordinateType type;
};

template<typename CoordinateType, typename CoordinateSystem>
struct coordinate_system<point_xy<CoordinateType, CoordinateSystem> >
{
    typedef CoordinateSystem type;
};

template<typename CoordinateType, typename CoordinateSystem>
struct dimension<point_xy<CoordinateType, CoordinateSystem> >
    : boost::mpl::int_<2>
{};

template<typename CoordinateType, typename CoordinateSystem, std::size_t Dimension>
struct access<point_xy<CoordinateType, CoordinateSystem>, Dimension >
{
    static inline CoordinateType get(
        point_xy<CoordinateType, CoordinateSystem> const& p)
    {
        return p.template get<Dimension>();
    }

    static inline void set(point_xy<CoordinateType, CoordinateSystem>& p,
        CoordinateType const& value)
    {
        p.template set<Dimension>(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_POINT_XY_HPP
