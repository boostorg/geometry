// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_BELOW_MINIMUM_SIZE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_BELOW_MINIMUM_SIZE_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{

// Silence warning C4127: conditional expression is constant
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127)
#endif


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_below_minimum_size
{

struct always_not_below
{
    template <typename Geometry>
    static inline bool apply(Geometry const&)
    {
        return true;
    }
};

template <std::size_t MinimumSize>
struct range_not_below_minimum_size
{
    template <typename Range>
    static inline bool apply(Range const& range)
    {
        return boost::size(range) >= MinimumSize;
    }
};

class polygon_not_below_minimum_size
{
private:

    template <typename Polygon>
    struct predicate
    {
        typedef range_not_below_minimum_size
            <
                core_detail::closure::minimum_ring_size
                    <
                        closure<Polygon>::value
                    >::value
            > type;
    };

    template <typename Predicate, typename InteriorRings>
    static inline bool
    apply_to_interior_rings(InteriorRings const& interior_rings)
    {
        return check_iterator_range
            <
                Predicate, true // allow for no interior rings
            >::apply(boost::begin(interior_rings), boost::end(interior_rings));
    }

public:
    template <typename Polygon>
    static inline bool apply(Polygon const& polygon)
    {
        typedef typename predicate<Polygon>::type predicate_type;
        return predicate_type::apply(exterior_ring(polygon))
            && apply_to_interior_rings
                <
                    predicate_type
                >(interior_rings(polygon));
    }
};

template <typename Predicate, bool AllowEmptyMultiGeometry>
struct multigeometry_not_below_minimum_size
{
    template <typename MultiGeometry>
    static inline bool apply(MultiGeometry const& multi_geometry)
    {
        return check_iterator_range
            <
                Predicate,
                AllowEmptyMultiGeometry
            >::apply(boost::begin(multi_geometry),
                     boost::end(multi_geometry));
    }
};

}} // namespace detail::is_below_minimum_size
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct is_not_below_minimum_size: not_implemented<Tag>
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, point_tag>
    : detail::is_below_minimum_size::always_not_below
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, box_tag>
    : detail::is_below_minimum_size::always_not_below
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, segment_tag>
    : detail::is_below_minimum_size::always_not_below
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, linestring_tag>
    : detail::is_below_minimum_size::range_not_below_minimum_size<2>
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, ring_tag>
    : detail::is_below_minimum_size::range_not_below_minimum_size
        <
            core_detail::closure::minimum_ring_size
                <
                    closure<Geometry>::value
                >::value
        >
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, polygon_tag>
    : detail::is_below_minimum_size::polygon_not_below_minimum_size
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, multi_point_tag>
    : detail::is_below_minimum_size::range_not_below_minimum_size<1>
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, multi_linestring_tag>
    : detail::is_below_minimum_size::multigeometry_not_below_minimum_size
        <
            detail::is_below_minimum_size::range_not_below_minimum_size<2>,
            false
        >
{};

template <typename Geometry>
struct is_not_below_minimum_size<Geometry, multi_polygon_tag>
    : detail::is_below_minimum_size::multigeometry_not_below_minimum_size
        <
            detail::is_below_minimum_size::polygon_not_below_minimum_size,
            false
        >
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_variant
{

template <typename Geometry>
struct is_below_minimum_size
{
    static inline bool apply(Geometry const& geometry)
    {
        concept::check<Geometry const>();

        return !dispatch::is_not_below_minimum_size<Geometry>::apply(geometry);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct is_below_minimum_size<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor: boost::static_visitor<bool>
    {
        template <typename Geometry>
        inline bool operator()(Geometry const& geometry) const
        {
            return is_below_minimum_size<Geometry>::apply(geometry);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry)
    {
        return boost::apply_visitor(visitor(), geometry);
    }
};

} // namespace resolve_variant


/*!
\brief \brief_calc{checks whether a geometry's number of points is
below the minimum acceptable size}
\ingroup is_below_minimum_size
\details \details_calc{is_below_minimum_size}.
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_calc{true if the geometry has less than the minimum
acceptable number of points}

\qbk{[include reference/algorithms/is_below_minimum_size.qbk]}
*/
template <typename Geometry>
inline bool is_below_minimum_size(Geometry const& geometry)
{
    return resolve_variant::is_below_minimum_size<Geometry>::apply(geometry);
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_IS_BELOW_MINIMUM_SIZE_HPP
