// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALG_DET_OV_RING_PROPERTIES_HPP
#define BOOST_GEOMETRY_ALG_DET_OV_RING_PROPERTIES_HPP


#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/algorithms/detail/ring_identifier.hpp>

#include <boost/geometry/geometries/box.hpp>


namespace boost { namespace geometry
{


template <typename Point>
struct ring_properties
{
    typedef Point point_type;
    typedef geometry::box<Point> box_type;

    ring_identifier ring_id;
    typename area_result<Point>::type area;
    int signum;

    bool intersects;
    bool produced;

    // "Stack"/counter of non-intersecting parent rings.
    // This denotes if it a negative ring should be included,
    int parent_count;

    // ID of the parent
    ring_identifier parent_ring_id;

    box_type box;

    point_type point;
    bool has_point;

    // Default constructor (necessary for vector, but not called)
    inline ring_properties()
        : intersects(false)
        , produced(false)
        , parent_count(0)
        , has_point(false)
    {
        parent_ring_id.source_index = -1;
    }

    template <typename Geometry>
    inline ring_properties(ring_identifier id, Geometry const& geometry,
            bool i, bool p = false)
        : ring_id(id)
        , area(geometry::area(geometry))
        , intersects(i)
        , produced(p)
        , parent_count(0)
        , box(geometry::make_envelope<box_type>(geometry))
    {
        has_point = geometry::point_on_border(point, geometry, true);
        typedef typename coordinate_type<Geometry>::type coordinate_type;
        coordinate_type zero = coordinate_type();
        signum = area > zero ? 1 : area < zero ? -1 : 0;
        parent_ring_id.source_index = -1;
    }

    inline bool positive() const { return signum == 1; }
    inline bool negative() const { return signum == -1; }

    inline bool operator<(ring_properties<Point> const& other) const
    {
        // Normal sorting: in reverse order
        return abs(area) > abs(other.area);
    }

    inline ring_identifier const& id(int direction) const
    {
        // Return the id of ifself, or of the parent
        return positive() || parent_ring_id.source_index < 0
            ? ring_id
            : parent_ring_id;
    }


    inline void push(ring_properties<Point> const& r,
                int direction, bool dissolve)
    {
        if (//(r.produced || r.untouched()) &&
            r.included(direction, dissolve))
        {
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << " id.push " << r.ring_id;
#endif
            parent_ring_id = r.ring_id;
        }
        if (! r.produced || dissolve)
        {
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << " or.push " << r.ring_id;
#endif
            parent_count++;
        }
    }


    inline void pop(ring_properties<Point> const& r)
    {
        if (! r.produced && parent_count > 0)
        {
#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << " or.pop";
#endif

            parent_count--;
        }
    }


    inline bool interior_included(int direction) const
    {
        if (negative())
        {
            // Original inner rings are included if there
            // are two untouched parents (union) or one (intersection);
            // Produced are ones are included if there is a parent found
            if (produced)
            {
                return parent_count > 0;
            }
            return
                (direction == 1  && parent_count == 1)
                || (direction == -1 && parent_count > 1);
        }
        return false;
    }

    inline bool included(int direction, bool dissolve) const
    {
        if (produced && ! dissolve)
        {
            // Traversed rings are included in all operations,
            // because traversal was direction-dependant.
            // On dissolve, this is not the case.
            return true;
        }
        if (intersects)
        {
            // Original rings which have intersections should
            // always be skipped
            return false;
        }

        if (positive())
        {
            // Outer rings are included if they don't have parents
            // (union) or have parents (intersection)
            if (produced)
            {
                return parent_count == 0;
            }
            return
                (direction == 1  && parent_count == 0)
                || (direction == -1 && parent_count > 0);
        }
        else if (negative())
        {
            // Inner rings are included if the last encountered parent
            // matches the operation
            return interior_included(direction);
        }
        return false;
    }

    inline bool untouched() const
    {
        // It should be in comparisons on parent/child if:
        // it is produced
        // it is not produced, and not intersecting
        return ! produced && ! intersects;
    }


#if defined(BOOST_GEOMETRY_DEBUG_IDENTIFIER)
    friend std::ostream& operator<<(std::ostream &os, ring_properties<Point> const& prop)
    {
        os << "prop: " << prop.ring_id << " " << prop.area;
        os << " count: " << prop.parent_count;
        std::cout << " parent: " << prop.parent_ring_id;
        if (prop.produced) std::cout << " produced";
        if (prop.intersects) std::cout << " intersects";
        if (prop.included(1, false)) std::cout << " union";
        if (prop.included(-1, false)) std::cout << " intersection";
        return os;
    }
#endif

};


template<typename Prop>
struct sort_on_id_or_parent_id
{
private :
    int m_direction;
public :
    sort_on_id_or_parent_id(int direction)
        : m_direction(direction)
    {}

    inline bool operator()(Prop const& left, Prop const& right) const
    {
        ring_identifier const& left_id = left.id(m_direction);
        ring_identifier const& right_id = right.id(m_direction);

        // If it is the same, sort on size descending
        return left_id == right_id
            ? abs(left.area) > abs(right.area)
            : left_id < right_id;
    }
};



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALG_DET_OV_RING_PROPERTIES_HPP
