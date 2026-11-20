// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POINT_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <boost/geometry/algorithms/detail/buffer/certified/coordinates.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/strategies/cartesian/exact/round_buffer.hpp>
#include <boost/geometry/util/range.hpp>

namespace boost { namespace geometry
{

enum class certified_buffer_status { success, invalid_input, cannot_certify };
using certified_point_buffer_status = certified_buffer_status;

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_point_buffer
{

namespace rb = ec::round_buffer;
using rb::rational_xy;

constexpr unsigned max_subdivision_depth = 16;
constexpr std::size_t max_vertices = 4096;

template <typename Number>
inline exact_number rational_value(Number const& value)
{
    if constexpr (std::is_same<Number, exact_number>::value)
    {
        return value;
    }
    else
    {
        return ec::exact_value(value);
    }
}

template <typename Point>
inline rational_xy read_point(Point const& point)
{
    return {rational_value(geometry::get<0>(point)), rational_value(geometry::get<1>(point))};
}

// Emit the start of each accepted chord. Its end is the next start, even at
// quadrant seams and closure. The chart parameter can increase or decrease.
template <typename Emit>
inline bool subdivide(unsigned quadrant, exact_number const& ta, rb::circle_direction const& a,
                      exact_number const& tb, rb::circle_direction const& b,
                      exact_number const& radius, exact_number const& chord_error,
                      unsigned depth, Emit& emit)
{
    if (rb::minor_arc_radial_error_within(a, b, radius, chord_error))
    {
        return emit(a.numerator());
    }
    if (depth == max_subdivision_depth)
    {
        return false;
    }
    exact_number const tm = (ta + tb) / 2;
    rb::circle_direction const middle(rb::rational_arc_sample(quadrant, tm));
    return subdivide(quadrant, ta, a, tm, middle, radius, chord_error, depth + 1, emit)
        && subdivide(quadrant, tm, middle, tb, b, radius, chord_error, depth + 1, emit);
}

inline exact_number dot(rational_xy const& a, rational_xy const& b)
{
    return a.x * b.x + a.y * b.y;
}

inline exact_number cross(rational_xy const& a, rational_xy const& b)
{
    return a.x * b.y - a.y * b.x;
}

inline rational_xy difference(rational_xy const& a, rational_xy const& b)
{
    return {a.x - b.x, a.y - b.y};
}

inline bool second_half(rational_xy const& v)
{
    return v.y < 0 || (v.y == 0 && v.x < 0);
}

inline bool advance_ccw(rational_xy const& a, rational_xy const& b, unsigned& wraps)
{
    if (cross(a, b) <= 0)
    {
        return false;
    }
    // The positive cross product means the CCW sweep is in (0, pi). Its
    // unwrapped angle crosses the positive x axis iff it changes halves 1->0.
    return !second_half(a) || second_half(b) || ++wraps == 1;
}

template <typename Polygon, typename VertexCheck, typename EdgeCheck>
inline bool certify_ring(Polygon const& polygon, rational_xy const& center, std::size_t count,
                         VertexCheck const& check_vertex, EdgeCheck const& check_distance)
{
    auto const& holes = interior_rings(polygon);
    if (count < 3 || boost::begin(holes) != boost::end(holes))
    {
        return false;
    }
    unsigned wraps = 0;
    auto const check_edge = [&](rational_xy const& a, rational_xy const& b)
    {
        bool const advance = geometry::point_order<Polygon>::value == clockwise
            ? advance_ccw(b, a, wraps) : advance_ccw(a, b, wraps);
        return advance && check_distance(a, b);
    };

    auto const& ring = exterior_ring(polygon);
    auto it = boost::begin(ring);
    auto const end = boost::end(ring);
    if (it == end) { return false; }
    rational_xy const first = difference(read_point(*it++), center);
    if (!check_vertex(first)) { return false; }
    rational_xy previous = first;
    for (std::size_t i = 1; i < count; ++i)
    {
        if (it == end) { return false; }
        rational_xy current = difference(read_point(*it++), center);
        if (!check_vertex(current) || !check_edge(previous, current)) { return false; }
        previous = std::move(current);
    }
    if constexpr (closure<Polygon>::value == closed)
    {
        if (it == end) { return false; }
        auto const last = difference(read_point(*it++), center);
        if (last.x != first.x || last.y != first.y) { return false; }
    }
    // One full turn with positive angular gaps enforces distinct vertices,
    // a simple boundary and one intersection per ray from the center.
    return it == end && check_edge(previous, first) && wraps == 1;
}

template <typename Polygon>
inline bool certify_export(Polygon const& polygon, rational_xy const& center,
                           exact_number const& radius, exact_number const& error, std::size_t count)
{
    exact_number const inner = radius > error ? exact_number(radius - error) : exact_number(0);
    exact_number const outer = radius + error;
    exact_number const inner2 = inner * inner, outer2 = outer * outer;
    auto const in_annulus = [&](rational_xy const& p)
    {
        exact_number const d2 = dot(p, p);
        return inner2 < d2 && d2 < outer2;
    };
    auto const outside_inner = [&](rational_xy const& a, rational_xy const& b)
    {
        rational_xy const edge = difference(b, a);
        // Endpoints passed in_annulus. Test the interior projection, if any.
        exact_number const area = cross(a, b);
        return dot(a, edge) >= 0 || dot(b, edge) <= 0 || area * area > inner2 * dot(edge, edge);
    };
    return certify_ring(polygon, center, count, in_annulus, outside_inner);
}

}} // namespace detail::certified_point_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Experimental certified round buffer of one 2D Cartesian Point.
\details On success, output is a simple, hole-free polygon containing the
    center. Its boundary and the exact circle, and its filled region and the
    exact disk, have Euclidean Hausdorff distance strictly less than max_error.
    The certificate applies to the stored output coordinates.
\return success, invalid_input (nonfinite value, nonpositive radius or
    negative tolerance), or cannot_certify (including zero tolerance,
    subdivision limits, overflow or a failed export certificate).
\note A returned failure leaves output unchanged. This opt-in algorithm does
    not dispatch to other geometry types or alter geometry::buffer.
    Numeric types are integral, IEEE float/double with gradual underflow,
    and cpp_rational. The output polygon must be default constructible and
    assignable.
*/
template <typename Point, typename Polygon, typename Radius, typename Error>
[[nodiscard]] inline certified_point_buffer_status buffer_point_certified(
    Point const& input, Polygon& output, Radius const& radius, Error const& max_error)
{
    static_assert(std::is_same<tag_t<Point>, point_tag>::value
               && std::is_same<tag_t<Polygon>, polygon_tag>::value,
                  "Certified point buffer requires a Point and a Polygon");
    static_assert(dimension<Point>::value == 2 && dimension<Polygon>::value == 2
               && std::is_same<cs_tag_t<Point>, cartesian_tag>::value
               && std::is_same<cs_tag_t<Polygon>, cartesian_tag>::value,
                  "Certified point buffer requires 2D Cartesian geometries");
    concepts::check<Point const>();
    concepts::check<Polygon>();

    namespace impl = detail::certified_point_buffer;
    using status = certified_point_buffer_status;
    impl::rational_xy center;
    impl::exact_number r, error;
    try
    {
        center = impl::read_point(input);
        r = impl::rational_value(radius);
        error = impl::rational_value(max_error);
    }
    catch (std::domain_error const&)
    {
        return status::invalid_input;
    }
    if (r <= 0 || error < 0) { return status::invalid_input; }
    if (error == 0) { return status::cannot_certify; }

    Polygon candidate;
    auto& ring = exterior_ring(candidate);
    using point = point_type_t<Polygon>;
    using coordinate = coordinate_type_t<point>;
    using rounder = impl::coordinate_writer<coordinate>;

    std::size_t count = 0;
    auto emit = [&](impl::rational_xy const& unit)
    {
        if (count == impl::max_vertices) { return false; }
        point p;
        geometry::set<0>(p, rounder::apply(center.x + r * unit.x));
        geometry::set<1>(p, rounder::apply(center.y + r * unit.y));
        range::push_back(ring, std::move(p));
        ++count;
        return true;
    };

    try
    {
        bool constexpr cw = point_order<Polygon>::value == clockwise;
        impl::exact_number const ta = cw ? 1 : 0, tb = cw ? 0 : 1;
        impl::exact_number const chord_error = error / 2;
        for (unsigned i = 0; i < 4; ++i)
        {
            unsigned const quadrant = cw ? 3 - i : i;
            impl::rb::circle_direction const a(impl::rb::rational_arc_sample(quadrant, ta));
            impl::rb::circle_direction const b(impl::rb::rational_arc_sample(quadrant, tb));
            if (!impl::subdivide(quadrant, ta, a, tb, b, r, chord_error, 0, emit))
            {
                return status::cannot_certify;
            }
        }
        if constexpr (closure<Polygon>::value == closed)
        {
            auto const first = boost::begin(ring);
            if (first == boost::end(ring)) { return status::cannot_certify; }
            range::push_back(ring, point(*first));
        }
        if (!impl::certify_export(candidate, center, r, error, count))
        {
            return status::cannot_certify;
        }
    }
    catch (std::overflow_error const&)
    {
        return status::cannot_certify;
    }
    catch (std::domain_error const&)
    {
        // A floating export may convert to infinity, which exact_value rejects.
        return status::cannot_certify;
    }
    output = std::move(candidate);
    return status::success;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POINT_HPP
