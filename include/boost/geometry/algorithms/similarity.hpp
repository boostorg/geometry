// Boost.Geometry

// Copyright (c) 2023 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_ALGORITHMS_SIMILARITY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_SIMILARITY_HPP

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/length.hpp>

#include <cstdint>
#include <vector>

// #define DEBUG_EXTRA_RING
#include <boost/geometry/algorithms/perimeter.hpp>

namespace boost { namespace geometry
{

template <typename CoordinateType>
struct similarity_info
{
    CoordinateType distance = 0;
    bool is_reversed = false;
}; 

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace similarity
{

// Structure containing projections of a geometry on another geometry,
// and additionally the points of the geometry itself
template <typename Point>
struct projection
{
    using coor_t = typename coordinate_type<Point>::type;

    // Indicates if it is a projection of the other geometry, or a point of the own geometry.
    bool is_projection = true;

    // For a projection of P on Q, this is the index in P
    // For a non projected point, it is also the index in P
    std::size_t source_index = 0;

    // For a projection of P on Q, this is the index of a segment in Q
    // For a non projected point, it is identical to source_index
    std::size_t segment_index = 0;

    // For a projection of P on Q, this is the offset on the segment in Q
    // For a non projected point, the offset is always 0.0
    coor_t offset = 0;

    // For a projection of P on Q, this is the distance from point P 
    // to its projection in Q.
    // For a non projected point, it is 0.0
    coor_t distance = 0;

    std::size_t sort_index = 0;

    // The projected point, or the point of the geometry itself
    Point point;
};

template <typename Point, typename Geometry>
auto get_projection(std::size_t index, Point const& point, Geometry const& target)
{
    using coor_t = typename coordinate_type<Point>::type;
    using closest_strategy = geometry::strategy::closest_points::detail::compute_closest_point_to_segment<coor_t>;
    projection<Point> result;

    for (std::size_t i = 0; i + 1 < target.size(); i++)
    {
        auto const& current = target[i];
        auto const& next = target[i + 1];
        projection<Point> other;
        auto const pp = closest_strategy::apply(point, current, next);
        geometry::convert(pp, other.point);
        other.distance = geometry::comparable_distance(point, other.point);
        if (i == 0 || other.distance < result.distance)
        {
            result = other;
            result.is_projection = true;
            result.source_index = index;
            result.segment_index = i;
            result.offset = geometry::distance(current, result.point);
        }
    }
    result.distance = geometry::distance(result.point, point);
    return result;
}

// Calculate projected points of all points of "source" on "target"
// The result is therefore similar to target.
template <typename Geometry1, typename Geometry2>
auto get_projections(Geometry1 const& source, Geometry2 const& target)
{
    std::vector<projection<typename point_type<Geometry1>::type>> result;
    result.resize(source.size());
    detail::for_each_with_index(source, [&target, &result](std::size_t index, const auto& point)
    {
        result[index] = get_projection(index, point, target);
    });
    return result;
}

// Reverse in the projections: the segment index and the offset.
// All the rest (source_index, distance, point) is the same for the reversed version.
// >------+-----+------> 4 points, 3 segments, last segment index is 2
//   *                   segment_index 0, reverse: 2 (2 - 0)
//           *           segment_index 1, reverse: 1 (2 - 1)
//                  *    segment_index 2, reverse: 0 (2 - 2)
template <typename Projections, typename Geometry2>
void reverse_projections(Geometry2 const& target, Projections& projections)
{
    std::size_t const last_segment_index = target.size() - 2;
    for (auto& projection : projections)
    {
        auto const si = projection.segment_index;
        if (si > last_segment_index)
        {
            // Defensive check.
            continue;
        }
        auto const segment_length = geometry::distance(target[si], target[si + 1]);
        projection.segment_index = last_segment_index - si;
        projection.offset = segment_length - projection.offset;
    }
}

// Add "self" points on collection which was created projecting an other line
// onto this line.
// Sort all points along the line.
template <typename It, typename Enriched>
void enrich_with_self(It begin, It end, Enriched& enriched)
{
    using info_t = std::decay_t<decltype(enriched.front())>;
    std::size_t index = 0;
    for (It it = begin; it != end; ++it)
    {
        info_t info;
        info.is_projection = false;
        info.source_index = index;
        info.segment_index = index;
        info.point = *it;
        enriched.push_back(std::move(info));
        index++;
    }

    std::sort(enriched.begin(), enriched.end(), 
        [](const auto& lhs, const auto& rhs){
            auto const lt = std::tie(lhs.segment_index, lhs.offset, lhs.source_index, lhs.is_projection);
            auto const rt = std::tie(rhs.segment_index, rhs.offset, rhs.source_index, rhs.is_projection);
            return lt < rt;
        });
}

template <typename Projections>
bool is_reversed(Projections const& projections)
{
    std::size_t count_right_order = 0;
    std::size_t count_wrong_order = 0;
    for (std::size_t i = 0; i + 1 < projections.size(); i++)
    {
        auto const current = std::tie(projections[i].segment_index, projections[i].offset);
        auto const next = std::tie(projections[i + 1].segment_index, projections[i + 1].offset);

        if (current <= next)
        {
            count_right_order++;
        }
        else
        {
            count_wrong_order++;
        }
    }

    return count_wrong_order > count_right_order;
}

// Makes a triangle. The first is supposed to form a segment.
template <typename Projection, typename Ring>
int fill_triangle(Projection const& s0, Projection const& s1, Projection const& p,Ring& triangle)
{
    using point_t = typename point_type<Ring>::type;
    using side_strategy = typename strategy::side::services::default_strategy
    <
        typename cs_tag<point_t>::type
    >::type;

    int const side = side_strategy::apply(s0.point, s1.point, p.point);

    if (side == 0)
    {
        // It is collinear. This happens a lot when the inputs are the same.
        // Constructing the triangle and calculating its area can be skipped.
        return 0;
    }


    // Construct the triangle clockwise.
    if (side == -1)
    {
        triangle[0] = s0.point;
        triangle[1] = s1.point;
        triangle[2] = p.point;
    }
    else
    {
        triangle[0] = s1.point;
        triangle[1] = s0.point;
        triangle[2] = p.point;
    }

    triangle[3] = triangle[0];

    return 1;
}

template <typename It, typename Ring, typename Visitor>
auto get_areal_sum(It p_it, It p_end, It q_it, It q_end, Ring& triangle, Visitor& visitor)
{
    using point_t = typename boost::geometry::point_type<Ring>::type;
    using side_strategy = typename boost::geometry::strategy::side::services::default_strategy
    <
        typename boost::geometry::cs_tag<point_t>::type
    >::type;

    std::vector<point_t> left, right;
    int ring_code = 0;
    typename coordinate_type<Ring>::type sum_area = 0;

    auto add_triangle = [&sum_area, &triangle, &visitor](int source_index, auto const& s0, auto const& s1, auto const& p)
    {
        int const code = fill_triangle(s0, s1, p, triangle);
        if (code != 0)
        {
            auto const area = geometry::area(triangle);

            // The area should be positive - taking the abs is a defensive check.
            // In some cases the area is subtracted.
            sum_area += code * std::abs(area);

            visitor.visit_triangle(triangle, s0, s1, p, source_index, code);
        }
    };

    auto finish_ring = [&]()
    {
        if (left.empty() || right.empty())
        {
            return;
        }

        if (ring_code == 1 || ring_code == 4)
        {
            std::reverse(left.begin(), left.end());
        }
        else if (ring_code == 2 || ring_code == 3)
        {
            std::reverse(right.begin(), right.end());
        }

        Ring extra_ring(left.begin(), left.end());
        for (const auto& p : right)
        {
            extra_ring.push_back(p);
        }
        extra_ring.push_back(left.front());

        auto const area = std::abs(geometry::area(extra_ring));
        sum_area += area;
#if 0
        std::cout << " END RING " << area;
#endif            
        std::decay_t<decltype(*p_it)> dummy;
        visitor.visit_triangle(extra_ring, dummy, dummy, dummy, 0, 0);

        left.clear();
        right.clear();
    };

    auto p_prev = p_it++;
    auto q_prev = q_it++;

#ifdef DEBUG_EXTRA_RING
    int index = 0;
#endif    
    for ( ; p_it != p_end; ++p_prev, ++q_prev, ++p_it, ++q_it)
    {
        int const side_q0_p = side_strategy::apply(p_prev->point, p_it->point, q_prev->point);
        int const side_q1_p = side_strategy::apply(p_prev->point, p_it->point, q_it->point);
        int const side_p0_q = side_strategy::apply(q_prev->point, q_it->point, p_prev->point);
        int const side_p1_q = side_strategy::apply(q_prev->point, q_it->point, p_it->point);

        int const code_p = side_p0_q == -1 && side_p1_q == -1 ? 1
            : side_p0_q == 1 && side_p1_q == 1 ? 2
            : 0;

        int const code_q = side_q0_p == -1 && side_q1_p == -1 ? 3
            : side_q0_p == 1 && side_q1_p == 1 ? 4
            : 0;

        // If both p and q are one-sided, prefer taking the previous code.
        // If there is not a previous code, prefer a non projected source.

        int const new_ring_code 
            = code_p > 0 && code_q > 0 ? (code_p == ring_code ? code_p : code_q == ring_code ? code_q : p_prev->is_projection && p_it->is_projection ? code_q : p_prev->is_projection || p_it->is_projection ? code_q : code_p)
            : code_p > 0 ? code_p
            : code_q > 0 ? code_q
            : 0;
        // int const new_ring_code = 0;

        bool const other_ring = new_ring_code != ring_code;
        if (other_ring)
        {
            finish_ring();
        }

        ring_code = new_ring_code;

#ifdef DEBUG_EXTRA_RING
        //if (new_ring_code > 0)
        if (p_prev->source_index != q_prev->source_index || p_it->source_index != q_it->source_index)
        {
            std::cout << index++ << " : " 
                << " sources: " << p_prev->source_index << " " << p_it->source_index << " " << q_prev->source_index << " " << q_it->source_index 
                << " proj: " << p_prev->is_projection << " " << p_it->is_projection << " " << q_prev->is_projection << " " << q_it->is_projection
                << " sides: " << " " << side_p0_q << " " << side_p1_q << " " <<  side_q0_p << " " << side_q1_p 
                << " code: " << code_p << " " << code_q << " -> " << new_ring_code
                << std::endl
                ;
        }
#endif       


        if (new_ring_code > 0)
        {
            if (other_ring)
            {
                // std::cout << " FIRST";
                left.push_back(p_prev->point);
                right.push_back(q_prev->point);
            }
            // std::cout << " NEXT";
            left.push_back(p_it->point);
            right.push_back(q_it->point);
#ifdef DEBUG_EXTRA_RING0
            std::cout << " RING" << std::endl;
#endif            
            continue;
        }

#ifdef DEBUG_EXTRA_RING0
        std::cout << std::endl;
#endif        

        const bool p_both_sides = side_p0_q * side_p1_q == -1;
        const bool q_both_sides = side_q0_p * side_q1_p == -1;

        if (p_both_sides && q_both_sides)
        {
            // Segments cross, make two triangles on either side of segment p
            add_triangle(0, *p_prev, *p_it, *q_prev);
            add_triangle(1, *p_prev, *p_it, *q_it);
        }
        else
        {
            add_triangle(0, *p_prev, *p_it, *q_prev);
            add_triangle(1, *q_prev, *q_it, *p_it);
        }

        // if (fill_quadrilateral(*p_prev, *p_it, *q_prev, *q_it, ring))
        // {
        //     auto const area = geometry::area(ring);

        //     // The area should be positive - taking the abs is a defensive check.
        //     sum_area += std::abs(area);

        //     visitor.visit_quadrilateral(ring, *p_prev, *p_it, *q_prev, *q_it);
        // }
    }
    finish_ring();

    return sum_area;
}

}} // namespace detail::similarity
#endif // DOXYGEN_NO_DETAIL


struct similarity_default_visitor
{
    template <typename T, typename I> void visit_triangle(T const& , I const& , I const& , I const& , int, int) {}
    template <typename C> void visit_projections(int, C const&) {}
};


template <typename Geometry1, typename Geometry2, typename Visitor>
auto similarity(Geometry1 const& p, Geometry2 const& q, Visitor& visitor = similarity_default_visitor())
{
    using namespace detail::similarity;

    using point_t = typename point_type<Geometry1>::type;
    using coor_t = typename coordinate_type<Geometry1>::type;

    similarity_info<coor_t> result;

    // Get projections of P on Q.
    // Later, the points of Q are added to these projections.
    // Therefore, this variable can be seen as an enriched version of range Q.
    auto q_enriched = get_projections(p, q);

    result.is_reversed = is_reversed(q_enriched);

    if (result.is_reversed)
    {
        // Reverse Q to align with P, and add own points using reversed iterators
        reverse_projections(q, q_enriched);
        enrich_with_self(boost::rbegin(q), boost::rend(q), q_enriched);
    }
    else
    {
        enrich_with_self(boost::begin(q), boost::end(q), q_enriched);
    }

    // Do the same for P. Because Q is possibly reversed to align with P, 
    // such a check is not necessary for P again.
    auto p_enriched = get_projections(q, p);
    enrich_with_self(boost::begin(p), boost::end(p), p_enriched);

    if (q_enriched.size() != p_enriched.size())
    {
        // Defensive check.
        // They should always have the same size: 
        // each other projected points, plus their own points.
        return result;
    }

    for (std::size_t i = 0; i < p_enriched.size(); i++)
    {
        p_enriched[i].sort_index = q_enriched[i].source_index;
        q_enriched[i].sort_index = p_enriched[i].source_index;
    }

    // Optional debug step.
    visitor.visit_projections(0, p_enriched);
    visitor.visit_projections(1, q_enriched);

    using ring_t = boost::geometry::model::ring<point_t>;

    ring_t triangle;
    triangle.resize(4);

    auto const sum_area = get_areal_sum(boost::begin(p_enriched), boost::end(p_enriched), 
        boost::begin(q_enriched), boost::end(q_enriched), triangle, visitor);

    auto const length = (std::min)(geometry::perimeter(p), geometry::perimeter(q));

    std::cout << "AREA: " << sum_area << std::endl;
    std::cout << "LENS p: " << geometry::length(p) << " " << geometry::perimeter(p) << std::endl;
    std::cout << "LENS q: " << geometry::length(q) << " " << geometry::perimeter(q) << std::endl;
    result.distance = length > 0 ? sum_area / length : std::sqrt(sum_area);        
    return result;
}

template <typename Geometry1, typename Geometry2>
auto average_distance(Geometry1 const& a, Geometry2 const& b)
{
    similarity_default_visitor v;
    return similarity(a, b, v).distance;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_SIMILARITY_HPP
