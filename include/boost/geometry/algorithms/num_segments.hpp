// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_NUM_SEGMENTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_NUM_SEGMENTS_HPP

#include <cstddef>

#include <boost/mpl/size_t.hpp>
#include <boost/mpl/times.hpp>

#include <boost/range.hpp>

#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/ipower.hpp>
#include <boost/geometry/util/range.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/algorithms/detail/counting.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace num_segments
{


struct range_count
{
    template <typename Range>
    static inline std::size_t apply(Range const& range, bool add_for_open)
    {
        std::size_t n = boost::size(range);
        if ( n <= 1 )
        {
            return 0;
        }
        if (add_for_open
            && geometry::closure<Range>::value == open
            && detail::disjoint::disjoint_point_point(range::front(range),
                                                      range::at(range, n - 1))
            )
        {
            return n;
        }
        return static_cast<std::size_t>(n - 1);
    }
};

}} // namespace detail::num_segments
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct num_segments
    : not_implemented<Tag>
{};

template <typename Geometry>
struct num_segments<Geometry, point_tag>
    : detail::counting::other_count<0>
{};

// the number of segments (1-dimensional faces) of the hypercube is
// given by the formula: d * 2^(d-1), where d is the dimension of the
// hypercube; see also:
//            http://en.wikipedia.org/wiki/Hypercube
template <typename Geometry>
struct num_segments<Geometry, box_tag>
    : detail::counting::other_count
        <
            boost::mpl::times
                <
                    geometry::dimension<Geometry>,
                    util::ipower
                        <
                            boost::mpl::size_t<2>,
                            geometry::dimension<Geometry>::value - 1
                        >
                >::value
        >
{};

template <typename Geometry>
struct num_segments<Geometry, segment_tag>
    : detail::counting::other_count<1>
{};

template <typename Geometry>
struct num_segments<Geometry, linestring_tag>
    : detail::num_segments::range_count
{};

template <typename Geometry>
struct num_segments<Geometry, ring_tag>
    : detail::num_segments::range_count
{};

template <typename Geometry>
struct num_segments<Geometry, polygon_tag>
    : detail::counting::polygon_count<detail::num_segments::range_count>
{};

template <typename Geometry>
struct num_segments<Geometry, multi_point_tag>
    : detail::counting::other_count<0>
{};

template <typename Geometry>
struct num_segments<Geometry, multi_linestring_tag>
    : detail::counting::multi_count
        <
            num_segments< typename boost::range_value<Geometry>::type>
        >
{};

template <typename Geometry>
struct num_segments<Geometry, multi_polygon_tag>
    : detail::counting::multi_count
        <
            num_segments< typename boost::range_value<Geometry>::type>
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



namespace resolve_variant
{


template <typename Geometry>
struct num_segments
{
    static inline std::size_t apply(Geometry const& geometry, bool add_for_open)
    {
        concept::check<Geometry const>();

        return dispatch::num_segments<Geometry>::apply(geometry, add_for_open);
    }
};


template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct num_segments<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor: boost::static_visitor<std::size_t>
    {
        bool m_add_for_open;

        visitor(bool add_for_open): m_add_for_open(add_for_open) {}

        template <typename Geometry>
        inline std::size_t operator()(Geometry const& geometry) const
        {
            return num_segments<Geometry>::apply(geometry, m_add_for_open);
        }
    };

    static inline std::size_t
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
          bool add_for_open)
    {
        return boost::apply_visitor(visitor(add_for_open), geometry);
    }
};


} // namespace resolve_variant



/*!
\brief \brief_calc{number of segments}
\ingroup num_segments
\details \details_calc{num_segments, number of segments}.
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\param add_for_open add one for open geometries (i.e. polygon types which are not closed)
\return \return_calc{number of segments}

\qbk{[include reference/algorithms/num_segments.qbk]}
*/
template <typename Geometry>
inline std::size_t num_segments(Geometry const& geometry,
                                bool add_for_open = false)
{
    return resolve_variant::num_segments
        <
            Geometry
        >::apply(geometry, add_for_open);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_NUM_SEGMENTS_HPP
