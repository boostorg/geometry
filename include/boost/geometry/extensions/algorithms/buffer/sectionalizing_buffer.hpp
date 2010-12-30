// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SECTIONALIZING_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SECTIONALIZING_BUFFER_HPP

#include <cstddef>

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/strategies/buffer_side.hpp>
#include <boost/geometry/algorithms/detail/buffer/intersecting_inserter.hpp>
#include <boost/geometry/algorithms/detail/buffer/linestring_buffer.hpp>
#include <boost/geometry/algorithms/detail/buffer/line_line_intersection.hpp>

#include <boost/geometry/algorithms/detail/sections/range_by_section.hpp>
#include <boost/geometry/algorithms/detail/sections/sectionalize.hpp>

#include <boost/geometry/algorithms/dissolve.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{



template
<
    typename GeometryOut, typename Geometry,
    typename DistanceStrategy,
    typename JoinStrategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
void sectionalizing_buffer(Geometry const& geometry,
    std::vector<GeometryOut>& buffered,
    DistanceStrategy const& distance_strategy,
    JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , Mapper& mapper
#endif
    )
{
    typedef typename point_type<Geometry>::type point_type;
    typedef geometry::box<point_type> box_type;
    typedef geometry::sections<box_type, 2> sections_type;
    typedef typename geometry::ring_type<GeometryOut>::type ring_type;

    // TEMPORARY
    typedef intersecting_inserter
        <
            std::vector<GeometryOut>
        > inserter_type;


    sections_type sections;
    geometry::sectionalize(geometry, sections);



    // Buffer all sections separately, and put them in a temporary vector
    std::vector<GeometryOut> buffered_sections;
    BOOST_FOREACH(typename sections_type::value_type const& section, sections)
    {
        if (! section.duplicate)
        {
            typedef typename boost::range_iterator
                <
                    typename geometry::range_type<Geometry>::type const
                >::type iterator_type;


            inserter_type inserter(buffered_sections);

            iterator_type begin, end;
            typedef std::pair<iterator_type, iterator_type> section_range;

TODO: UPDATE: get _ section IS OBSOLETE NOW AND DISCARDED.
TAKE range_by_section AND ADD section.begin_index/section.end_index

            geometry::get _ section(geometry, section, begin, end); // get_section is DISCARDED
            geometry::detail::buffer::linestring_buffer
                <
                    section_range, ring_type, DistanceStrategy, JoinStrategy
                >::apply(std::make_pair(begin, end), inserter,
                            distance_strategy,
                            join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            , mapper
#endif
                            );
        }
    }

    // IF there are bowl-like shapes, there can still be self-intersections
    std::vector<GeometryOut> dissolved;
    BOOST_FOREACH(GeometryOut const& p, buffered_sections)
    {
        geometry::dissolve(p, dissolved);
    }

    /*BOOST_FOREACH(GeometryOut const& p, buffered_sections)
    {
        if (geometry::intersects(p))
        {
            //std::cout << ".";
        }
    }*/

    // TEMP
    //buffered.swap(dissolved);
    //return;
    // END TEMP


    BOOST_FOREACH(GeometryOut const& p, dissolved)
    {
        if (buffered.empty())
        {
            buffered.push_back(p);
            //geometry::union_inserter<GeometryOut>(geometry, p, std::back_inserter(buffered));
        }
        else if (boost::size(buffered) == 1)
        {
            std::vector<GeometryOut> unioned;
            geometry::union_inserter<GeometryOut>(buffered.front(), p, std::back_inserter(unioned));
            buffered.swap(unioned);
        }
        else
        {
            std::cerr << " D " << buffered.size();
            /*std::vector<GeometryOut> dissolved;
            dissolved.push_back(p);
            geometry::dissolver(buffered, dissolved);
            dissolved.swap(buffered);*/
        }
    }

    // Output
}


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SECTIONALIZING_BUFFER_HPP
