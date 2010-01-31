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
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>

#include <boost/geometry/geometries/concepts/detail/check_clear.hpp>
#include <boost/geometry/geometries/concepts/detail/check_append.hpp>


namespace boost { namespace geometry { namespace concept {


/*!
\brief Linestring concept
\ingroup concepts
\par Formal definition:
The linestring concept is defined as following:
- there must be a specialization of traits::tag defining linestring_tag as type
- it must behave like a Boost.Range
- either it can behave like the std library, having push_back and clear
- or it can implement a mechanism for clearing and adding points:
   - there can be a specialization of traits::use_std class indicating
     that it does not use the standard library (for modifications)
   - there should then be a specialization of traits::clear
     to make a linestring empty
   - there should then be a specialization of traits::append_point
     to add a point to a linestring

\note to fulfil the concepts, no traits class has to be specialized to
define the point type. The point type is taken using boost::range_value<X>::type

\par Example:

A custom linestring, defining the necessary specializations to fulfil to the concept.

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


public :

    BOOST_CONCEPT_USAGE(Linestring)
    {
        // Check if it can be modified
        static const bool use_std = traits::use_std
            <
                typename boost::remove_const<Geometry>::type
            >::value;

        Geometry* ls;
        detail::check_clear<Geometry, use_std>::apply(*ls);

        point_type* p;
        detail::check_append<Geometry, point_type, use_std>::apply(*ls, *p);
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
