// Boost.Geometry

// Copyright (c) 2023 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_ALGORITHMS_SIMILARITY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_SIMILARITY_HPP

#include <cstdint>
#include <vector>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/length.hpp>


namespace boost { namespace geometry
{

struct range_info
{
    std::size_t begin{};
    std::size_t end{};
};

struct partial_info
{
    double ratio{0.0};
    std::size_t index;
    double sim_distance{};
};

template <typename Point>
struct similarity_info
{
    using coor_t = typename coordinate_type<Point>::type;
    
    coor_t distance{};
    bool is_reversed{false};
    bool is_partial{false};

    std::size_t index; // TODO
}; 

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace similarity
{

// Structure containing projections of a geometry on another geometry,
// and additionally the points of the geometry itself
template <typename Point>
struct projection
{
    using point_type = Point;
    using coor_t = typename coordinate_type<Point>::type;

    // Indicates if it is a projection of the other geometry, or a point of the own geometry.
    bool is_projection{true};

    // For a projection of P on Q, this is the index in P
    // For a non projected point, it is also the index in P
    std::size_t source_index{};

    // For a projection of P on Q, this is the index of a segment in Q
    // For a non projected point, it is identical to source_index
    std::size_t segment_index{};

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
    using closest_strategy = geometry::strategy::closest_points::detail::compute_closest_point_to_segment<double>;
    projection<Point> result{};

    for (std::size_t i = 0; i + 1 < target.size(); i++)
    {
        auto const& current = target[i];
        auto const& next = target[i + 1];
        projection<Point> other;
        auto const pp = closest_strategy::apply(point, current, next);
        geometry::convert(pp, other.point);
        other.distance = geometry::distance(point, other.point);
        if (i == 0 || other.distance < result.distance)
        {
            result = other;
            result.is_projection = true;
            result.source_index = index;
            result.segment_index = i;
            result.offset = geometry::distance(current, result.point);
        }
    }
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
            // std::cerr << "OOPS" << std::endl;
            continue;
        }
        auto const segment_length = geometry::distance(target[si], target[si + 1]);
        // if (projection.offset > segment_length)
        // {
        //     std::cerr << "OOPS 2" << std::endl;
        // }
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
    double sum_area = 0.0;

    auto p_prev = p_it++;
    auto q_prev = q_it++;

    for ( ; p_it != p_end; ++p_it, ++q_it)
    {
        if (fill_quadrilateral(*p_prev, *p_it, *q_prev, *q_it, ring))
        {
            double const area = geometry::area(ring);

            // if (debug && area < 0.0)
            // {
            //     std::cout << (area < 0.0 ? "*WRONG" : " right") << " area for " << i 
            //         << " sides: " << side_p0_q << " " << side_p1_q << " " << side_q0_p << " " << side_q1_p
            //         << " " << " -> " << kScale * area << std::endl;
            // }

            // The area should be positive - taking the abs is a defensive check.
            sum_area += std::abs(area);

            visitor.visit_quadrilateral(ring, *p_prev, *p_it, *q_prev, *q_it);
        }

        p_prev = p_it;
        q_prev = q_it;
    }
    return sum_area;
}

// Finds the best division, the index where P/Q are more similar on one side and less on the other side.
//        -----------  Q, close to P at the start, but than farther away
//       /
// -----/
// ------------------  P
//      *              it tries to find index at point *
// Criterium:
// - we start with the total similaraty (length-averaged distance)
// - we go half-way. There the left-similarity is BETTER and the right-similarity is WORSE than the whole.
//   If that is the case, half-way is better. But might not yet be optimal.
// - we go half-way the left part (= 1/4). Calculate left and right similarities. Is that better?
//   To calculate that, we divide left/right and compare it with left/right of half-way
// - we go half-way the right part (= 3/4). Is that better?
template <typename It, typename Ring, typename Visitor>
void find_division(It p_begin, It p_end, It q_begin, It q_end, 
        range_info const& range, double total_length, double previous_ratio, partial_info& result, Ring& ring, Visitor& visitor, bool debug)
{
    auto const range_size = range.end - range.begin;
    if (range_size < 2 || range.begin >= range.end)
    {
        return;
    }
    auto const mid = range_size / 2;
    auto const p_mid = boost::next(p_begin, range.begin + mid);
    auto const q_mid = boost::next(q_begin, range.begin + mid);
    auto const total_size = std::distance(p_begin, p_end);
    // TODO: this should be done based on real length
    auto const mid_fraction = (range.begin + mid) / static_cast<double>(total_size);
    auto const left_length = total_length * mid_fraction;
    auto const right_length = total_length * (1.0 - mid_fraction);

    auto const left_sim = get_areal_sum(p_begin, p_mid, q_begin, q_mid, ring, visitor) / left_length;
    auto const right_sim = get_areal_sum(p_mid, p_end, q_mid, q_end, ring, visitor) / right_length;

    constexpr double eps = 1.0e-9;
    bool const left_too_far =  left_sim < eps && right_sim > eps;
    bool const right_too_far = left_sim > eps && right_sim < eps;
    bool const left_better = (! left_too_far) && left_sim < result.sim_distance && right_sim > result.sim_distance;
    bool const right_better = (! right_too_far) && ((right_sim < result.sim_distance && left_sim > result.sim_distance) || result.sim_distance < eps);

    auto const ratio = left_sim > right_sim ? left_sim / std::max(eps, right_sim) : right_sim / std::max(eps, left_sim);

    if (debug)
    {
        double kScale = 1.0e4;
        std::cout << std::setprecision(20) << "PART " << range.begin << ".." << range.end 
        << " at " << std::distance(p_begin, p_mid)
                << " previous " << kScale * result.sim_distance
                << " new " << kScale * left_sim << " " << kScale * right_sim
                << " ratio " << std::setprecision(10) << ratio
                << (left_better ? " LEFT " : "")
                << (right_better ? " RIGHT " : "")
                << (left_too_far ? " right " : "")
                << (right_too_far ? " left " : "")
                 << std::endl;
    }  

    if (ratio < previous_ratio) // || !(left_better || right_better || right_too_far || left_too_far))
    {
        // It is not better than found earlier.
        return;
    }
    if (ratio > result.ratio)
    {
        // This is the best found until now.
        TODO
        result.sim_distance = std::min(left_sim, right_sim);
        result.index = range.begin + mid;
        if (result.sim_distance > eps)
        {
            result.ratio = ratio;
        }
        if (debug)
        {
            std::cout << "REASSIGN -> " << result.ratio << " " << result.sim_distance << " at " << result.index << std::endl;
        }
    }

    // Try to make it better
    if (left_better)
    {
        find_division(p_begin, p_end, q_begin, q_end, {range.begin, range.begin + mid}, total_length, ratio, result, ring, visitor, debug);
    }
    else if (right_better)
    {
        find_division(p_begin, p_end, q_begin, q_end, {range.begin + mid, range.end}, total_length, ratio, result, ring, visitor, debug);
    }
    else if (right_too_far)
    {
        find_division(p_begin, p_end, q_begin, q_end, {range.begin, range.begin + mid}, total_length, 0.0, result, ring, visitor, debug);
    }
    else if (left_too_far)
    {
        find_division(p_begin, p_end, q_begin, q_end, {range.begin + mid, range.end}, total_length, 0.0, result, ring, visitor, debug);
    }
}

}} // namespace detail::similarity
#endif // DOXYGEN_NO_DETAIL


