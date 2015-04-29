// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_MULTIPOINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_MULTIPOINT_HPP

#include <algorithm>
#include <utility>
#include <vector>

#include <boost/algorithm/minmax_element.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/convert_on_spheroid.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/algorithms/detail/envelope/range.hpp>
#include <boost/geometry/algorithms/dispatch/envelope.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{


class envelope_multipoint_on_spheroid
{
private:
    template <typename T>
    struct lon_lat_pair
        : std::pair<T, T>
    {
        lon_lat_pair(T const& first, T const& second)
            : std::pair<T, T>(first, second)
        {}
    };

    struct longitude_less
    {
        template <typename Pair>
        inline bool operator()(Pair const& pair1, Pair const& pair2) const
        {
            return math::smaller(pair1.first, pair2.first);
        }
    };

    struct latitude_less
    {
        template <typename Pair>
        inline bool operator()(Pair const& pair1, Pair const& pair2) const
        {
            return math::smaller(pair1.second, pair2.second);
        }
    };


    template <typename Constants, typename MultiPoint, typename OutputIterator>
    static inline void analyze_point_coordinates(MultiPoint const& multipoint,
                                                 bool& has_south_pole,
                                                 bool& has_north_pole,
                                                 OutputIterator oit)
    {
        typedef typename boost::range_iterator
            <
                MultiPoint const
            >::type iterator_type;
        typedef typename coordinate_type<MultiPoint>::type coordinate_type;

        // analyze point coordinates:
        // (1) normalize point coordinates
        // (2) check if any point is the north or the south pole
        // (3) put all non-pole points in a container
        //
        // notice that at this point in the algorithm, we have at
        // least two points on the spheroid
        has_south_pole = has_north_pole = false;

        for (iterator_type it = boost::begin(multipoint);
             it != boost::end(multipoint);
             ++it)
        {
            coordinate_type lon = geometry::get<0>(*it);
            coordinate_type lat = geometry::get<1>(*it);

            math::normalize_spheroidal_coordinates
                <
                    typename coordinate_system<MultiPoint>::type::units,
                    coordinate_type
                >(lon, lat);

            if (math::equals(lat, Constants::min_latitude()))
            {
                has_south_pole = true;
            }
            else if (math::equals(lat, Constants::max_latitude()))
            {
                has_north_pole = true;
            }
            else
            {
                *oit++ = lon_lat_pair<coordinate_type>(lon, lat);
            }
        }
    }

    template <typename SortedRange, typename Value>
    static inline Value maximum_gap(SortedRange const& sorted_range,
                                    Value& max_gap_left,
                                    Value& max_gap_right)
    {
        typedef typename boost::range_iterator
            <
                SortedRange const
            >::type iterator_type;

        iterator_type it1 = boost::begin(sorted_range), it2 = it1;
        ++it2;
        max_gap_left = it1->first;
        max_gap_right = it2->first;

        Value max_gap = max_gap_right - max_gap_left;
        for (++it1, ++it2; it2 != boost::end(sorted_range); ++it1, ++it2)
        {
            Value gap = it2->first - it1->first;
            if (math::larger(gap, max_gap))
            {
                max_gap_left = it1->first;
                max_gap_right = it2->first;
                max_gap = gap;
            }
        }

        return max_gap;
    }

    template
    <
        typename Constants,
        typename CoordinateRange,
        typename LongitudeLess,
        typename CoordinateType
    >
    static inline void get_min_max_longitudes(CoordinateRange& range,
                                              LongitudeLess const& lon_less,
                                              CoordinateType& lon_min,
                                              CoordinateType& lon_max)
    {
        typedef typename boost::range_iterator
            <
                CoordinateRange const
            >::type iterator_type;

        // compute min and max longitude values
        std::pair<iterator_type, iterator_type> min_max_longitudes
            = boost::minmax_element(boost::begin(range),
                                    boost::end(range),
                                    lon_less);

        lon_min = min_max_longitudes.first->first;
        lon_max = min_max_longitudes.second->first;

        // if the longitude span is "large" compute the true maximum gap
        if (math::larger(lon_max - lon_min, Constants::half_period()))
        {
            std::sort(boost::begin(range), boost::end(range), lon_less);

            CoordinateType max_gap_left(0), max_gap_right(0);
            CoordinateType max_gap
                = maximum_gap(range, max_gap_left, max_gap_right);

            CoordinateType complement_gap
                = Constants::period() + lon_min - lon_max;

            if (math::larger(max_gap, complement_gap))
            {
                lon_min = max_gap_right;
                lon_max = max_gap_left + Constants::period();
            }
        }
    }

