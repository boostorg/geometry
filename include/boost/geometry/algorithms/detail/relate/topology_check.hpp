// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014-2017, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TOPOLOGY_CHECK_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TOPOLOGY_CHECK_HPP

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/algorithms/detail/equals/point_point.hpp>
#include <boost/geometry/policies/compare.hpp>

#include <boost/geometry/util/has_nan_coordinate.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

// TODO: change the name for e.g. something with the word "exterior"

template <typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type>
struct topology_check
    : not_implemented<Tag>
{};

//template <typename Point>
//struct topology_check<Point, point_tag>
//{
//    static const char interior = '0';
//    static const char boundary = 'F';
//
//    static const bool has_interior = true;
//    static const bool has_boundary = false;
//
//    topology_check(Point const&) {}
//    template <typename IgnoreBoundaryPoint>
//    topology_check(Point const&, IgnoreBoundaryPoint const&) {}
//};

template <typename Linestring>
struct topology_check<Linestring, linestring_tag>
{
    static const char interior = '1';
    static const char boundary = '0';

    bool has_interior;
    bool has_boundary;

    topology_check(Linestring const& ls)
        : m_ls(ls)
    {
        std::size_t count = boost::size(ls);
        has_interior = count > 0;
        // NOTE: Linestring with all points equal is treated as 1d linear ring
        has_boundary = count > 1
                    && ! detail::equals::equals_point_point(range::front(ls), range::back(ls));
    }

    /*template <typename Visitor>
    void for_each_boundary_point(Visitor& visitor) const
    {
        if (has_boundary)
        {
            visitor.apply(range::front(m_ls));
            visitor.apply(range::back(m_ls));
        }
    }*/

    Linestring const& m_ls;
};

template <typename MultiLinestring>
struct topology_check<MultiLinestring, multi_linestring_tag>
{
    static const char interior = '1';
    static const char boundary = '0';

    bool has_interior;
    bool has_boundary;

    typedef typename geometry::point_type<MultiLinestring>::type point_type;
    std::vector<point_type> endpoints;

    topology_check(MultiLinestring const& mls)
    {
        endpoints.reserve(boost::size(mls) * 2);

        typedef typename boost::range_iterator<MultiLinestring const>::type ls_iterator;
        for ( ls_iterator it = boost::begin(mls) ; it != boost::end(mls) ; ++it )
        {
            typename boost::range_reference<MultiLinestring const>::type
                ls = *it;

            std::size_t count = boost::size(ls);

            if (count > 0)
            {
                has_interior = true;
            }

            if (count > 1)
            {
                typedef typename boost::range_reference
                    <
                        typename boost::range_value<MultiLinestring const>::type const
                    >::type point_reference;
                
                point_reference front_pt = range::front(ls);
                point_reference back_pt = range::back(ls);

                // don't store boundaries of linear rings, this doesn't change anything
                if (! equals::equals_point_point(front_pt, back_pt))
                {
                    // do not add points containing NaN coordinates
                    // because they cannot be reasonably compared, e.g. with MSVC
                    // an assertion failure is reported in std::equal_range()
                    // NOTE: currently ignoring_counter calling std::equal_range()
                    //   is not used anywhere in the code, still it's safer this way
                    if (! geometry::has_nan_coordinate(front_pt))
                    {
                        endpoints.push_back(front_pt);
                    }
                    if (! geometry::has_nan_coordinate(back_pt))
                    {
                        endpoints.push_back(back_pt);
                    }
                }
            }
        }

        has_boundary = false;

        if ( !endpoints.empty() )
        {
            std::sort(endpoints.begin(), endpoints.end(), geometry::less<point_type>());
            has_boundary = find_odd_count(endpoints.begin(), endpoints.end());
        }
    }

    template <typename It>
    static inline bool find_odd_count(It first, It last)
    {
        interrupting_visitor visitor;
        for_each_boundary_point(first, last, visitor);
        return visitor.found;
    }

    struct interrupting_visitor
    {
        bool found;
        interrupting_visitor() : found(false) {}
        template <typename Point>
        bool apply(Point const&)
        {
            found = true;
            return false;
        }
    };

    template <typename Visitor>
    void for_each_boundary_point(Visitor& visitor) const
    {
        for_each_boundary_point(endpoints.begin(), endpoints.end(), visitor);
    }

    template <typename It, typename Visitor>
    static void for_each_boundary_point(It first, It last, Visitor& visitor)
    {
        if ( first == last )
            return;

        std::size_t count = 1;
        It prev = first;
        ++first;
        for ( ; first != last ; ++first, ++prev )
        {
            // the end of the equal points subrange
            if ( ! equals::equals_point_point(*first, *prev) )
            {
                // odd count -> boundary
                if ( count % 2 != 0 )
                {
                    if (! visitor.apply(*prev))
                    {
                        return;
                    }
                }

                count = 1;
            }
            else
            {
                ++count;
            }
        }

        // odd count -> boundary
        if ( count % 2 != 0 )
        {
            visitor.apply(*prev);
        }
    }
};

template <typename Ring>
struct topology_check<Ring, ring_tag>
{
    static const char interior = '2';
    static const char boundary = '1';
    static const bool has_interior = true;
    static const bool has_boundary = true;

    topology_check(Ring const&) {}
};

template <typename Polygon>
struct topology_check<Polygon, polygon_tag>
{
    static const char interior = '2';
    static const char boundary = '1';
    static const bool has_interior = true;
    static const bool has_boundary = true;

    topology_check(Polygon const&) {}
};

template <typename MultiPolygon>
struct topology_check<MultiPolygon, multi_polygon_tag>
{
    static const char interior = '2';
    static const char boundary = '1';
    static const bool has_interior = true;
    static const bool has_boundary = true;

    topology_check(MultiPolygon const&) {}
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TOPOLOGY_CHECK_HPP
