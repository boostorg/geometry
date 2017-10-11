// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP
#define BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP


#include <string>

#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/segment.hpp>

#include <boost/geometry/srs/projection.hpp>
#include <boost/geometry/srs/projections/impl/pj_transform.hpp>

#include <boost/geometry/views/detail/indexed_point_view.hpp>

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

template <typename T1, typename T2>
inline bool same_object(T1 const& , T2 const& )
{
    return false;
}

template <typename T>
inline bool same_object(T const& o1, T const& o2)
{
    return boost::addressof(o1) == boost::addressof(o2);
}

template <typename G1, typename G2>
struct same_tags
{
    static const bool value = boost::is_same
        <
            typename geometry::tag<G1>::type,
            typename geometry::tag<G2>::type
        >::value;
};

template
<
    typename PtIn,
    typename PtOut,
    bool SameUnits = geometry::is_radian
                        <
                            typename geometry::coordinate_system<PtIn>::type
                        >::type::value
                     ==
                     geometry::is_radian
                        <
                            typename geometry::coordinate_system<PtOut>::type
                        >::type::value
>
struct transform_geometry_point_coordinates
{
    static inline void apply(PtIn const& in, PtOut & out)
    {
        geometry::set<0>(out, geometry::get<0>(in));
        geometry::set<1>(out, geometry::get<1>(in));
    }
};

template <typename PtIn, typename PtOut>
struct transform_geometry_point_coordinates<PtIn, PtOut, false>
{
    static inline void apply(PtIn const& in, PtOut & out)
    {
        geometry::set_from_radian<0>(out, geometry::get_as_radian<0>(in));
        geometry::set_from_radian<1>(out, geometry::get_as_radian<1>(in));
    }
};

template <typename Geometry, typename CT>
struct transform_geometry_point
{
    typedef typename geometry::point_type<Geometry>::type point_type;

    typedef geometry::model::point
        <   
            typename select_most_precise
                <
                    typename geometry::coordinate_type<point_type>::type,
                    CT
                >::type,
            geometry::dimension<point_type>::type::value,
            typename geometry::coordinate_system<point_type>::type
        > type;

    template <typename PtIn, typename PtOut>
    static inline void apply(PtIn const& in, PtOut & out)
    {
        transform_geometry_point_coordinates<PtIn, PtOut>::apply(in, out);
        projections::detail::copy_higher_dimensions<2>(in, out);
    }
};

template <typename Geometry, typename CT>
struct transform_geometry_range_base
{
    template <typename In, typename Out>
    static inline void apply(In const& in, Out & out)
    {
        // Change the order and/or closure if needed
        // In - arbitrary geometry
        // Out - either Geometry or std::vector
        // So the order and closure of In and Geometry shoudl be compared
        // std::vector's order is assumed to be the same as of Geometry
        geometry::detail::conversion::range_to_range
            <
                In,
                Out,
                geometry::point_order<In>::value != geometry::point_order<Out>::value,
                transform_geometry_point<Geometry, CT>
            >::apply(in, out);
    }
};

template
<
    typename Geometry,
    typename CT,
    typename Tag = typename geometry::tag<Geometry>::type
>
struct transform_geometry
{};

template <typename Point, typename CT>
struct transform_geometry<Point, CT, point_tag>
    : transform_geometry_point<Point, CT>
{};

template <typename Segment, typename CT>
struct transform_geometry<Segment, CT, segment_tag>
{
    typedef geometry::model::segment
        <
            typename transform_geometry_point<Segment, CT>::type
        > type;

    template <typename In, typename Out>
    static inline void apply(In const& in, Out & out)
    {
        apply<0>(in, out);
        apply<1>(in, out);
    }

private:
    template <std::size_t Index, typename In, typename Out>
    static inline void apply(In const& in, Out & out)
    {
        geometry::detail::indexed_point_view<In const, Index> in_pt0(in);
        geometry::detail::indexed_point_view<Out, Index> out_pt0(out);
        transform_geometry_point<Segment, CT>::apply(in_pt0, out_pt0);
    }
};