struct similarity_default_visitor
{
    template <typename T, typename I> void visit_quadrilateral(T const& , I const& , I const& , I const& , I const& ) {}
    template <typename C> void visit_projections(int, C const&) {}
};


template <typename Geometry1, typename Geometry2, typename Visitor>
auto similarity(Geometry1 const& p, Geometry2 const& q, bool consider_partial = false, Visitor& visitor = similarity_default_visitor(), bool debug = false)
{
    using namespace detail::similarity;

    using point_t = typename point_type<Geometry1>::type;

    similarity_info<point_t> result;

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

    auto p_begin = boost::begin(p_enriched);
    auto p_end = boost::end(p_enriched);
    auto q_begin = boost::begin(q_enriched);
    auto q_end = boost::end(q_enriched);
    double sum_area = get_areal_sum(p_begin, p_end, q_begin, q_end, ring, visitor);

    const double length = (std::min)(geometry::length(p), geometry::length(q));
    const double average_distance = length > 0 ? sum_area / length : std::sqrt(sum_area);        

    result.distance = average_distance;

    if (consider_partial && sum_area > 0 && length > 0.0)
    {
        partial_info part;
        part.ratio = 0.0;
        part.index = 0;
        part.sim_distance = average_distance;
        find_division(p_begin, p_end, q_begin, q_end, {0, q_enriched.size()}, length, 0.0, part, ring, visitor, debug);

        if (part.index != 0)
        {
            result.distance = part.sim_distance;
            result.index = part.index;
            result.is_partial = true;
        }
    }

    return result;
}

template <typename Geometry1, typename Geometry2>
double average_distance(Geometry1 const& a, Geometry2 const& b, bool consider_partial = false)
{
    similarity_default_visitor v;
    return similarity(a, b, consider_partial, v, false).distance;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_SIMILARITY_HPP


// PART 0..29 at 14 previous 25.898039226400840107 new 51.688648950232028767 0 ratio 51688648.95 left 
// PART 0..14 at 7 previous 51.688648950232028767 new 73.888834317631378212 10.028356170353541543 ratio 7.367990632 RIGHT 
// PART 7..14 at 10 previous 10.028356170353541543 new 59.862741985774739817 7.2063052955694093527 ratio 8.30699499 RIGHT 
// PART 10..14 at 12 previous 7.2063052955694093527 new 51.20659575145721476 5.4146934179804215859 ratio 9.456970469 RIGHT 
// PART 12..14 at 13 previous 5.4146934179804215859 new 50.691796112304395194 1.7126282663985041843 ratio 29.59883187 RIGHT 