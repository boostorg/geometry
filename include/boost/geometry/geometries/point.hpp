// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_POINT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_POINT_HPP

#include <cstddef>

#include <boost/mpl/int.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/util/math.hpp>

namespace boost { namespace geometry
{

/*!
    \brief Basic point class, having coordinates defined in a neutral way
    \ingroup geometries
    \tparam T numeric type, for example double, float, int
    \tparam D coordinate dimension as number of coordinates, for example 2
    \tparam C coordinate system, for example cs::cartesian
*/
template<typename T, std::size_t D, typename C>
class point
{
public:

    // Concept typedefs and members
    typedef T coordinate_type;
    typedef C coordinate_system;

    static const std::size_t coordinate_count = D;

    /// Default constructor, no initialization at all
    inline point()
    {}

    /// Constructs with one, or optionally two or three values
    inline point(T const& v0, T const& v1 = 0, T const& v2 = 0)
    {
        if (D >= 1) m_values[0] = v0;
        if (D >= 2) m_values[1] = v1;
        if (D >= 3) m_values[2] = v2;
    }


    /// Compile time access to coordinate values
    template <std::size_t K>
    inline T const& get() const
    {
        BOOST_STATIC_ASSERT(K < D);
        return m_values[K];
    }

    template <std::size_t K>
    inline void set(T value)
    {
        BOOST_STATIC_ASSERT(K < D);
        m_values[K] = value;
    }


private:

    T m_values[D];
};


// Adapt the point to the concept
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem
>
struct tag<point<CoordinateType, DimensionCount, CoordinateSystem> >
{
    typedef point_tag type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem
>
struct coordinate_type<point<CoordinateType, DimensionCount, CoordinateSystem> >
{
    typedef CoordinateType type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem
>
struct coordinate_system<point<CoordinateType, DimensionCount, CoordinateSystem> >
{
    typedef CoordinateSystem type;
};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem
>
struct dimension<point<CoordinateType, DimensionCount, CoordinateSystem> >
    : boost::mpl::int_<DimensionCount>
{};

template
<
    typename CoordinateType,
    std::size_t DimensionCount,
    typename CoordinateSystem,
    std::size_t Dimension
>
struct access<point<CoordinateType, DimensionCount, CoordinateSystem>, Dimension>
{
    static inline CoordinateType get(
        point<CoordinateType, DimensionCount, CoordinateSystem> const& p)
    {
        return p.template get<Dimension>();
    }

    static inline void set(
        point<CoordinateType, DimensionCount, CoordinateSystem>& p,
        CoordinateType const& value)
    {
        p.template set<Dimension>(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_POINT_HPP
