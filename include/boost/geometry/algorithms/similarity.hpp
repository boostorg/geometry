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

template <typename Projection, typename Ring>
bool fill_quadrilateral(Projection const& p0, Projection const& p1, Projection const& q0, Projection const& q1, Ring& ring)
{
    using point_t = typename point_type<Ring>::type;
    using side_strategy = typename strategy::side::services::default_strategy
    <
        typename cs_tag<point_t>::type
    >::type;

    int const side_q0_p = side_strategy::apply(p0.point, p1.point, q0.point);
    int const side_q1_p = side_strategy::apply(p0.point, p1.point, q1.point);
    int const side_p0_q = side_strategy::apply(q0.point, q1.point, p0.point);
    int const side_p1_q = side_strategy::apply(q0.point, q1.point, p1.point);

    if (side_q0_p == 0 && side_q1_p == 0 && side_p0_q == 0 && side_p1_q == 0)
    // This might be enough:
    // if (side_q0_p == 0 && side_q1_p == 0)
    {
        // All sides are the same. This happens a lot when the inputs are the same.
        // Constructing the quadrilateral and calculating its area can be skipped.
        return false;
    }

    // Construct a clockwise ring (a quadrilateral).
    // For example
    // [2] +-----------+ [3]  [i + 1] (1)
    //     |           |
    //     p           q
    //     |           |
    // [1] +-----------+ [0]  [i] (0)
    // Here q(0) is right (-1) from p, so taken q(0) first. 
    // (Stated otherwise, p(0) is left (1) from q)
    // Then take p(0) as [1], second point.
    // As third point [2] then p(1) is taken because it is left (1) from q.
    // Finally q(1) used as [3] and the ring is closed [4].
    bool const first_q0 = side_q0_p == -1 || side_p0_q == 1;
    bool const third_p1 = side_p1_q == 1 || side_q1_p == -1;

    ring[0] = (first_q0 ? q0 : p0).point;
    ring[1] = (first_q0 ? p0 : q0).point;
    ring[2] = (third_p1 ? p1 : q1).point;
    ring[3] = (third_p1 ? q1 : p1).point;
    ring[4] = ring[0];

    return true;
}

template <typename It, typename Ring, typename Visitor>
auto get_areal_sum(It p_it, It p_end, It q_it, It q_end, Ring& ring, Visitor& visitor)
{
    typename coordinate_type<Ring>::type sum_area = 0;

    auto p_prev = p_it++;
    auto q_prev = q_it++;

    for ( ; p_it != p_end; ++p_it, ++q_it)
    {
        if (fill_quadrilateral(*p_prev, *p_it, *q_prev, *q_it, ring))
        {
            auto const area = geometry::area(ring);

            // The area should be positive - taking the abs is a defensive check.
            sum_area += std::abs(area);

            visitor.visit_quadrilateral(ring, *p_prev, *p_it, *q_prev, *q_it);
        }

        p_prev = p_it;
        q_prev = q_it;
    }
    return sum_area;
}

}} // namespace detail::similarity
#endif // DOXYGEN_NO_DETAIL


struct similarity_default_visitor
{
    template <typename T, typename I> void visit_quadrilateral(T const& , I const& , I const& , I const& , I const& ) {}
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

    // Optional debug step.
    visitor.visit_projections(0, p_enriched);
    visitor.visit_projections(1, q_enriched);

    if (q_enriched.size() != p_enriched.size())
    {
        // Defensive check.
        // They should always have the same size: 
        // each other projected points, plus their own points.
        return result;
    }

    using ring_t = boost::geometry::model::ring<point_t>;

    ring_t ring;
    ring.resize(5);

    auto const sum_area = get_areal_sum(boost::begin(p_enriched), boost::end(p_enriched), 
        boost::begin(q_enriched), boost::end(q_enriched), ring, visitor);

    auto const length = (std::min)(geometry::length(p), geometry::length(q));
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
