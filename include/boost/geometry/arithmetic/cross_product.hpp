// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ARITHMETIC_CROSS_PRODUCT_HPP
#define BOOST_GEOMETRY_ARITHMETIC_CROSS_PRODUCT_HPP


#include <cstddef>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename P1, typename P2, std::size_t Dimension>
struct cross_product
{
    // We define cross product only for 2d (see Wolfram) and 3d.
    // In Math, it is also well-defined for 7-dimension.
    // Generalisation of cross product to n-dimension is defined as
    // wedge product but it is not direct analogue to binary cross product.
};

template <typename P1, typename P2>
struct cross_product<P1, P2, 2>
{
    typedef P1 return_type;

    static inline return_type apply(P1 const& p1, P2 const& p2)
    {
        assert_dimension<P1, 2>();
        assert_dimension<P2, 2>();

        // For 2-dimensions, analog of the cross product U(x,y) and V(x,y) is
        // Ux * Vy - Uy * Vx
        // which is returned as 0-component (or X) of 2d vector, 1-component is undefined.
        return_type v;
        set<0>(v, get<0>(p1) * get<1>(p2) - get<1>(p1) * get<0>(p2));
        return v;
    }
};

template <typename P1, typename P2>
struct cross_product<P1, P2, 3>
{
    typedef P1 return_type;

    static inline return_type apply(P1 const& p1, P2 const& p2)
    {
        assert_dimension<P1, 3>();
        assert_dimension<P2, 3>();

        return_type v;
        set<0>(v, get<1>(p1) * get<2>(p2) - get<2>(p1) * get<1>(p2));
        set<1>(v, get<2>(p1) * get<0>(p2) - get<0>(p1) * get<2>(p2));
        set<2>(v, get<0>(p1) * get<1>(p2) - get<1>(p1) * get<0>(p2));
        return v;
    }
};


template <typename ReturnType, typename U, typename V>
class cross_product2
{
    template <typename T>
    static inline ReturnType rt(T const& v)
    {
        return boost::numeric_cast<ReturnType>(v);
    }

public :

    // Most common dimension, as also defined by Wolfram:
    // http://mathworld.wolfram.com/CrossProduct.html
    // "In two dimensions, the analog of the cross product for u=(u_x,u_y) and v=(v_x,v_y) is
    //   uxv = det(uv)	
    //   = u_x v_y - u_y v_x"
    static inline ReturnType apply(U const& ux, U const& uy
                                 , V const& vx, V const& vy)
    {
        return rt(ux) * rt(vy) - rt(uy) * rt(vx);
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


// TODO: This is a simple draft. If relevant, it can be extended to:
// - accept vectors of different coordinate_type, but common coordinate_system
// - if vectors are of mixed 2d and 3d, lower dimension is used
// - define result_type that will generate type of vector based on:
// -- select_coordinate_type
// -- selection of lower dimension

/*!
\brief Computes the cross product of two vectors.
\details Both vectors shall be of the same type.
         This type also determines type of result vector.
\ingroup arithmetic
\param p1 first vector
\param p2 second vector
\return the cross product vector
 */
template <typename P1, typename P2>
inline P1 cross_product(P1 const& p1, P2 const& p2)
{
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<P1>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<P2>) );

    return detail::cross_product
        <
            P1, P2,
            dimension<P1>::type::value
        >::apply(p1, p2);
}


/*!
\brief Computes the cross product of two vectors, version for four values
\details Because we often have the four coordinate values (often differences)
    available, it is convenient to have a version which works directly on these,
    without having to make a (temporary) Point or Vector
\ingroup arithmetic
\return the cross product value
 */
template <typename ReturnType, typename U, typename V>
inline ReturnType cross_product2(U const& ux, U const& uy
                               , V const& vx, V const& vy)
{
    return detail::cross_product2
        <
            ReturnType, U, V
        >::apply(ux, uy, vx, vy);
}

// Synonym, because yes, sometimes the algorithm calls it "determinant"
template <typename ReturnType, typename U, typename V>
inline ReturnType determinant(U const& ux, U const& uy
                            , V const& vx, V const& vy)
{
    return detail::cross_product2
        <
            ReturnType, U, V
        >::apply(ux, uy, vx, vy);
}


// TEMPORARY, to be harmonized with cross_product
template <typename ReturnType, typename U, typename V>
inline ReturnType cross_product2(U const& u, V const& v)
{
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<U>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<V>) );

    return detail::cross_product2
        <
            ReturnType, 
            typename geometry::coordinate_type<U>::type,
            typename geometry::coordinate_type<V>::type
        >::apply(get<0>(u), get<1>(u), get<0>(v), get<1>(v));
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ARITHMETIC_CROSS_PRODUCT_HPP
