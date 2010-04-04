// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DISSOLVE_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DISSOLVE_HPP


#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/core/point_type.hpp>

#include <boost/geometry/algorithms/dissolve.hpp>
#include <boost/geometry/algorithms/union.hpp>

#include <boost/geometry/algorithms/detail/overlay/dissolver.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace dissolve
{

template <typename Multi, typename GeometryOut>
struct dissolve_multi
{
    template <typename OutputIterator>
    static inline OutputIterator apply(Multi const& multi, OutputIterator out)
    {
        typedef typename boost::range_value<Multi>::type polygon_type;
        typedef typename boost::range_iterator<Multi const>::type iterator_type;

        // Step 1: dissolve all polygons in the multi-polygon, independantly
        std::vector<GeometryOut> step1;
        for (iterator_type it = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            dissolve_ring_or_polygon
                <
                    polygon_type,
                    GeometryOut
                >::apply(*it, std::back_inserter(step1));
        }

        // Step 2: remove mutual overlap
        std::vector<GeometryOut> step2; // TODO avoid this, output to "out", if possible
        detail::dissolver::dissolver_generic<detail::dissolver::plusmin_policy>::apply(step1, step2);
        BOOST_FOREACH(GeometryOut const& g, step2)
        {
            *out++ = g;
        }
        return out;
    }
};


// Dissolve on multi_linestring tries to create larger linestrings from input,
// or closed rings.

template <typename Multi, typename GeometryOut>
struct dissolve_multi_linestring
{
    typedef typename point_type<Multi>::type point_type;
    typedef typename boost::range_iterator<Multi const>::type iterator_type;
    typedef typename boost::range_value<Multi>::type linestring_type;
    typedef typename distance_result<point_type>::type distance_result_type;

    struct mapped
    {
        int index;
        bool is_from;
        mapped(int i, bool f) : index(i), is_from(f)
        {}
    };

    // Have a map<point, <index,start/end> > such that we can find
    // the corresponding point on each end. Note that it uses the
    // default "equals" for the point-type
    typedef std::multimap
        <
            point_type,
            mapped,
            boost::geometry::less<point_type>
        > map_type;

    typedef typename map_type::const_iterator map_iterator_type;

    static inline void copy(linestring_type const& ls, 
            GeometryOut& target, 
            bool copy_forward)
    {
        if (copy_forward)
        {
            std::copy(boost::begin(ls), boost::end(ls),
                std::back_inserter(target));
        }
        else
        {
            std::reverse_copy(boost::begin(ls), boost::end(ls),
                std::back_inserter(target));
        }
    }

    static inline map_iterator_type find_start(map_type const& map,
            std::map<int, bool>& included)
    {
        for (map_iterator_type it = map.begin();
            it != map.end();
            ++it)
        {
            int count = map.count(it->first);
            if (count == 1 && ! included[it->second.index])
            {
                included[it->second.index] = true;
                return it;
            }
        }
        return map.end();
    }

    template <typename OutputIterator>
    static inline OutputIterator apply(Multi const& multi, OutputIterator out)
    {
        if (boost::size(multi) <= 0)
        {
            return out;
        }

        map_type map;

        // 1: fill the map.
        int index = 0;
        for (iterator_type it = boost::begin(multi);
            it != boost::end(multi);
            ++it, ++index)
        {
            linestring_type const& ls = *it;
            if (boost::size(ls) > 0)
            {
                map.insert(std::make_pair(*boost::begin(ls), mapped(index, true)));
                map.insert(std::make_pair(*(boost::end(ls) - 1), mapped(index, false)));
            }
        }

        std::map<int, bool> included;

        // 2: merge (dissolve) the lines

        // 2a: start with one having a unique starting point
        map_iterator_type first = find_start(map, included);
        bool found = first != map.end();
        if (! found)
        {
            return out;
        }

        int current_index = first->second.index;
        GeometryOut current;
        copy(multi[current_index], current, first->second.is_from);

        while(found)
        {
            // 2b: get all candidates, ask for range
            point_type const& p = *(boost::end(current) - 1);
            std::pair<map_iterator_type, map_iterator_type> range
                        = map.equal_range(p);

            // 2c: for all candidates get closest one
            found = false;
            int closest_index = -1;
            bool from_is_closest = false;
            // TODO: make utility to initalize distance result with large value
            distance_result_type min_dist
                = make_distance_result<distance_result_type>(100);
            for (map_iterator_type it = range.first;
                ! found && it != range.second;
                ++it)
            {
                if (it->second.index != current_index
                    && ! included[it->second.index])
                {
                    linestring_type const& ls = multi[it->second.index];
                    point_type const& p = it->second.is_from
                        ? *boost::begin(ls)
                        : *(boost::end(ls) - 1);

                    distance_result_type d = geometry::distance(it->first, p);
                    if (! found || d < min_dist)
                    {
                        closest_index = it->second.index;
                        from_is_closest = it->second.is_from;
                        min_dist = d;

                        //std::cout << "TO " << geometry::wkt(p) << std::endl;
                    }
                    found = true;
                }
            }
            // 2d: if there is a closest one add it
            if (found && closest_index >= 0)
            {
                current_index = closest_index;
                included[current_index] = true;
                copy(multi[current_index], current, from_is_closest);
            }

            if (! found && (included.size() != boost::size(multi)))
            {
                // Get one which is NOT found and go again
                map_iterator_type next = find_start(map, included);
                found = next != map.end();

                if (found)
                {
                    current_index = next->second.index;

                    *out++ = current;
                    geometry::clear(current);

                    copy(multi[current_index], current, next->second.is_from);
                }
            }
        }
        if (boost::size(current) > 0)
        {
            *out++ = current;
        }

        return out;
    }
};


}} // namespace detail::dissolve
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template<typename Multi, typename GeometryOut>
struct dissolve<multi_polygon_tag, polygon_tag, Multi, GeometryOut>
    : detail::dissolve::dissolve_multi<Multi, GeometryOut>
{};


template<typename Multi, typename GeometryOut>
struct dissolve<multi_linestring_tag, linestring_tag, Multi, GeometryOut>
    : detail::dissolve::dissolve_multi_linestring<Multi, GeometryOut>
{};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DISSOLVE_HPP