template <typename MultiPoint, typename CT>
struct transform_geometry<MultiPoint, CT, multi_point_tag>
    : transform_geometry_range_base<MultiPoint, CT>
{
    typedef model::multi_point
        <
            typename transform_geometry_point<MultiPoint, CT>::type
        > type;
};

template <typename LineString, typename CT>
struct transform_geometry<LineString, CT, linestring_tag>
    : transform_geometry_range_base<LineString, CT>
{
    typedef model::linestring
        <
            typename transform_geometry_point<LineString, CT>::type
        > type;
};

template <typename Ring, typename CT>
struct transform_geometry<Ring, CT, ring_tag>
    : transform_geometry_range_base<Ring, CT>
{
    typedef model::ring
        <
            typename transform_geometry_point<Ring, CT>::type,
            geometry::point_order<Ring>::value == clockwise,
            geometry::closure<Ring>::value == closed
        > type;
};

template
<
    typename OutGeometry,
    typename CT,
    bool EnableTemporary = ! boost::is_same
                                <
                                    typename select_most_precise
                                        <
                                            typename geometry::coordinate_type<OutGeometry>::type,
                                            CT
                                        >::type,
                                    typename geometry::coordinate_type<OutGeometry>::type
                                >::type::value
>
struct transform_geometry_wrapper
{
    typedef transform_geometry<OutGeometry, CT> transform;
    typedef typename transform::type type;

    template <typename InGeometry>
    transform_geometry_wrapper(InGeometry const& in, OutGeometry & out)
        : m_out(out)
    {
        if (! same_object(in, m_temp))
            transform::apply(in, m_temp);
    }

    type & get() { return m_temp; }
    void finish() { geometry::convert(m_temp, m_out); } // this is always copy 1:1 without changing the order or closure

private:
    type m_temp;
    OutGeometry & m_out;
};

template
<
    typename OutGeometry,
    typename CT
>
struct transform_geometry_wrapper<OutGeometry, CT, false>
{
    typedef transform_geometry<OutGeometry, CT> transform;
    typedef OutGeometry type;

    template <typename InGeometry>
    transform_geometry_wrapper(InGeometry const& in, OutGeometry & out)
        : m_out(out)
    {
        if (! same_object(in, out))
            transform::apply(in, out);
    }

    OutGeometry & get() { return m_out; }
    void finish() {}

private:
    OutGeometry & m_out;
};

template <typename CT>
struct transform_range
{
    template <typename Proj1, typename Proj2, typename RangeIn, typename RangeOut>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             RangeIn const& in, RangeOut & out)
    {
        transform_geometry_wrapper<RangeOut, CT> wrapper(in, out);

        pj_transform(proj1.proj(), proj1.proj().params(),
                     proj2.proj(), proj2.proj().params(),
                     wrapper.get());

        wrapper.finish();
    }
};

template <typename Policy>
struct transform_multi
{
    template <typename Proj1, typename Proj2, typename MultiIn, typename MultiOut>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             MultiIn const& in, MultiOut & out)
    {
        if (! same_object(in, out))
            range::resize(out, boost::size(in));

        apply(proj1, proj2,
              boost::begin(in), boost::end(in),
              boost::begin(out));
    }

private:
    template <typename Proj1, typename Proj2, typename InIt, typename OutIt>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             InIt in_first, InIt in_last, OutIt out_first)
    {
        for ( ; in_first != in_last ; ++in_first, ++out_first )
        {
            Policy::apply(proj1, proj2, *in_first, *out_first);
        }
    }
};

template
<
    typename Geometry,
    typename CT,
    typename Tag = typename geometry::tag<Geometry>::type
>
struct transform
    : not_implemented<Tag>
{};

