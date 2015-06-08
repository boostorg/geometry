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

#include <boost/assert.hpp>
#include <boost/core/addressof.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/range.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_reference.hpp>

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
    typename Difference = typename boost::range_difference
        <
            typename boost::remove_reference
                <
                    typename interior_return_type<Polygon>::type
                >::type
        >::type
>
class polygon_rings_iterator
    : public boost::iterator_facade
        <
            polygon_rings_iterator<Polygon, Value, Reference, Difference>,
            Value,
            boost::random_access_traversal_tag,
            Reference,
            Difference
        >
{
    typedef typename boost::range_size
        <
            typename boost::remove_reference
                <
                    typename interior_return_type<Polygon>::type
                >::type
        >::type size_type;

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
        , m_index(static_cast<size_type>(num_rings(polygon)))
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
        BOOST_ASSERT(m_polygon == other.m_polygon);
        return m_index == other.m_index;
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
        return static_cast<Difference>(other.m_index)
            - static_cast<Difference>(m_index);
    }

    inline void advance(Difference n)
    {
        m_index += n;
    }

private:
    Polygon* m_polygon;
    size_type m_index;
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


template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct views_container_initializer
{};

template <typename Polygon>
struct views_container_initializer<Polygon, polygon_tag>
{
    typedef polygon_rings_iterator<Polygon> rings_iterator_type;

    static inline std::size_t num_rings(Polygon const& polygon)
    {
        return geometry::num_interior_rings(polygon) + 1;
    }

    template <typename BoundaryView>
    static inline void apply(Polygon const& polygon, BoundaryView* views)
    {
        std::uninitialized_copy(rings_iterator_type(polygon),
                                rings_iterator_type(polygon, true),
                                views);
    }
};

template <typename MultiPolygon>
struct views_container_initializer<MultiPolygon, multi_polygon_tag>
{
    typedef typename boost::mpl::if_
        <
            boost::is_const<MultiPolygon>,
            typename boost::range_value<MultiPolygon>::type const,
            typename boost::range_value<MultiPolygon>::type
        >::type polygon_type;

    typedef polygon_rings_iterator<polygon_type> rings_iterator_type;

    static inline std::size_t num_rings(MultiPolygon const& multipolygon)
    {
        return geometry::num_interior_rings(multipolygon)
            + static_cast<std::size_t>(boost::size(multipolygon));
    }

    template <typename BoundaryView>
    static inline void apply(MultiPolygon const& multipolygon,
                             BoundaryView* views)
    {
        BoundaryView* cur_it = views;
        for (typename boost::range_iterator<MultiPolygon>::type
                 it = boost::begin(multipolygon);
             it != boost::end(multipolygon);
             ++it)
        {
            cur_it = std::uninitialized_copy(rings_iterator_type(*it),
                                             rings_iterator_type(*it, true),
                                             cur_it);
        }
    }
};


template <typename Areal>
class areal_boundary
{
    typedef boundary_view<typename ring_type<Areal>::type> boundary_view_type;
    typedef views_container_initializer<Areal> initializer;

    inline void initialize_views(Areal const& areal)
    {
        std::pair<boundary_view_type*, std::ptrdiff_t> result
            = std::get_temporary_buffer<boundary_view_type>(m_num_rings);

        if ( result.second < m_num_rings )
        {
            throw std::bad_alloc();
        }

        m_views = result.first;
        try
        {
            initializer::apply(areal, m_views);
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    inline void clear()
    {
        for (std::ptrdiff_t i = 0; i < m_num_rings; ++i)
        {
            m_views[i].~boundary_view_type();
        }
        std::return_temporary_buffer(m_views);
    }

public:
    typedef boundary_view_type* iterator;
    typedef boundary_view_type const* const_iterator;

    typedef multi_linestring_tag tag_type;

    explicit areal_boundary(Areal& areal)
        : m_num_rings(static_cast
                          <
                              std::ptrdiff_t
                          >(initializer::num_rings(areal)))
    {
        initialize_views(areal);
    }

    ~areal_boundary()
    {
        clear();
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
    : detail::boundary_views::areal_boundary<Polygon>
{
    explicit boundary_view(Polygon& polygon)
        : detail::boundary_views::areal_boundary<Polygon>(polygon)
    {}
};

template <typename MultiPolygon>
struct boundary_view<MultiPolygon, multi_polygon_tag>
    : detail::boundary_views::areal_boundary<MultiPolygon>
{
    explicit boundary_view(MultiPolygon& multipolygon)
        : detail::boundary_views::areal_boundary
            <
                MultiPolygon
            >(multipolygon)
    {}
};


} // namespace detail_dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_VIEWS_DETAIL_BOUNDARY_VIEW_IMPLEMENTATION_HPP
