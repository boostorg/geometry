// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_TRANSLATE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_TRANSLATE_HPP

#include <boost/geometry/extensions/algebra/geometries/concepts/vector_concept.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>

namespace boost { namespace geometry {

namespace detail { namespace translate {

template <typename Box, typename Vector, std::size_t Dimension>
struct box_cartesian
{
    BOOST_MPL_ASSERT_MSG((0 < Dimension), INVALID_DIMENSION, (Box));

    static inline void apply(Box & box, Vector const& vector)
    {
        box_cartesian<Box, Vector, Dimension-1>::apply(box, vector);
        set<min_corner, Dimension-1>(box, get<min_corner, Dimension-1>(box) + get<Dimension-1>(vector));
        set<max_corner, Dimension-1>(box, get<max_corner, Dimension-1>(box) + get<Dimension-1>(vector));
    }
};

template <typename Box, typename Vector>
struct box_cartesian<Box, Vector, 1>
{
    static inline void apply(Box & box, Vector const& vector)
    {
        set<min_corner, 0>(box, get<min_corner, 0>(box) + get<0>(vector));
        set<max_corner, 0>(box, get<max_corner, 0>(box) + get<0>(vector));
    }
};

}} // namespace detail::translate

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

template <typename Geometry, typename Vector, typename Tag = typename tag<Geometry>::type>
struct translate
{
    BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THIS_TAG, (Tag));
};

template <typename Point, typename Vector>
struct translate<Point, Vector, point_tag>
{
    BOOST_CONCEPT_ASSERT( (concept::Point<Point>) );

    static inline void apply(Point & point, Vector const& vector)
    {
        typedef boost::mpl::bool_<
            boost::is_same<
                typename traits::coordinate_system<Point>::type,
                cs::cartesian
            >::value
        > is_cartesian;
        apply(point, vector, is_cartesian());
    }

    static inline void apply(Point & point, Vector const& vector, boost::mpl::bool_<true> /*is_cartesian*/)
    {
        for_each_coordinate(point, detail::point_operation<Vector, std::plus>(vector));
    }

    static inline void apply(Point & point, Vector const& vector, boost::mpl::bool_<false> /*is_cartesian*/)
    {
        typedef typename traits::coordinate_system<Point>::type cs;
        BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THIS_CS, (cs));
    }
};

template <typename Box, typename Vector>
struct translate<Box, Vector, box_tag>
{
    typedef typename traits::point_type<Box>::type point_type;

    BOOST_CONCEPT_ASSERT( (concept::Point<point_type>) );

    static inline void apply(Box & box, Vector const& vector)
    {
        typedef boost::mpl::bool_<
            boost::is_same<
                typename traits::coordinate_system<point_type>::type,
                cs::cartesian
            >::value
        > is_cartesian;
        apply(box, vector, is_cartesian());
    }

    static inline void apply(Box & box, Vector const& vector, boost::mpl::bool_<true> /*is_cartesian*/)
    {
        geometry::detail::translate::box_cartesian<
            Box, Vector, traits::dimension<point_type>::value
        >::apply(box, vector);
    }

    static inline void apply(Box & box, Vector const& vector, boost::mpl::bool_<false> /*is_cartesian*/)
    {
        typedef typename traits::coordinate_system<point_type>::type cs;
        BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THIS_CS, (cs));
    }
};

// TODO - other geometries

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

template <typename Geometry, typename Vector>
inline void translate(Geometry & g, Vector const& v)
{
    concept::check_concepts_and_equal_dimensions<Geometry, Vector const>();

    dispatch::translate<Geometry, Vector>::apply(g, v);
}

template <typename GeometrySrc, typename Vector, typename GeometryDst>
inline void translated(GeometrySrc const& gsrc, Vector const& v, GeometryDst & gdst)
{
    geometry::convert(gsrc, gdst);
    geometry::translate(gdst, v);
}

template <typename GeometryDst, typename GeometrySrc, typename Vector>
inline GeometryDst return_translated(GeometrySrc const& gsrc, Vector const& v)
{
    GeometryDst res;
    translated(gsrc, v, res);
    return res;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_TRANSLATE_HPP
