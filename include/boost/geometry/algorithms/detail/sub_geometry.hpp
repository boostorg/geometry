// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SUB_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SUB_GEOMETRY_HPP

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace sub_geometry {

// TODO: later implement Geometry-specialized index_type
// polygons/multi_polygons also need ring_index member
struct index_type
{
    index_type() : multi_index(-1) {}

    int multi_index;

    inline bool operator<(index_type const& i) const
    {
        return multi_index < i.multi_index;
    }

    inline bool operator==(index_type const& i) const
    {
        return multi_index == i.multi_index;
    }
};

}} // namespace detail::sub_geometry

namespace detail_dispatch { namespace sub_geometry {

// TODO: later remove IsMulti and move to multi directory
template <typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type,
          bool IsMulti = boost::is_base_of<multi_tag, Tag>::value>
struct get : not_implemented<Tag>
{};

template <typename Geometry, typename Tag>
struct get<Geometry, Tag, false>
{
    typedef Geometry & result_type;

    template <typename Id> static inline
    result_type apply(Geometry & geometry, Id const&)
    {
        return geometry;
    }
};

template <typename Geometry>
struct get<Geometry, polygon_tag, false>
{
    typedef typename geometry::ring_type<Geometry>::type & result_type;

    template <typename Id> static inline
    result_type apply(Geometry & geometry, Id const& id)
    {
        if ( id.ring_index < 0 )
            return geometry::exterior_ring(geometry);
        else
        {
            BOOST_ASSERT( id.ring_index < boost::size(geometry::interior_rings(geometry)) );

            return *(boost::begin(geometry::interior_rings(geometry)) + id.ring_index);
        }
    }
};

template <typename Geometry, typename Tag>
struct get<Geometry, Tag, true>
{
    typedef typename boost::range_value<Geometry>::type value_type;
    typedef typename boost::mpl::if_c
        <
            boost::is_const<Geometry>::value,
            typename boost::add_const<value_type>::type,
            value_type
        >::type sub_type;
    typedef detail_dispatch::sub_geometry::get<sub_type> get_type;
    typedef typename get_type::result_type result_type;

    template <typename Id> static inline
    result_type apply(Geometry & geometry, Id const& id)
    {
        BOOST_ASSERT(0 <= id.multi_index);
        return get_type::apply(*(boost::begin(geometry) + id.multi_index), id);
    }
};

}} // namespace detail_dispatch::sub_geometry

namespace detail { namespace sub_geometry {

template <typename Geometry>
struct result_type
{
    typedef typename detail_dispatch::sub_geometry::get<Geometry>::result_type type;
};

// This function also works for geometry::segment_identifier

template <typename Geometry, typename Id> inline
typename detail_dispatch::sub_geometry::get<Geometry>::result_type
get(Geometry & geometry, Id const& id)
{
    return detail_dispatch::sub_geometry::get<Geometry>::apply(geometry, id);
};

}} // namespace detail::sub_geometry
#endif

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SUB_GEOMETRY_HPP
