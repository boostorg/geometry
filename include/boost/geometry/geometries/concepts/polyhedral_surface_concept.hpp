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
	using ring_type = typename ring_type<Geometry>::type;
	
	BOOST_CONCEPT_ASSERT( (concepts::Ring<ring_type>) );
public:

	BOOST_CONCEPT_USAGE(PolyhedralSurface)
	{
	}
#endif
};

// polyhedral surface(constant version)
template <typename Geometry>
class ConstPolyhedralSurface
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
	using const_polyhedral_type = typename std::remove_const<Geometry>::type;
	using ring_type = typename ring_type<const_polyhedral_type>::type;

	BOOST_CONCEPT_ASSERT( (concepts::ConstRing<ring_type>) );

public:

	BOOST_CONCEPT_USAGE(ConstPolyhedralSurface)
	{
	}

#endif
};

}}} // namespace boost::geometry::concepts
#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_POLYHEDRALSURFACE_HPP
