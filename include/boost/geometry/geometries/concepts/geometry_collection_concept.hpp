// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_GEOMETRY_COLLECTION_CONCEPT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_GEOMETRY_COLLECTION_CONCEPT_HPP


#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>

#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/core/visit.hpp>

#include <boost/geometry/geometries/concepts/box_concept.hpp>
#include <boost/geometry/geometries/concepts/linestring_concept.hpp>
#include <boost/geometry/geometries/concepts/multi_point_concept.hpp>
#include <boost/geometry/geometries/concepts/multi_linestring_concept.hpp>
#include <boost/geometry/geometries/concepts/multi_polygon_concept.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/polygon_concept.hpp>
#include <boost/geometry/geometries/concepts/ring_concept.hpp>
#include <boost/geometry/geometries/concepts/segment_concept.hpp>

#include <boost/geometry/util/sequence.hpp>
#include <boost/geometry/util/type_traits.hpp>


namespace boost { namespace geometry { namespace concepts
{

template <typename Geometry>
class GeometryCollection
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    BOOST_CONCEPT_ASSERT( (boost::ForwardRangeConcept<Geometry>) );

    using sequence_t = typename traits::geometry_types<Geometry>::type;
    using first_t = typename util::sequence_front<sequence_t>::type;
    // NOTE: It is possible to relax this requirement.
    BOOST_GEOMETRY_STATIC_ASSERT((! util::is_geometry_collection<first_t>::value),
                                 "The first type can't be a geometry collection.",
                                 Geometry);
    BOOST_CONCEPT_ASSERT( (typename concepts::concept_type<first_t>::type) );

public:

    BOOST_CONCEPT_USAGE(GeometryCollection)
    {
        Geometry* gc = 0;
        first_t* g = 0;

        traits::clear<Geometry>::apply(*gc);
        traits::emplace_back<Geometry>::apply(*gc, *g);

        traits::iter_visit<Geometry>::apply([](auto &&) {}, boost::begin(*gc));
    }
#endif // DOXYGEN_NO_CONCEPT_MEMBERS
};


template <typename Geometry>
class ConstGeometryCollection
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    BOOST_CONCEPT_ASSERT( (boost::ForwardRangeConcept<Geometry>) );

    using sequence_t = typename traits::geometry_types<Geometry>::type;
    using first_t = typename util::sequence_front<sequence_t>::type;
    // NOTE: It is possible to relax this requirement.
    BOOST_GEOMETRY_STATIC_ASSERT((! util::is_geometry_collection<first_t>::value),
                                 "The first type can't be a geometry collection.",
                                 Geometry);
    BOOST_CONCEPT_ASSERT( (typename concepts::concept_type<first_t const>::type) );

public:

    BOOST_CONCEPT_USAGE(ConstGeometryCollection)
    {
        Geometry const* gc = 0;

        traits::iter_visit<Geometry>::apply([](auto &&) {}, boost::begin(*gc));
    }
#endif // DOXYGEN_NO_CONCEPT_MEMBERS
};


template <typename Geometry>
struct concept_type<Geometry, geometry_collection_tag>
{
    using type = GeometryCollection<Geometry>;
};

template <typename Geometry>
struct concept_type<Geometry const, geometry_collection_tag>
{
    using type = ConstGeometryCollection<Geometry>;
};


}}} // namespace boost::geometry::concepts


#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_GEOMETRY_COLLECTION_CONCEPT_HPP
