// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MYSQL_ALGORITHMS_DETAIL_SUB_GEOMETRY_HPP
#define BOOST_GEOMETRY_MYSQL_ALGORITHMS_DETAIL_SUB_GEOMETRY_HPP

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

// TODO: later remove IsMulti and move to multi directory
template <typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type,
          bool IsMulti = boost::is_base_of<multi_tag, Tag>::value>
struct get_dispatch
{
    BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THIS_GEOMETRY, (Geometry, Tag));
};

template <typename Geometry, typename Tag>
struct get_dispatch<Geometry, Tag, false>
{
    typedef Geometry & result_type;

    template <typename Id> static inline
    result_type apply(Geometry & geometry, Id const&)
    {
        return geometry;
    }
};

template <typename Geometry>
struct get_dispatch<Geometry, polygon_tag, false>
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
struct get_dispatch<Geometry, Tag, true>
{
    typedef typename boost::range_value<Geometry>::type sub_type;
    typedef typename get_dispatch<sub_type>::result_type result_type;

    template <typename Id> static inline
    result_type apply(Geometry & geometry, Id const& id)
    {
        BOOST_ASSERT(0 <= id.multi_index);
        return get_dispatch<sub_type>::apply(*(boost::begin(geometry) + id.multi_index), id);
    }
};

template <typename Geometry>
struct result_type
{
    typedef typename get_dispatch<Geometry>::result_type type;
};

//template <typename Geometry>
//struct result_type<Geometry const>
//{
//    typedef typename get_dispatch<Geometry const>::result_type type;
//};

// This function also works for geometry::segment_identifier

template <typename Geometry, typename Id> inline
typename get_dispatch<Geometry>::result_type
get(Geometry & g, Id const& id)
{
    return get_dispatch<Geometry>::apply(g, id);
};

//template <typename Geometry, typename Id> inline
//typename get_dispatch<Geometry>::result_type
//get(Geometry const& g, Id const& id)
//{
//    return get_dispatch<Geometry const>::apply(g, id);
//};

} // namespace sub_geometry

} // namespace detail
#endif

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MYSQL_ALGORITHMS_DETAIL_SUB_GEOMETRY_HPP
