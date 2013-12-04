// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_SECTIONALIZE_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_SECTIONALIZE_HPP

#include <cstddef>
#include <vector>

#include <boost/concept/requires.hpp>
#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/detail/sections/sectionalize.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace sectionalize
{


template <std::size_t DimensionCount, typename Policy>
struct sectionalize_multi
{
    template <typename MultiGeometry, typename Sections>
    static inline void apply(MultiGeometry const& multi, Sections& sections,
                ring_identifier ring_id, std::size_t max_count)
    {
        ring_id.multi_index = 0;
        for (typename boost::range_iterator<MultiGeometry const>::type
                    it = boost::begin(multi);
            it != boost::end(multi);
            ++it, ++ring_id.multi_index)
        {
            Policy::apply(*it, sections, ring_id, max_count);
        }
    }
};


}} // namespace detail::sectionalize
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename MultiPolygon,
    bool Reverse,
    std::size_t DimensionCount
>
struct sectionalize<multi_polygon_tag, MultiPolygon, Reverse,  DimensionCount>
    : detail::sectionalize::sectionalize_multi
        <
            DimensionCount,
            detail::sectionalize::sectionalize_polygon
                <
                    Reverse,
                    DimensionCount
                >
        >

{};


} // namespace dispatch
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_SECTIONS_SECTIONALIZE_HPP