template <typename Point, typename CT>
struct transform<Point, CT, point_tag>
{
    template <typename Proj1, typename Proj2, typename PointIn, typename PointOut>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             PointIn const& in, PointOut & out)
    {
        transform_geometry_wrapper<PointOut, CT> wrapper(in, out);

        typedef typename transform_geometry_wrapper<PointOut, CT>::type point_type;
        point_type * ptr = boost::addressof(wrapper.get());

        std::pair<point_type *, point_type *> range = std::make_pair(ptr, ptr + 1);

        pj_transform(proj1.proj(), proj1.proj().params(),
                     proj2.proj(), proj2.proj().params(),
                     range);

        wrapper.finish();
    }
};

template <typename MultiPoint, typename CT>
struct transform<MultiPoint, CT, multi_point_tag>
    : transform_range<CT>
{};

template <typename Segment, typename CT>
struct transform<Segment, CT, segment_tag>
{
    template <typename Proj1, typename Proj2, typename SegmentIn, typename SegmentOut>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             SegmentIn const& in, SegmentOut & out)
    {
        transform_geometry_wrapper<SegmentOut, CT> wrapper(in, out);

        typedef typename geometry::point_type
            <
                typename transform_geometry_wrapper<SegmentOut, CT>::type
            >::type point_type;

        point_type points[2];

        geometry::detail::assign_point_from_index<0>(wrapper.get(), points[0]);
        geometry::detail::assign_point_from_index<1>(wrapper.get(), points[1]);

        std::pair<point_type*, point_type*> range = std::make_pair(points, points + 2);

        pj_transform(proj1.proj(), proj1.proj().params(),
                     proj2.proj(), proj2.proj().params(),
                     range);

        geometry::detail::assign_point_to_index<0>(points[0], wrapper.get());
        geometry::detail::assign_point_to_index<1>(points[1], wrapper.get());

        wrapper.finish();
    }
};

template <typename Linestring, typename CT>
struct transform<Linestring, CT, linestring_tag>
    : transform_range<CT>
{};

template <typename MultiLinestring, typename CT>
struct transform<MultiLinestring, CT, multi_linestring_tag>
    : transform_multi<transform_range<CT> >
{};

template <typename Ring, typename CT>
struct transform<Ring, CT, ring_tag>
    : transform_range<CT>
{};

template <typename Polygon, typename CT>
struct transform<Polygon, CT, polygon_tag>
{
    template <typename Proj1, typename Proj2, typename PolygonIn, typename PolygonOut>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             PolygonIn const& in, PolygonOut & out)
    {
        transform_range<CT>::apply(proj1, proj2,
                                   geometry::exterior_ring(in),
                                   geometry::exterior_ring(out));
        transform_multi<transform_range<CT> >::apply(proj1, proj2,
                                                     geometry::interior_rings(in),
                                                     geometry::interior_rings(out));
    }
};

template <typename MultiPolygon, typename CT>
struct transform<MultiPolygon, CT, multi_polygon_tag>
    : transform_multi
        <
            transform
                <
                    typename boost::range_value<MultiPolygon>::type,
                    CT,
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
        BOOST_MPL_ASSERT_MSG((projections::detail::same_tags<GeometryIn, GeometryOut>::value),
                             NOT_SUPPORTED_COMBINATION_OF_GEOMETRIES,
                             (GeometryIn, GeometryOut));

        try
        {
            projections::detail::transform
                <
                    GeometryOut,
                    calc_t
                >::apply(m_proj1, m_proj2, in, out);
            
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
        BOOST_MPL_ASSERT_MSG((projections::detail::same_tags<GeometryIn, GeometryOut>::value),
                             NOT_SUPPORTED_COMBINATION_OF_GEOMETRIES,
                             (GeometryIn, GeometryOut));

        try
        {
            projections::detail::transform
                <
                    GeometryOut,
                    calc_t
                >::apply(m_proj2, m_proj1, in, out);

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
