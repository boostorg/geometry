#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYHEDRALSURFACE_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYHEDRALSURFACE_HPP

#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/geometries/concepts/ring_concept.hpp>

namespace boost { namespace geometry { namespace concepts 
{


template <typename Geometry>
class PolyhedralSurface
{

#ifndef DOXYGEN_NO_CONCEPT_MEMBERS

	typedef typename ring_type<Geometry>::type ring_type;
	
	
	BOOST_CONCEPT_ASSERT( (concepts::Ring<ring_type>) );

public:
	
	BOOST_CONCEPT_USAGE(PolyhedralSurface)
	{

	}
#endif
	
};

//polyhedral surface(constant version)
template <typename Geometry>
class ConstPolyhedral
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS

	typedef typename std::remove_const<Geometry>::type const_polyhedral_type;

	typedef typename ring_type<const_polyhedral_type>::type ring_type;

	BOOST_CONCEPT_ASSERT( (concepts::ConstRing<ring_type>) );

public:

	BOOST_CONCEPT_USAGE(ConstPolyhedral)
	{

	}

#endif
};

} // namepspace concepts

} // namespace geometry

} // namespace boost
#endif 
