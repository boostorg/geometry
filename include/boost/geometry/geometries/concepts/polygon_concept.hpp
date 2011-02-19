// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYGON_CONCEPT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYGON_CONCEPT_HPP

#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/ring_concept.hpp>


namespace boost { namespace geometry { namespace concept
{


/*!
\brief Polygon concept
\ingroup concepts
\par Formal definition:
The polygon concept is defined as following:
- there must be a specialization of traits::tag defining polygon_tag as type
- there must be a specialization of traits::ring_type defining the type of its
exterior ring and interior rings as type
- there must be a specialization of traits::interior_type defining the type of
the collection of its interior rings as type
- there must be a specialization of traits::exterior_ring
  with two functions named "get",
  returning the exterior ring, a const version and a mutable version
- there must be a specialization of traits::interior_rings
  with two functions named "get", returning the interior rings,
  a const version and a mutable version

\note to fulfil the concepts, no traits class has to be specialized to
define the point type. The point type is extracted from the ring type
*/
template <typename Geometry>
class Polygon
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    typedef typename point_type<Geometry>::type point_type;
    typedef typename ring_type<Geometry>::type ring_type;
    typedef typename traits::ring_const_type<Geometry>::type ring_const_type;
    typedef typename traits::ring_mutable_type<Geometry>::type ring_mutable_type;
    typedef typename traits::interior_const_type<Geometry>::type interior_const_type;
    typedef typename traits::interior_mutable_type<Geometry>::type interior_mutable_type;

    BOOST_CONCEPT_ASSERT( (concept::Point<point_type>) );
    BOOST_CONCEPT_ASSERT( (concept::Ring<ring_type>) );

    //BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<interior_type>) );

    struct checker
    {
        static inline void apply()
        {
            Geometry* poly;
            ring_mutable_type e = exterior_ring(*poly);
            interior_mutable_type i = interior_rings(*poly);

            boost::ignore_unused_variable_warning(e);
            boost::ignore_unused_variable_warning(i);
            boost::ignore_unused_variable_warning(poly);
        }

    };

public:


    BOOST_CONCEPT_USAGE(Polygon)
    {
        // Check if it can be modified
        // TODO

        //Geometry* poly;
        //clear(*poly);
        //append(*poly, point_type());

        checker::apply();
    }
#endif
};


/*!
\brief Polygon concept (const version)
\ingroup const_concepts
\details The ConstPolygon concept check the same as the Polygon concept,
but does not check write access.
*/
template <typename Geometry>
class ConstPolygon
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS

    typedef typename point_type<Geometry>::type point_type;
    typedef typename ring_type<Geometry>::type ring_type;
    typedef typename ring_return_type<Geometry>::type ring_return_type;
    typedef typename interior_return_type<Geometry>::type interior_type;
    //typedef typename traits::ring_const_type<Geometry const>::type ring_const_type;
    //typedef typename traits::interior_const_type<Geometry const>::type interior_const_type;

    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<point_type>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstRing<ring_type>) );

    ////BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<interior_type>) );

    struct checker
    {
        static inline void apply()
        {
            Geometry const* poly;
            ring_return_type ce = exterior_ring(*poly);
            interior_type ci = interior_rings(*poly);

            boost::ignore_unused_variable_warning(ce);
            boost::ignore_unused_variable_warning(ci);
            boost::ignore_unused_variable_warning(poly);
        }
    };


public:

    BOOST_CONCEPT_USAGE(ConstPolygon)
    {
        checker::apply();
    }
#endif
};

}}} // namespace boost::geometry::concept

#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYGON_CONCEPT_HPP
