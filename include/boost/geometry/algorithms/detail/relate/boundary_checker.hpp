// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_BOUNDARY_CHECKER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_BOUNDARY_CHECKER_HPP

#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>
#include <boost/geometry/algorithms/detail/range_helpers.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

enum boundary_query { boundary_front, boundary_back, boundary_front_explicit, boundary_back_explicit, boundary_any };

template <typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type>
class boundary_checker {};

template <typename Geometry>
class boundary_checker<Geometry, linestring_tag>
{
    typedef typename point_type<Geometry>::type point_type;

public:
    boundary_checker(Geometry const& g)
        : has_boundary(! detail::equals::equals_point_point(range::front(g), range::back(g)))
        , geometry(g)
    {}

    template <boundary_query BoundaryQuery>
    bool is_boundary(point_type const& pt, segment_identifier const& sid) const
    {
        // TODO: replace with assert?
        if ( boost::size(geometry) < 2 )
            return false;

        // TODO: handle also linestrings with points_num == 2 and equals(front, back) - treat like point?

        if ( !has_boundary )
            return false;

        if ( BoundaryQuery == boundary_front_explicit )
        {
// TODO: temporarily disable
            //BOOST_ASSERT(this->template is_boundary<boundary_front>(pt, sid));
            return true;
        }
        else if ( BoundaryQuery == boundary_back_explicit )
        {
// TODO: temporarily disable
            //BOOST_ASSERT(this->template is_boundary<boundary_back>(pt, sid));
            return true;
        }
        else if ( BoundaryQuery == boundary_front )
        {
            return sid.segment_index == 0
                && detail::equals::equals_point_point(pt, range::front(geometry));
        }
        else if ( BoundaryQuery == boundary_back )
        {
            return sid.segment_index + 2 == geometry::num_points(geometry)
                && detail::equals::equals_point_point(pt, range::back(geometry));
        }
        else if ( BoundaryQuery == boundary_any )
        {
            return sid.segment_index == 0
                && detail::equals::equals_point_point(pt, range::front(geometry))
                || sid.segment_index + 2 == geometry::num_points(geometry)
                && detail::equals::equals_point_point(pt, range::back(geometry));
        }

        BOOST_ASSERT(false);
        return false;
    }

private:
    bool has_boundary;
    Geometry const& geometry;
};

template <typename Geometry>
class boundary_checker<Geometry, multi_linestring_tag>
{
    typedef typename point_type<Geometry>::type point_type;

public:
    boundary_checker(Geometry const& g)
        : is_filled(false), geometry(g)
    {}

    template <boundary_query BoundaryQuery>
    bool is_boundary(point_type const& pt, segment_identifier const& sid)
    {
        typedef typename boost::range_size<Geometry>::type size_type;
        size_type multi_count = boost::size(geometry);

        // TODO: replace with assert?
        if ( multi_count < 1 )
            return false;

        // TODO: for explicit parameters ASSERT could be used

        if ( BoundaryQuery == boundary_front || BoundaryQuery == boundary_front_explicit )
        {
            if ( sid.segment_index != 0 )
                return false;
        }

        if ( BoundaryQuery == boundary_back || BoundaryQuery == boundary_back_explicit )
        {
            if ( sid.segment_index + 2 != geometry::num_points(geometry) )
                return false;
        }

        if ( BoundaryQuery == boundary_any )
        {
            if ( sid.segment_index != 0 && sid.segment_index + 2 != geometry::num_points(geometry) )
                return false;
        }

        if ( ! is_filled )
        {
            //boundary_points.clear();
            boundary_points.reserve(multi_count * 2);

            typedef typename boost::range_iterator<Geometry const>::type multi_iterator;
            for ( multi_iterator it = boost::begin(geometry) ;
                  it != boost::end(geometry) ; ++ it )
            {
                // point - no boundary
                if ( boost::size(*it) < 2 )
                    continue;

                // TODO: handle also linestrings with points_num == 2 and equals(front, back) - treat like point?

                boundary_points.push_back(range::front(*it));
                boundary_points.push_back(range::back(*it));
            }

            std::sort(boundary_points.begin(), boundary_points.end(), geometry::less<point_type>());

            is_filled = true;
        }

        std::size_t equal_points_count
            = boost::size(
                std::equal_range(boundary_points.begin(),
                                 boundary_points.end(),
                                 pt,
                                 geometry::less<point_type>())
            );

        return equal_points_count % 2 != 0 && equal_points_count > 0; // the number is odd and > 0
                
    }

private:
    bool is_filled;
    // TODO: store references/pointers instead of points?
    std::vector<point_type> boundary_points;
    Geometry const& geometry;
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_BOUNDARY_CHECKER_HPP
