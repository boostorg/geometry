#ifndef BOOST_GEOMETRY_GEOMETRIES_POLYHEDRALSURFACE_HPP
#define BOOST_GEOMETRY_GEOMETRIES_POLYHEDRALSURFACE_HPP

#include <memory>
#include <vector>

#include <boost/concept/assert.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/config.hpp>
#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
#include <initializer_list>
#endif

namespace boost { namespace geometry 
{

namespace model
{


template 
<
    typename Ring,
    template<typename, typename> class Container = std::vector,
    template<typename> class Allocator = std::allocator

>
class PolyhedralSurface : public Container<Ring, Allocator<Ring> >
{
	BOOST_CONCEPT_ASSERT( (concepts::Ring<Ring>) );

public :

    using point_type = model::point<double, 3, boost::geometry::cs::cartesian>;
    using ring_type = ring<point_type, true, true>;
    using ph = Container<ring_type, Allocator<ring_type> >;

#ifndef BOOST_NO_CXX11_HDR_INITIALIZER_LIST

    /// \constructor_default{polyhedron}
    inline PolyhedralSurface()
	    : ph()
	{}

    /// \constructor_initialized_list{polyhedron}
	inline PolyhedralSurface(std::initializer_list<ring_type> l)
	    : ph(l.begin(), l.end())
	{}

#endif
	
};

} // namespace model

#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename Ring,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct tag
<
    model::PolyhedralSurface
        <
            Ring, 
            Container, Allocator
        > 
>
{
	typedef polyhedral_surface_tag type;
};

template
<
    typename Ring,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct Poly_ring_type
<
    model::PolyhedralSurface
        <
           Ring, 
           Container, Allocator
        >
>
{
	typedef typename model::PolyhedralSurface
	    <
	        Ring, 
	        Container, Allocator
	    >::ring_type& type;
};

/*template 
<
    typename Ring,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct ring_type
<
    model::PolyhedralSurface
        < 
            Ring,
            Container, Allocator
        >
>
{
    typedef typename model::PolyhedralSurface
        <
            Ring,
            Container, Allocator
        >::ring_type& type;

};*/


} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif
