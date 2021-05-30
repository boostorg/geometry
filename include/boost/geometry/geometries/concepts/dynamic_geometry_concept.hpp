// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_DYNAMIC_GEOMETRY_CONCEPT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_DYNAMIC_GEOMETRY_CONCEPT_HPP


#include <boost/concept_check.hpp>

#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/visit.hpp>

#include <boost/geometry/geometries/concepts/box_concept.hpp>
#include <boost/geometry/geometries/concepts/geometry_collection_concept.hpp>
#include <boost/geometry/geometries/concepts/linestring_concept.hpp>
#include <boost/geometry/geometries/concepts/multi_point_concept.hpp>
#include <boost/geometry/geometries/concepts/multi_linestring_concept.hpp>
#include <boost/geometry/geometries/concepts/multi_polygon_concept.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/polygon_concept.hpp>
#include <boost/geometry/geometries/concepts/ring_concept.hpp>
#include <boost/geometry/geometries/concepts/segment_concept.hpp>

#include <boost/geometry/util/sequence.hpp>


namespace boost { namespace geometry { namespace concepts
{

template <typename Geometry>
class DynamicGeometry
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    using sequence_t = typename traits::geometry_types<Geometry>::type;
    using first_t = typename util::sequence_front<sequence_t>::type;
    BOOST_CONCEPT_ASSERT( (typename concepts::concept_type<first_t>::type) );

public:

    BOOST_CONCEPT_USAGE(DynamicGeometry)
    {
        Geometry* dg = 0;
        first_t* g = 0;

        *dg = *g;

        traits::visit<Geometry>::apply([](auto &&) {}, *dg);
    }
#endif // DOXYGEN_NO_CONCEPT_MEMBERS
};


template <typename Geometry>
class ConstDynamicGeometry
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    using sequence_t = typename traits::geometry_types<Geometry>::type;
    using first_t = typename util::sequence_front<sequence_t>::type;
    BOOST_CONCEPT_ASSERT( (typename concepts::concept_type<first_t const>::type) );

public:

    BOOST_CONCEPT_USAGE(ConstDynamicGeometry)
    {
        Geometry const* dg = 0;

        traits::visit<Geometry>::apply([](auto &&) {}, *dg);
    }
#endif // DOXYGEN_NO_CONCEPT_MEMBERS
};


template <typename Geometry>
struct concept_type<Geometry, dynamic_geometry_tag>
{
    using type = DynamicGeometry<Geometry>;
};

template <typename Geometry>
struct concept_type<Geometry const, dynamic_geometry_tag>
{
    using type = ConstDynamicGeometry<Geometry>;
};


}}} // namespace boost::geometry::concepts


#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_DYNAMIC_GEOMETRY_CONCEPT_HPP
