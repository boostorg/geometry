// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OCCUPATION_INFO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OCCUPATION_INFO_HPP


#include <algorithm>
#include <boost/range.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/iterators/closing_iterator.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename T, typename P1, typename P2>
inline T calculate_angle(P1 const& from_point, P2 const& to_point)
{
    typedef P1 vector_type;
    vector_type v = from_point;
    geometry::subtract_point(v, to_point);
    return atan2(geometry::get<1>(v), geometry::get<0>(v));
}

template <typename Iterator, typename Vector>
inline Iterator advance_circular(Iterator it, Vector const& vector, bool forward = true)
{
	int const increment = forward ? 1 : -1;
	if (it == boost::begin(vector) && increment < 0)
	{
		it = boost::end(vector);
	}
	it += increment;
	if (it == boost::end(vector))
	{
		it = boost::begin(vector);
	}
	return it;
}

template <typename T>
struct angle_info
{
	typedef T angle_type;
    T angle;
    bool incoming;
};

template <typename AngleInfo>
class occupation_info
{
	typedef std::vector<AngleInfo> collection_type;

	struct angle_sort
	{
		inline bool operator()(AngleInfo const& left, AngleInfo const& right) const
		{
			// In this case we can compare even double using equals
			// return geometry::math::equals(left.angle, right.angle)
			return left.angle == right.angle
				? int(left.incoming) < int(right.incoming)
				: left.angle < right.angle
				;
		}
	};

    collection_type angles;
    bool m_occupied;
	bool m_calculated;

	inline bool is_occupied()
	{
		if (boost::size(angles) <= 1)
		{
			return false;
		}

		std::sort(angles.begin(), angles.end(), angle_sort());

		typedef geometry::closing_iterator<collection_type const> closing_iterator;
		closing_iterator vit(angles);
		closing_iterator end(angles, true);

		closing_iterator prev = vit++;
		for( ; vit != end; prev = vit++)
		{
			if (! geometry::math::equals(prev->angle, vit->angle)
				&& ! prev->incoming
				&& vit->incoming)
			{
				return false;
			}
		}
		return true;
	}

public :
    inline occupation_info()
        : m_occupied(false)
		, m_calculated(false)
    {}

	template <typename Point1, typename Point2>
	inline void add(Point1 const& point1, Point2 const& point2, bool incoming)
	{
        AngleInfo info;
        info.incoming = incoming;
        info.angle = calculate_angle<typename AngleInfo::angle_type>(point1, point2);
        angles.push_back(info);

		m_calculated = false;
	}

	inline bool occupied()
	{
		if (! m_calculated)
		{
			m_occupied = is_occupied();
			m_calculated = true;
		}
		return m_occupied;
	}

};


template <typename Point, typename Ring, typename Info>
inline void add_incoming_and_outgoing_angles(Point const& point,
				Ring const& ring, int segment_index,
                Info& info)
{
    typedef typename boost::range_iterator
        <
            Ring const
        >::type iterator_type;

	int const n = boost::size(ring);
	if (segment_index >= n || segment_index < 0)
	{
		return;
	}

	iterator_type it = boost::begin(ring) + segment_index;

    if (geometry::equals(point, *it))
    {
		it = advance_circular(it, ring, false);
    }

	info.add(*it, point, true);

	it = advance_circular(it, ring);
    for (int defensive_check = 0; 
		geometry::equals(point, *it) && defensive_check < n; 
		defensive_check++)
    {
		it = advance_circular(it, ring);
    }

	info.add(*it, point, false);
}


// Map in two senses of the word: it is a std::map where the key is a point.
// Per point an "occupation_info" record is kept
// Used for the buffer (but will also be used for intersections/unions having complex self-tangencies)
template <typename Point, typename OccupationInfo>
class occupation_map
{
public :
    typedef std::map<Point, OccupationInfo, geometry::less<Point> > map_type;
    map_type map;

    inline OccupationInfo& find_or_insert(Point const& point)
    {
        typename map_type::iterator it = map.find(point);
        if (it == boost::end(map))
        {
            std::pair<typename map_type::iterator, bool> pair 
                        = map.insert(std::make_pair(point, OccupationInfo()));
            it = pair.first;
        }

        return it->second;
    }

    inline bool contains(Point const& point) const
    {
        typename map_type::const_iterator it = map.find(point);
        return it != boost::end(map);
    }
};


} // namespace detail
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OCCUPATION_INFO_HPP
