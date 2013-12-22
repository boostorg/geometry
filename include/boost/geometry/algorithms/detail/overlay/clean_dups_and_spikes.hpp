// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_CLEAN_DUPS_AND_SPIKES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_CLEAN_DUPS_AND_SPIKES_HPP

#include <boost/range.hpp>

#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

// This is refactored from remove_spikes. This function considers rescaled points
template <typename Range, typename RescalePolicy>
static inline void clean_dups_and_spikes(Range& range,
                RescalePolicy const& rescale_policy)
{
    std::size_t n = boost::size(range);
    if (n < core_detail::closure::minimum_ring_size
                    <
                        geometry::closure<Range>::value
                    >::value)
    {
        return;
    }

    typedef typename point_type<Range>::type point_type;

    typedef std::pair
    <
        point_type,
        typename geometry::robust_point_type
            <
                point_type,
                RescalePolicy
            >::type
    > point_pair;

    std::deque<point_pair> cleaned;
    for (typename boost::range_iterator<Range const>::type it = boost::begin(range);
        it != boost::end(range); ++it)
    {
        point_pair pp;
        pp.first = *it;
        geometry::recalculate(pp.second, pp.first, rescale_policy);

        // Add point
        cleaned.push_back(pp);

        while(cleaned.size() >= 3
                && point_is_spike_or_equal(cleaned.back().second,
                    (cleaned.end() - 3)->second,
                    (cleaned.end() - 2)->second))
        {
            // Remove pen-ultimate point causing the spike (or which was equal)
            cleaned.erase(cleaned.end() - 2);
        }
    }

    // For a closed-polygon, remove closing point
    // this makes checking first point(s) easier and consistent
    if (geometry::closure<Range>::value == geometry::closed)
    {
        cleaned.pop_back();
    }

    bool found = false;
    do
    {
        found = false;

        // Check for spike in first point
        int const penultimate = 2;
        while(cleaned.size() > 3
                && point_is_spike_or_equal(cleaned.front().second,
                    (cleaned.end() - penultimate)->second,
                    cleaned.back().second))
        {
            cleaned.pop_back();
            found = true;
        }

        // Check for spike in second point
        while(cleaned.size() > 3
                && point_is_spike_or_equal((cleaned.begin() + 1)->second,
                    cleaned.back().second,
                    cleaned.front().second))
        {
            cleaned.pop_front();
            found = true;
        }
    }
    while (found);


    // Create new output
    geometry::clear(range);
    range.reserve(n);
    for (typename boost::range_iterator<std::deque<point_pair> const>::type it = boost::begin(cleaned);
        it != boost::end(cleaned); ++it)
    {
        range.push_back(it->first);
    }

    // Close if necessary
    if (geometry::closure<Range>::value == geometry::closed)
    {
        point_type const first = range.front();
        range.push_back(first);
    }
}



}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_CLEAN_DUPS_AND_SPIKES_HPP
