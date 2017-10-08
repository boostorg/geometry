// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP
#define BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP


#include <string>

#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/srs/projection.hpp>

#include <boost/geometry/srs/projections/impl/pj_transform.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>


namespace boost { namespace geometry
{

namespace projections { namespace detail
{

struct transform_range
{
    template <typename Proj1, typename Proj2, typename Range>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             Range & range)
    {
        pj_transform(proj1.proj(), proj1.proj().params(),
                     proj2.proj(), proj2.proj().params(),
                     range);
    }
};

template <typename Policy>
struct transform_multi
{
    template <typename Proj1, typename Proj2, typename Multi>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             Multi & multi)
    {
        apply(proj1, proj2, boost::begin(multi), boost::end(multi));
    }

private:
    template <typename Proj1, typename Proj2, typename FwdIt>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             FwdIt first, FwdIt last)
    {
        for ( ; first != last ; ++first )
        {
            Policy::apply(proj1, proj2, *first);
        }
    }
};

template
<
    typename Geometry,
    typename Tag = typename geometry::tag<Geometry>::type
>
struct transform
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THIS_GEOMETRY,
                         (Geometry));
};

template <typename Point>
struct transform<Point, point_tag>
{
    template <typename Proj1, typename Proj2>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             Point & point)
    {
        Point * ptr = boost::addressof(point);

        std::pair<Point*, Point*> range = std::make_pair(ptr, ptr + 1);

        transform_range::apply(proj1, proj2, range);
    }
};

template <typename MultiPoint>
struct transform<MultiPoint, multi_point_tag>
    : transform_range
{};

template <typename Segment>
struct transform<Segment, segment_tag>
{
    template <typename Proj1, typename Proj2>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             Segment & segment)
    {
        typedef typename geometry::point_type<Segment>::type point_type;

        point_type points[2];

        geometry::detail::assign_point_from_index<0>(segment, points[0]);
        geometry::detail::assign_point_from_index<1>(segment, points[1]);

        transform_range::apply(proj1, proj2, points);

        geometry::detail::assign_point_to_index<0>(points[0], segment);
        geometry::detail::assign_point_to_index<1>(points[1], segment);
    }
};

template <typename Linestring>
struct transform<Linestring, linestring_tag>
    : transform_range
{};

template <typename MultiLinestring>
struct transform<MultiLinestring, multi_linestring_tag>
    : transform_multi<transform_range>
{};

template <typename Ring>
struct transform<Ring, ring_tag>
    : transform_range
{};

template <typename Polygon>
struct transform<Polygon, polygon_tag>
{
    template <typename Proj1, typename Proj2>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             Polygon & poly)
    {
        transform_range::apply(proj1, proj2, geometry::exterior_ring(poly));
        transform_multi<transform_range>::apply(proj1, proj2, geometry::interior_rings(poly));
    }
};

template <typename MultiPolygon>
struct transform<MultiPolygon, multi_polygon_tag>
    : transform_multi
        <
            transform
                <
                    typename boost::range_value<MultiPolygon>::type,
                    polygon_tag
                >
        >
{};


}} // namespace projections::detail
    
namespace srs
{


/*!
    \brief Representation of projection
    \details Either dynamic or static projection representation
    \ingroup projection
    \tparam Proj1 default_dynamic or static projection parameters
    \tparam Proj2 default_dynamic or static projection parameters
    \tparam CT calculation type used internally
*/
template
<
    typename Proj1 = srs::dynamic,
    typename Proj2 = srs::dynamic,
    typename CT = double
>
class transformation
{
    // TODO: currently the cordinate type of RangeOut is used instead
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

public:
    transformation()
    {}

    template <typename Parameters1>
    explicit transformation(Parameters1 const& parameters1)
        : m_proj1(parameters1)
    {}

    template <typename Parameters1, typename Parameters2>
    transformation(Parameters1 const& parameters1, Parameters2 const& parameters2)
        : m_proj1(parameters1)
        , m_proj2(parameters2)
    {}

    template <typename GeometryIn, typename GeometryOut>
    bool forward(GeometryIn const& in, GeometryOut & out) const
    {
        // TODO: no need to convert the whole geometry
        // would be ok to do this per-range
        if (boost::addressof(in) != boost::addressof(out))
            geometry::convert(in, out);

        try
        {
            projections::detail::transform
                <
                    GeometryOut
                >::apply(m_proj1, m_proj2, out);
            
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    template <typename GeometryIn, typename GeometryOut>
    bool inverse(GeometryIn const& in, GeometryOut & out) const
    {
        // TODO: no need to convert the whole geometry
        // would be ok to do this per-range
        if (boost::addressof(in) != boost::addressof(out))
            geometry::convert(in, out);

        try
        {
            projections::detail::transform
                <
                    GeometryOut
                >::apply(m_proj2, m_proj1, out);

            return true;
        }
        catch(...)
        {
            return false;
        }
    }

private:
    projections::proj_wrapper<Proj1, CT> m_proj1;
    projections::proj_wrapper<Proj2, CT> m_proj2;
};

} // namespace srs


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP
