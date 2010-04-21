// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_CONNECT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_CONNECT_HPP

#include <map>


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/strategies/distance_result.hpp>
#include <boost/geometry/policies/compare.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace connect
{

// Dissolve on multi_linestring tries to create larger linestrings from input,
// or closed rings.

template <typename Multi, typename GeometryOut>
struct connect_multi_linestring
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
            std::map<int, bool>& included, int expected_count = 1)
    {
        for (map_iterator_type it = map.begin();
            it != map.end();
            ++it)
        {
            typename map_type::size_type count = map.count(it->first);
            if (count == expected_count && ! included[it->second.index])
            {
                included[it->second.index] = true;
                return it;
            }
        }

        // Not found with one point, try one with two points
        // to find rings
        if (expected_count == 1)
        {
            return find_start(map, included, 2);
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

        // 2: connect the lines

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
            // 2b: get all candidates, by asking multi-map for range
            point_type const& p = *(boost::end(current) - 1);
            std::pair<map_iterator_type, map_iterator_type> range
                        = map.equal_range(p);

            // Alternatively, we might look for the closest points
            if (range.first == map.end())
            {
                std::cout << "nothing found" << std::endl;
            }

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

}} // namespace detail::connect
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    typename GeometryOutTag,
    typename Geometry,
    typename GeometryOut
>
struct connect
{};


template<typename Multi, typename GeometryOut>
struct connect<multi_linestring_tag, linestring_tag, Multi, GeometryOut>
    : detail::connect::connect_multi_linestring<Multi, GeometryOut>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template
<
    typename Geometry,
    typename Collection
>
inline void connect(Geometry const& geometry, Collection& output_collection)
{
    concept::check<Geometry const>();

    typedef typename boost::range_value<Collection>::type geometry_out;

    concept::check<geometry_out>();

    dispatch::connect
    <
        typename tag<Geometry>::type,
        typename tag<geometry_out>::type,
        Geometry,
        geometry_out
    >::apply(geometry, std::back_inserter(output_collection));
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_CONNECT_HPP