    template
    <
        typename Constants,
        typename Iterator,
        typename LatitudeLess,
        typename CoordinateType
    >
    static inline void get_min_max_latitudes(Iterator const first,
                                             Iterator const last,
                                             LatitudeLess const& lat_less,
                                             bool has_south_pole,
                                             bool has_north_pole,
                                             CoordinateType& lat_min,
                                             CoordinateType& lat_max)
    {
        if (has_south_pole && has_north_pole)
        {
            lat_min = Constants::min_latitude();
            lat_max = Constants::max_latitude();
        }
        else if (has_south_pole)
        {
            lat_min = Constants::min_latitude();
            lat_max = std::max_element(first, last, lat_less)->second;
        }
        else if (has_north_pole)
        {
            lat_min = std::min_element(first, last, lat_less)->second;
            lat_max = Constants::max_latitude();
        }
        else
        {
            std::pair<Iterator, Iterator> min_max_latitudes
                = boost::minmax_element(first, last, lat_less);

            lat_min = min_max_latitudes.first->second;
            lat_max = min_max_latitudes.second->second;
        }
    }

public:
    template <typename MultiPoint, typename Box>
    static inline void apply(MultiPoint const& multipoint, Box& mbr)
    {
        typedef typename coordinate_type<MultiPoint>::type coordinate_type;
        typedef lon_lat_pair<coordinate_type> pair_type;

        typedef math::detail::constants_on_spheroid
            <
                coordinate_type,
                typename coordinate_system<MultiPoint>::type::units
            > constants;


        if (boost::empty(multipoint))
        {
            return;
        }

        if (boost::size(multipoint) == 1)
        {
            return dispatch::envelope
                <
                    typename boost::range_value<MultiPoint>::type
                >::apply(range::front(multipoint), mbr);
        }

        // analyze the coordinates and put the non-pole ones in the
        // coordinates vector
        std::vector<pair_type> coordinates;
        bool has_north_pole(false), has_south_pole(false);

        analyze_point_coordinates<constants>(multipoint,
                                             has_south_pole, has_north_pole,
                                             std::back_inserter(coordinates));

        coordinate_type lon_min, lat_min, lon_max, lat_max;
        if (coordinates.size() == 1)
        {
            // we have one non-pole point and at least one pole point
            lon_min = lon_max = coordinates.begin()->first;
            lat_min = (has_south_pole)
                ? constants::min_latitude()
                : constants::max_latitude();
            lat_max = (has_north_pole)
                ? constants::max_latitude()
                : constants::min_latitude();
        }
        else if (coordinates.size() == 0)
        {
            // all points are pole points
            BOOST_ASSERT(has_south_pole || has_north_pole);
            lon_min = lon_max = coordinate_type(0);
            lat_min = (has_south_pole)
                ? constants::min_latitude()
                : constants::max_latitude();
            lat_max = (has_north_pole)
                ? constants::max_latitude()
                : constants::min_latitude();
        }
        else
        {
            get_min_max_longitudes<constants>(coordinates,
                                              longitude_less(),
                                              lon_min,
                                              lon_max);

            get_min_max_latitudes<constants>(coordinates.begin(),
                                             coordinates.end(),
                                             latitude_less(),
                                             has_south_pole,
                                             has_north_pole,
                                             lat_min,
                                             lat_max);
        }

        math::convert_coordinates
            <
                typename coordinate_system<MultiPoint>::type::units,
                typename coordinate_system<Box>::type::units
            >(lon_min, lat_min);

        math::convert_coordinates
            <
                typename coordinate_system<MultiPoint>::type::units,
                typename coordinate_system<Box>::type::units
            >(lon_max, lat_max);

        assign_values(mbr, lon_min, lat_min, lon_max, lat_max);
    }
};


}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename MultiPoint, typename CSTag>
struct envelope<MultiPoint, multi_point_tag, CSTag>
    : detail::envelope::envelope_range<>
{};

template <typename MultiPoint>
struct envelope<MultiPoint, multi_point_tag, spherical_equatorial_tag>
    : detail::envelope::envelope_multipoint_on_spheroid
{};

template <typename MultiPoint>
struct envelope<MultiPoint, multi_point_tag, geographic_tag>
    : detail::envelope::envelope_multipoint_on_spheroid
{};


} // namespace dispatch
#endif

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_MULTIPOINT_HPP
