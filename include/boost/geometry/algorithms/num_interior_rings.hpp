// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_NUM_INTERIOR_RINGS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_NUM_INTERIOR_RINGS_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/core/interior_rings.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct num_interior_rings
{
    static inline std::size_t apply(Geometry const& )
    {
        return 0;
    }
};



template <typename Polygon>
struct num_interior_rings<Polygon, polygon_tag>
{
    static inline std::size_t apply(Polygon const& polygon)
    {
        return boost::size(geometry::interior_rings(polygon));
    }

};


template <typename MultiPolygon>
struct num_interior_rings<MultiPolygon, multi_polygon_tag>
{
    static inline std::size_t apply(MultiPolygon const& multi_polygon)
    {
        std::size_t n = 0;
        for (typename boost::range_iterator<MultiPolygon const>::type
            it = boost::begin(multi_polygon);
            it != boost::end(multi_polygon);
            ++it)
        {
            n += num_interior_rings
                    <
                        typename boost::range_value<MultiPolygon const>::type
                    >::apply(*it);
        }
        return n;
    }

};


} // namespace dispatch
#endif


/*!
\brief \brief_calc{number of interior rings}
\ingroup num_interior_rings
\details \details_calc{num_interior_rings, number of interior rings}.
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_calc{number of interior rings}

\qbk{[include reference/algorithms/num_interior_rings.qbk]}

\note Defined by OGC as "numInteriorRing". To be consistent with "numPoints"
    letter "s" is appended
*/
template <typename Geometry>
inline std::size_t num_interior_rings(Geometry const& geometry)
{
    return dispatch::num_interior_rings<Geometry>::apply(geometry);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_NUM_INTERIOR_RINGS_HPP
