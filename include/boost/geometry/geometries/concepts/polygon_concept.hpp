// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2020-2021.
// Modifications copyright (c) 2020-2021, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYGON_CONCEPT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYGON_CONCEPT_HPP

#include <type_traits>

#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/geometries/concepts/concept_type.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/ring_concept.hpp>


namespace boost { namespace geometry { namespace concepts
{

/*!
\brief Checks polygon concept
\ingroup concepts
*/
template <typename PolygonType>
class Polygon
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    using polygon_type = typename std::remove_const<PolygonType>::type;

    using ring_const_type = typename traits::ring_const_type<polygon_type>::type;
    using ring_mutable_type = typename traits::ring_mutable_type<polygon_type>::type;
    using interior_const_type = typename traits::interior_const_type<polygon_type>::type;
    using interior_mutable_type = typename traits::interior_mutable_type<polygon_type>::type;

    using point_type = point_type_t<PolygonType>;
    using ring_type = ring_type_t<PolygonType>;

    BOOST_CONCEPT_ASSERT( (concepts::Point<point_type>) );
    BOOST_CONCEPT_ASSERT( (concepts::Ring<ring_type>) );

    //BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<interior_type>) );

    struct checker
    {
        static inline void apply()
        {
            polygon_type* poly = nullptr;
            polygon_type const* cpoly = nullptr;
            static_assert(std::is_convertible
                    <
                        ring_mutable_type,
                        decltype(traits::exterior_ring<PolygonType>::get(*poly))
                    >::value,
                    "traits::exterior_ring<PolygonType>::get(polygon_type&) must be convertible to ring_mutable_type");
            static_assert(std::is_convertible
                    <
                        interior_mutable_type,
                        decltype(traits::interior_rings<PolygonType>::get(*poly))
                    >::value,
                    "traits::interior_rings<PolygonType>::get(polygon_type&) must be convertible to interior_mutable_type");
            static_assert(std::is_convertible
                    <
                        ring_const_type,
                        decltype(traits::exterior_ring<PolygonType>::get(*cpoly))
                    >::value,
                    "traits::exterior_ring<PolygonType>::get(const polygon_type&) must be convertible to ring_const_type");
            static_assert(std::is_convertible
                    <
                        interior_const_type,
                        decltype(traits::interior_rings<PolygonType>::get(*cpoly))
                    >::value,
                    "traits::interior_rings<PolygonType>::get(const polygon_type&) must be convertible to interior_const_type");
        }
    };

public:

    BOOST_CONCEPT_USAGE(Polygon)
    {
        checker::apply();
    }
#endif
};


/*!
\brief Checks polygon concept (const version)
\ingroup const_concepts
*/
template <typename PolygonType>
class ConstPolygon
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS

    using const_polygon_type = typename std::remove_const<PolygonType>::type;

    using ring_const_type = typename traits::ring_const_type<const_polygon_type>::type;
    using interior_const_type = typename traits::interior_const_type<const_polygon_type>::type;

    using point_type = point_type_t<const_polygon_type>;
    using ring_type = ring_type_t<const_polygon_type>;

    BOOST_CONCEPT_ASSERT( (concepts::ConstPoint<point_type>) );
    BOOST_CONCEPT_ASSERT( (concepts::ConstRing<ring_type>) );

    ////BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<interior_type>) );

    struct checker
    {
        static inline void apply()
        {
            const_polygon_type const* cpoly = nullptr;
            static_assert(std::is_convertible
                    <
                        ring_const_type,
                        decltype(traits::exterior_ring<PolygonType>::get(*cpoly))
                    >::value,
                    "traits::exterior_ring<PolygonType>::get(const polygon_type&) must be convertible to ring_const_type");
            static_assert(std::is_convertible
                    <
                        interior_const_type,
                        decltype(traits::interior_rings<PolygonType>::get(*cpoly))
                    >::value,
                    "traits::interior_rings<PolygonType>::get(const polygon_type&) must be convertible to interior_const_type");
        }
    };

public:

    BOOST_CONCEPT_USAGE(ConstPolygon)
    {
        checker::apply();
    }
#endif
};


template <typename Geometry>
struct concept_type<Geometry, polygon_tag>
{
    using type = Polygon<Geometry>;
};

template <typename Geometry>
struct concept_type<Geometry const, polygon_tag>
{
    using type = ConstPolygon<Geometry>;
};


}}} // namespace boost::geometry::concepts

#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYGON_CONCEPT_HPP
