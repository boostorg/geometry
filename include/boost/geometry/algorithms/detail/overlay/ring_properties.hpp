// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALG_DET_OV_RING_PROPERTIES_HPP
#define BOOST_GEOMETRY_ALG_DET_OV_RING_PROPERTIES_HPP

#define BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
#ifdef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
#include <deque>
#else
#include <boost/array.hpp>
#endif

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
    struct parent
    {
        int c;
        ring_identifier id;

        inline parent()
            : c(0)
        {}

        inline parent(int c_, ring_identifier i)
            : c(c_), id(i)
        {}
    };

#ifdef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
    inline void push(parent const& p)
    {
        parents.push_back(p);
    }
    inline void pop()
    {
        parents.pop_back();
    }
    inline bool parent_stack_empty() const
    {
        return parents.empty();
    }
#else
    inline void push(parent const& p)
    {
        parents[array_size++] = p;
    }
    inline void pop()
    {
        array_size--;
    }
    inline bool parent_stack_empty() const
    {
        return array_size == 0;
    }
#endif


    inline bool interior_included(int direction) const
    {
        if (area < 0 && ! parent_stack_empty())
        {
            // Inner rings are included if the last encountered parent
            // matches the operation
#ifdef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
            int d = parents.back().c;
            if (d == 0
                && parents.size() == 2
                && parents.front().c == 0)
#else
            int d = parents[array_size - 1].c;
            if (d == 0
                && array_size == 2
                && parents[0].c == 0)
#endif
            {
                // It is contained by two outer rings, both of them
                // are not included in the other. So they must be
                // equal. Then always included, both in union and
                // in intersection.
                return direction == -1;
            }

            if (d == 0)
            {
                d = 1;
            }
            return d * direction == 1;
        }
        return false;
    }

    inline bool included(int direction) const
    {
        if (produced)
        {
            // Traversed rings are included in all operations,
            // because traversal was direction-dependant.
            return true;
        }
        if (intersects)
        {
            // Original rings which have intersections should
            // always be skipped
            return false;
        }

        if (area > 0)
        {
            // Outer rings are included if they don't have parents
            // (union) or have parents (intersection)
            return (parent_stack_empty() ? 1 : -1) * direction == 1;
        }
        else if (area < 0 && ! parent_stack_empty())
        {
            // Inner rings are included if the last encountered parent
            // matches the operation
            return interior_included(direction);
        }
        return false;
    }

    typedef Point point_type;
    typedef geometry::box<Point> box_type;

    ring_identifier ring_id;
    typename area_result<Point>::type area;

    int c;
    bool intersects;
    bool produced;

    // Stack of non-intersecting parent rings.
    // This denotes if it a negative ring should be included,
    // and which is the parent.
#ifdef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
    std::deque<parent> parents;
#else
    parent parents[3];
    int array_size;
#endif

    // If the parent is an intersecting ring, take
    // this as the parent.
    ring_identifier parent_id;
    bool has_parent_id;


    box_type box;

    point_type point;
    bool has_point;

    inline ring_properties()
        : c(0)
        , intersects(false)
        , produced(false)
        , has_point(false)
        , has_parent_id(false)
#ifndef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
        , array_size(0)
#endif
    {}

    template <typename Geometry>
    inline ring_properties(ring_identifier id, Geometry const& geometry,
            bool i, bool p = false)
        : ring_id(id)
        , area(geometry::area(geometry))
        , c(0)
        , intersects(i)
        , produced(p)
        , has_parent_id(false)
        , box(geometry::make_envelope<box_type>(geometry))
#ifndef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
        , array_size(0)
#endif
    {
        has_point = geometry::point_on_border(point, geometry);
    }

    inline bool operator<(ring_properties<Point> const& other) const
    {
        // Normal sorting: in reverse order
        return std::abs(area) > std::abs(other.area);
    }

    inline ring_identifier const& id() const
    {
        if (has_parent_id)
        {
            return parent_id;
        }
        return parent_stack_empty() || area > 0
            ? ring_id
#ifdef BOOST_GEOMETRY_ASSEMBLE_PARENT_DEQUE
            : parents.back().id;
#else
            : parents[array_size - 1].id;
#endif
    }


};


template<typename Prop>
struct sort_on_id_or_parent_id
{
    inline bool operator()(Prop const& left, Prop const& right) const
    {
        ring_identifier const& left_id = left.id();
        ring_identifier const& right_id = right.id();

        // If it is the same, sort on size descending
        return left_id == right_id
            ? std::abs(left.area) > std::abs(right.area)
            : left_id < right_id;
    }
};



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALG_DET_OV_RING_PROPERTIES_HPP
