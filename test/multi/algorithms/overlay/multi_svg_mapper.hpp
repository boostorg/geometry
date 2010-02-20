// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GEOMETRY_TEST_MULTI_UTIL_SVG_MAPPER_HPP
#define GEOMETRY_TEST_MULTI_UTIL_SVG_MAPPER_HPP

#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/algorithms/transform.hpp>

#include <boost/geometry/extensions/io/svg/write_svg_multi.hpp>

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename MultiPolygon>
struct svg_map<boost::geometry::multi_polygon_tag, true, MultiPolygon>
{
    template <typename TransformStrategy>
    static inline void apply(std::ostream& stream,
                    std::string const& style, int size,
                    MultiPolygon const& multi_polygon,
                    TransformStrategy const& strategy)
    {
        boost::geometry::multi_polygon<boost::geometry::polygon<boost::geometry::point_xy<int> > > impoly;
        boost::geometry::transform(multi_polygon, impoly, strategy);
        stream << boost::geometry::svg(impoly, style, size) << std::endl;
    }
};


} // namespace dispatch
#endif



#endif // GEOMETRY_TEST_MULTI_UTIL_SVG_MAPPER_HPP
