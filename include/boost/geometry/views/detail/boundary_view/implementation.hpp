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

    iterator begin()
    {
        return base_type::begin();
    }

    iterator end()
    {
        return base_type::end();
    }

    const_iterator begin() const
    {
        return base_type::begin();
    }

    const_iterator end() const
    {
        return base_type::end();
    }
};


struct as_boundary_view
{
    template <typename Ring>
    inline boundary_view<Ring> operator()(Ring& ring) const
    {
        return boundary_view<Ring>(ring);
    }
};


template <typename Polygon>
class polygon_boundary
{
    typedef std::vector
        <
            boundary_view<typename ring_type<Polygon>::type>
        > views_vector_type;

    inline void initialize_views(Polygon& polygon)
    {
        polygon_rings_iterator<Polygon> first(polygon);
        polygon_rings_iterator<Polygon> last(polygon, true);
        std::transform(first, last,
                       std::back_inserter(m_views),
                       as_boundary_view());
    }

public:
    typedef typename views_vector_type::iterator iterator;
    typedef typename views_vector_type::const_iterator const_iterator;

    typedef multi_linestring_tag tag_type;

    explicit polygon_boundary(Polygon& polygon)
    {
        initialize_views(polygon);
    }

    inline iterator begin()
    {
        return m_views.begin();
    }

    inline iterator end()
    {
        return m_views.end();
    }

    inline const_iterator begin() const
    {
        return m_views.begin();
    }

    inline const_iterator end() const
    {
        return m_views.end();
    }

private:
    views_vector_type m_views;
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

    typedef std::vector
        <
            boundary_view<typename ring_type<MultiPolygon>::type>
        > views_vector_type;

    inline void initialize_views(MultiPolygon& multipolygon)
    {
        std::back_insert_iterator<views_vector_type> oit(m_views);
        for (typename boost::range_iterator<MultiPolygon>::type
                 it = boost::begin(multipolygon);
             it != boost::end(multipolygon);
             ++it)
        {
            polygon_rings_iterator<polygon_type> first(*it);
            polygon_rings_iterator<polygon_type> last(*it, true);
            oit = std::transform(first, last, oit, as_boundary_view());
        }
    }
public:
    typedef typename views_vector_type::iterator iterator;
    typedef typename views_vector_type::const_iterator const_iterator;

    typedef multi_linestring_tag tag_type;

    multipolygon_boundary(MultiPolygon& multipolygon)
    {
        initialize_views(multipolygon);
    }

    inline iterator begin()
    {
        return m_views.begin();
    }

    inline iterator end()
    {
        return m_views.end();
    }

    inline const_iterator begin() const
    {
        return m_views.begin();
    }

    inline const_iterator end() const
    {
        return m_views.end();
    }

private:
    views_vector_type m_views;
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
