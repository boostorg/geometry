// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_VIEWS_DETAIL_BOUNDARY_VIEW_IMPLEMENTATION_HPP
#define BOOST_GEOMETRY_VIEWS_DETAIL_BOUNDARY_VIEW_IMPLEMENTATION_HPP

#include <cstddef>
#include <algorithm>
#include <iterator>
#include <memory>
#include <new> // for std::bad_alloc
#include <utility>
#include <vector>

#include <boost/core/addressof.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/range.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/views/closeable_view.hpp>

#include <boost/geometry/algorithms/num_interior_rings.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace boundary_views
{


template
<
    typename Polygon,
    typename Value = typename ring_type<Polygon>::type,
    typename Reference = typename ring_return_type<Polygon>::type,
    typename Difference = std::ptrdiff_t
>
class polygon_rings_iterator
    : public boost::iterator_facade
        <
            polygon_rings_iterator<Polygon, Value, Reference>,
            Value,
            boost::random_access_traversal_tag,
            Reference
        >
{
public:
    // default constructor
    polygon_rings_iterator()
        : m_polygon(NULL)
        , m_index(0)
    {}

    // for begin
    polygon_rings_iterator(Polygon& polygon)
        : m_polygon(boost::addressof(polygon))
        , m_index(0)
    {}

    // for end
    polygon_rings_iterator(Polygon& polygon, bool)
        : m_polygon(boost::addressof(polygon))
        , m_index(num_rings(polygon))
    {}

    template
    <
        typename OtherPolygon,
        typename OtherValue,
        typename OtherReference,
        typename OtherDifference
    >
    polygon_rings_iterator(polygon_rings_iterator
                           <
                               OtherPolygon,
                               OtherValue,
                               OtherReference,
                               OtherDifference
                           > const& other)
        : m_polygon(other.m_polygon)
        , m_index(other.m_index)
    {
        static const bool is_convertible
            = boost::is_convertible<OtherPolygon, Polygon>::value;

        BOOST_MPL_ASSERT_MSG((is_convertible),
                             NOT_CONVERTIBLE,
                             (types<OtherPolygon>));
    }

private:
    friend class boost::iterator_core_access;

    template
    <
        typename OtherPolygon,
        typename OtherValue,
        typename OtherReference,
        typename OtherDifference
    >
    friend class polygon_rings_iterator;


    static inline std::size_t num_rings(Polygon const& polygon)
    {
        return geometry::num_interior_rings(polygon) + 1;
    }

    inline Reference dereference() const
    {
        if (m_index == 0)
        {
            return exterior_ring(*m_polygon);
        }
        return range::at(interior_rings(*m_polygon), m_index - 1);
    }

    template
    <
        typename OtherPolygon,
        typename OtherValue,
        typename OtherReference,
        typename OtherDifference
    >
    inline bool equal(polygon_rings_iterator
                      <
                          OtherPolygon,
                          OtherValue,
                          OtherReference,
                          OtherDifference
                      > const& other) const
    {
        return m_polygon == other.m_polygon && m_index == other.m_index;
    }

    inline void increment()
    {
        ++m_index;
    }

    inline void decrement()
    {
        --m_index;
    }

    template
    <
        typename OtherPolygon,
        typename OtherValue,
        typename OtherReference,
        typename OtherDifference
    >
    inline Difference distance_to(polygon_rings_iterator
                                  <
                                      OtherPolygon,
                                      OtherValue,
                                      OtherReference,
                                      OtherDifference
                                  > const& other) const
    {
        return other.m_index - m_index;
    }

    inline void advance(Difference n)
    {
        m_index += n;
    }

private:
    Polygon* m_polygon;
    std::size_t m_index;
};


template <typename Ring>
class ring_boundary : closeable_view<Ring, closure<Ring>::value>::type
{
private:
    typedef typename closeable_view<Ring, closure<Ring>::value>::type base_type;

public:
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    
    typedef linestring_tag tag_type;

    explicit ring_boundary(Ring& ring)
        : base_type(ring) {}

    iterator begin() { return base_type::begin(); }
    iterator end() { return base_type::end(); }
    const_iterator begin() const { return base_type::begin(); }
    const_iterator end() const { return base_type::end(); }
};


template <typename Polygon>
class polygon_boundary
{
    typedef boundary_view<typename ring_type<Polygon>::type> boundary_view_type;
    typedef polygon_rings_iterator<Polygon> rings_iterator_type;

    inline void initialize_views(Polygon& polygon)
    {
        std::pair<boundary_view_type*, std::ptrdiff_t> result
            = std::get_temporary_buffer<boundary_view_type>(m_num_rings);

        if ( result.second == 0 )
        {
            throw std::bad_alloc();
        }

        m_views = result.first;

        std::ptrdiff_t ctr = 0;
        rings_iterator_type first(polygon);
        rings_iterator_type last(polygon, true);
        for (rings_iterator_type rit = first; rit != last; ++rit, ++ctr)
        {
            new (m_views + ctr) boundary_view_type(*rit);
        }
    }


public:
    typedef boundary_view_type* iterator;
    typedef boundary_view_type const* const_iterator;

    typedef multi_linestring_tag tag_type;

    explicit polygon_boundary(Polygon& polygon)
        : m_num_rings(static_cast
                          <
                              std::ptrdiff_t
                          >(num_interior_rings(polygon) + 1))
    {
        initialize_views(polygon);
    }

    ~polygon_boundary()
    {
        std::return_temporary_buffer(m_views);
    }

    inline iterator begin() { return m_views; }
    inline iterator end() { return m_views + m_num_rings; }
    inline const_iterator begin() const { return m_views; }
    inline const_iterator end() const { return m_views + m_num_rings; }

private:
    std::ptrdiff_t m_num_rings;
    boundary_view_type* m_views;
};


template <typename MultiPolygon>
class multipolygon_boundary
{
private:
    typedef typename boost::mpl::if_
        <
            boost::is_const<MultiPolygon>,
            typename boost::range_value<MultiPolygon>::type const,
            typename boost::range_value<MultiPolygon>::type
        >::type polygon_type;

    typedef polygon_rings_iterator<polygon_type> rings_iterator_type;

    typedef boundary_view
        <
            typename ring_type<MultiPolygon>::type
        > boundary_view_type;

    inline void initialize_views(MultiPolygon& multipolygon)
    {
        std::pair<boundary_view_type*, std::ptrdiff_t> result
            = std::get_temporary_buffer<boundary_view_type>(m_num_rings);

        if ( result.second == 0 )
        {
            throw std::bad_alloc();
        }

        m_views = result.first;
        std::ptrdiff_t ctr = 0;
        for (typename boost::range_iterator<MultiPolygon>::type
                 it = boost::begin(multipolygon);
             it != boost::end(multipolygon);
             ++it)
        {
            rings_iterator_type first(*it);
            rings_iterator_type last(*it, true);
            for (rings_iterator_type rit = first; rit != last; ++rit, ++ctr)
            {
                new (m_views + ctr) boundary_view_type(*rit);
            }
        }
    }

public:
    typedef boundary_view_type* iterator;
    typedef boundary_view_type const* const_iterator;

    typedef multi_linestring_tag tag_type;

    multipolygon_boundary(MultiPolygon& multipolygon)
        : m_num_rings(static_cast
                          <
                              std::ptrdiff_t
                          >(geometry::num_interior_rings(multipolygon)
                            + boost::size(multipolygon)))
    {
        initialize_views(multipolygon);
    }

    ~multipolygon_boundary()
    {
        std::return_temporary_buffer(m_views);
    }

    inline iterator begin() { return m_views; }
    inline iterator end() { return m_views + m_num_rings; }
    inline const_iterator begin() const { return m_views; }
    inline const_iterator end() const { return m_views + m_num_rings; }

private:
    std::ptrdiff_t m_num_rings;
    boundary_view_type* m_views;
};


}} // namespace detail::boundary_view
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace detail_dispatch
{


template <typename Ring>
struct boundary_view<Ring, ring_tag>
    : detail::boundary_views::ring_boundary<Ring>
{
    explicit boundary_view(Ring& ring)
        : detail::boundary_views::ring_boundary<Ring>(ring)
    {}
};

template <typename Polygon>
struct boundary_view<Polygon, polygon_tag>
    : detail::boundary_views::polygon_boundary<Polygon>
{
    explicit boundary_view(Polygon& polygon)
        : detail::boundary_views::polygon_boundary<Polygon>(polygon)
    {}
};

template <typename MultiPolygon>
struct boundary_view<MultiPolygon, multi_polygon_tag>
    : detail::boundary_views::multipolygon_boundary<MultiPolygon>
{
    explicit boundary_view(MultiPolygon& multipolygon)
        : detail::boundary_views::multipolygon_boundary
            <
                MultiPolygon
            >(multipolygon)
    {}
};


} // namespace detail_dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_VIEWS_DETAIL_BOUNDARY_VIEW_IMPLEMENTATION_HPP
