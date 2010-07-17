// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_LINESTRING_CONCEPT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_LINESTRING_CONCEPT_HPP


#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/container_access.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>



namespace boost { namespace geometry { namespace concept
{


/*!
\brief Linestring concept
\ingroup concepts
\par Formal definition:
The linestring concept is defined as following:
- there must be a specialization of traits::tag defining linestring_tag as type
- it must behave like a Boost.Range
- it must implement a std::back_insert_iterator
    - either by implementing push_back
    - or by specializing std::back_insert_iterator

\note to fulfill the concepts, no traits class has to be specialized to
define the point type.

\par Example:

A custom linestring, defining the necessary specializations to fulfill to the concept.

Suppose that the following linestring is defined:
\dontinclude doxygen_5.cpp
\skip custom_linestring1
\until };

It can then be adapted to the concept as following:
\dontinclude doxygen_5.cpp
\skip adapt custom_linestring1
\until }}

\note
- There is also the registration macro BOOST_GEOMETRY_REGISTER_LINESTRING
- For registration of std::vector<P> (and deque, and list) it is enough to
include the header-file geometries/adapted/std_as_linestring.hpp. That registers
a vector as a linestring (so it cannot be registered as a linear ring then,
in the same source code).


*/

template <typename Geometry>
class Linestring
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    typedef typename point_type<Geometry>::type point_type;

    BOOST_CONCEPT_ASSERT( (concept::Point<point_type>) );
    BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<Geometry>) );

    // There should be a std::back_insert_iterator, to add points
    typedef std::back_insert_iterator<Geometry> back_inserter;

public :

    BOOST_CONCEPT_USAGE(Linestring)
    {
        // Check if it can be modified
        Geometry* ls;
        traits::clear<Geometry>::apply(*ls);
    }
#endif
};


/*!
\brief Linestring concept (const version)
\ingroup const_concepts
\details The ConstLinestring concept check the same as the Linestring concept,
but does not check write access.
*/
template <typename Geometry>
class ConstLinestring
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
    typedef typename point_type<Geometry>::type point_type;

    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<point_type>) );
    BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<Geometry>) );


public :

    BOOST_CONCEPT_USAGE(ConstLinestring)
    {
    }
#endif
};

}}} // namespace boost::geometry::concept


#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_LINESTRING_CONCEPT_HPP
