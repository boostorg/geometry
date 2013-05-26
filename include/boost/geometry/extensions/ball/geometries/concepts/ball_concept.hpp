// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_BALL_GEOMETRIES_CONCEPTS_BALL_CONCEPT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_BALL_GEOMETRIES_CONCEPTS_BALL_CONCEPT_HPP

#include <boost/concept_check.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/point_type.hpp>

namespace boost { namespace geometry { namespace concept {

/*!
    \brief Checks Ball concept (const version)
    \ingroup concepts
    \details The ConstBall concept check the same as the Ball concept,
    but does not check write access.
*/
template <typename Geometry>
class ConstBall
{
    typedef typename point_type<Geometry>::type point_type;
    typedef typename radius_type<Geometry>::type radius_type;


    template <size_t Dimension, size_t DimensionCount>
    struct dimension_checker
    {
        static void apply()
        {
            typedef typename coordinate_type<Geometry>::type coordinate_type;
            const Geometry* s = 0;
            coordinate_type coord(geometry::get<center_point, Dimension>(*s));
            boost::ignore_unused_variable_warning(coord);
            dimension_checker<Dimension + 1, DimensionCount>::apply();
        }
    };

    template <size_t DimensionCount>
    struct dimension_checker<DimensionCount, DimensionCount>
    {
        static void apply() {}
    };

public :

    BOOST_CONCEPT_USAGE(ConstBall)
    {
        static const size_t n = dimension<Geometry>::value;
        dimension_checker<0, n>::apply();
        dimension_checker<0, n>::apply();

        // Check radius access
        Geometry const* s = 0;
        radius_type coord(geometry::get<radius_value, 0>(*s));
        boost::ignore_unused_variable_warning(coord);
    }
};


/*!
    \brief Checks Ball concept
    \ingroup concepts
*/
template <typename Geometry>
class Ball
{
    BOOST_CONCEPT_ASSERT( (concept::ConstBall<Geometry>) );

    typedef typename point_type<Geometry>::type point_type;
    typedef typename radius_type<Geometry>::type radius_type;


    template <size_t Dimension, size_t DimensionCount>
    struct dimension_checker
    {
        static void apply()
        {
            Geometry* s;
            geometry::set<center_point, Dimension>(*s, geometry::get<center_point, Dimension>(*s));
            dimension_checker<Dimension + 1, DimensionCount>::apply();
        }
    };

    template <size_t DimensionCount>
    struct dimension_checker<DimensionCount, DimensionCount>
    {
        static void apply() {}
    };

public :

    BOOST_CONCEPT_USAGE(Ball)
    {
        static const size_t n = dimension<Geometry>::type::value;
        dimension_checker<0, n>::apply();
        dimension_checker<0, n>::apply();

        // Check radius access
        Geometry* s = 0;
        geometry::set<radius_value, 0>(*s, geometry::get<radius_value, 0>(*s));
    }
};



}}} // namespace boost::geometry::concept

#endif // BOOST_GEOMETRY_EXTENSIONS_BALL_GEOMETRIES_CONCEPTS_BALL_CONCEPT_HPP
