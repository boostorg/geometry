// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_SEGMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_SEGMENT_HPP

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/algorithms/detail/buffer/certified/point.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_segment_buffer
{

namespace common = certified_point_buffer;
namespace ec = common::ec;
namespace rb = common::rb;
using common::certify_ring;
using common::cross;
using common::difference;
using common::dot;
using common::exact_number;
using common::rational_value;
using common::rational_xy;
using common::read_point;
using status = certified_buffer_status;

inline bool same_point(rational_xy const& a, rational_xy const& b)
{
    return a.x == b.x && a.y == b.y;
}

struct rational_segment
{
    rational_segment(rational_xy const& a, rational_xy const& b)
        : from(a), to(b), direction(difference(b, a)), norm2(dot(direction, direction))
    {}

    // Valid for a degenerate segment too: projection <= 0 returns first.
    exact_number squared_distance(rational_xy const& p) const
    {
        rational_xy const v = difference(p, from);
        exact_number const projection = dot(direction, v);
        if (projection <= 0) { return dot(v, v); }
        if (projection >= norm2)
        {
            auto const tail = difference(p, to);
            return dot(tail, tail);
        }
        exact_number const area = cross(direction, v);
        return area * area / norm2;
    }

    exact_number side(rational_xy const& p) const
    {
        return cross(direction, difference(p, from));
    }

    rational_xy from, to, direction;
    exact_number norm2;
};

inline bool opposite_signs(exact_number const& a, exact_number const& b)
{
    return (a > 0 && b < 0) || (a < 0 && b > 0);
}

template <typename Polygon>
inline bool certify_export(Polygon const& candidate, rational_xy const& a, rational_xy const& b,
                           exact_number const& radius, exact_number const& error, std::size_t count)
{
    rational_xy const middle{(a.x + b.x) / 2, (a.y + b.y) / 2};
    rational_segment const source(difference(a, middle), difference(b, middle));
    exact_number const inner = radius > error ? exact_number(radius - error) : exact_number(0);
    exact_number const outer = radius + error;
    exact_number const inner2 = inner * inner, outer2 = outer * outer;
    auto const vertex = [&](rational_xy const& p)
    {
        exact_number const d2 = source.squared_distance(p);
        return inner2 < d2 && d2 < outer2;
    };
    auto const edge = [&](rational_xy const& p, rational_xy const& q)
    {
        rational_segment const chord(p, q);
        // Distance between disjoint planar segments attains a minimum with
        // at least one endpoint. The output endpoints already passed vertex().
        // Touches and collinear overlaps have a zero endpoint distance, so
        // only a proper interior crossing needs a separate test.
        return chord.squared_distance(source.from) > inner2
            && chord.squared_distance(source.to) > inner2
            && !(opposite_signs(source.side(p), source.side(q))
                 && opposite_signs(chord.side(source.from), chord.side(source.to)));
    };
    return certify_ring(candidate, middle, count, vertex, edge);
}

// Every ideal vertex has coordinates base + numerator/sqrt(D) with the same
// positive D. For D=N/M, compute floor(2^bits*sqrt(N*M)) with integer sqrt.
// Its remainder distinguishes an exact rational scale from a strict bracket.
class inverse_length_enclosure
{
    using integer = boost::multiprecision::cpp_int;

public:
    explicit inverse_length_enclosure(exact_number const& squared_length)
        : m_denominator(boost::multiprecision::denominator(squared_length))
        , m_product(boost::multiprecision::numerator(squared_length) * m_denominator)
    {
        enclose(96);
    }

    bool is_exact() const { return m_lower == m_upper; }

    template <typename Coordinate>
    bool round(exact_number const& base, exact_number const& numerator, Coordinate& out)
    {
        using rounder = ec::point_detail::output_rounder<Coordinate>;
        if constexpr (std::is_same<Coordinate, exact_number>::value)
        {
            // Rationals have no nearest grid point for irrational coordinates.
            // This candidate is tested against the true capsule at export.
            out = base + numerator * (m_lower + m_upper) / 2;
            return true;
        }
        else
        {
            for (;;)
            {
                Coordinate const lo = rounder::apply(base + numerator * m_lower);
                Coordinate const hi = rounder::apply(base + numerator * m_upper);
                if (lo == hi)
                {
                    if constexpr (std::is_floating_point<Coordinate>::value)
                    {
                        if (!std::isfinite(lo)) { return false; }
                    }
                    // The nearest/ties-even map is monotone. Matching rounded
                    // bounds certify the rounded value of the ideal vertex.
                    out = lo;
                    return true;
                }
                if (m_bits >= 4096) { return false; }
                enclose(m_bits < 1536 ? 4 * m_bits : 4096);
            }
        }
    }

private:
    void enclose(unsigned bits)
    {
        integer remainder;
        integer const low_root = boost::multiprecision::sqrt(m_product << (2 * bits), remainder);
        exact_number const scaled_denominator(m_denominator << bits);
        m_upper = scaled_denominator / low_root;
        m_lower = remainder == 0 ? m_upper : exact_number(scaled_denominator / (low_root + 1));
        m_bits = bits;
    }

    integer m_denominator, m_product;
    exact_number m_lower, m_upper;
    unsigned m_bits = 0;
};

template <typename Polygon>
inline status build(rational_xy const& a, rational_xy const& b, Polygon& output,
                    exact_number const& radius, exact_number const& error,
                    bool require_rational_scale = false)
{
    if (radius <= 0 || error < 0) { return status::invalid_input; }
    if (error == 0) { return status::cannot_certify; }
    if (same_point(a, b))
    {
        using exact_point = model::point<exact_number, 2, cs::cartesian>;
        return geometry::buffer_point_certified(exact_point(a.x, a.y), output, radius, error);
    }

    rb::straight_offset const right(a, b, radius, rb::offset_side::right);
    rational_xy const tangent = difference(b, a);
    auto const& normal = right.normal().numerator();
    inverse_length_enclosure scale(right.normal().squared_length());
    if (require_rational_scale && !scale.is_exact()) { return status::cannot_certify; }

    Polygon candidate;
    auto& ring = exterior_ring(candidate);
    using point = point_type_t<Polygon>;
    using coordinate = coordinate_type_t<point>;
    std::size_t count = 0;
    auto const emit_point = [&](rational_xy const& center, rational_xy const& unit)
    {
        if (count == common::max_vertices) { return false; }
        coordinate x, y;
        if (!scale.round(center.x, radius * (unit.x * normal.x + unit.y * tangent.x), x)
         || !scale.round(center.y, radius * (unit.x * normal.y + unit.y * tangent.y), y))
        {
            return false;
        }
        point p;
        geometry::set<0>(p, x);
        geometry::set<1>(p, y);
        range::push_back(ring, std::move(p));
        ++count;
        return true;
    };

    try
    {
        bool constexpr cw = geometry::point_order<Polygon>::value == clockwise;
        exact_number const ta = cw ? 1 : 0, tb = cw ? 0 : 1;
        exact_number const chord_error = error / 2;
        for (unsigned i = 0; i < 4; ++i)
        {
            unsigned const quadrant = cw ? 3 - i : i;
            // In the (right normal, tangent) basis, quadrants 0 and 1
            // form the cap at b and 2 and 3 form the cap at a. At each
            // cap end, emit its terminal offset vertex too. The next
            // vertex is its exact translate along a true offset side.
            auto const& center = quadrant < 2 ? b : a;
            auto emit = [&](rational_xy const& unit) { return emit_point(center, unit); };
            rb::circle_direction const u(rb::rational_arc_sample(quadrant, ta));
            rb::circle_direction const v(rb::rational_arc_sample(quadrant, tb));
            if (!common::subdivide(quadrant, ta, u, tb, v, radius, chord_error, 0, emit)
             || (i % 2 == 1 && !emit(v.numerator())))
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
        if (!certify_export(candidate, a, b, radius, error, count))
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
        return status::cannot_certify;
    }
    output = std::move(candidate);
    return status::success;
}

template <typename Tag, typename Input, typename Polygon>
inline void check_types()
{
    static_assert(std::is_same<tag_t<Input>, Tag>::value
               && std::is_same<tag_t<Polygon>, polygon_tag>::value,
                  "Certified capsule buffer requires the specified input tag and a Polygon");
    static_assert(dimension<Input>::value == 2 && dimension<Polygon>::value == 2
               && std::is_same<cs_tag_t<Input>, cartesian_tag>::value
               && std::is_same<cs_tag_t<Polygon>, cartesian_tag>::value,
                  "Certified capsule buffer requires 2D Cartesian geometries");
    concepts::check<Input const>();
    concepts::check<Polygon>();
}

}} // namespace detail::certified_segment_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Experimental certified round buffer of one 2D Cartesian Segment.
\details On success, the stored polygon is simple and hole-free. Both the
    boundary and filled region have Euclidean Hausdorff distance strictly
    less than max_error from the corresponding parts of the exact capsule.
    Equal endpoints reduce to a Point buffer. Numeric types and output
    requirements are the same as for buffer_point_certified.
\return success, invalid_input, or cannot_certify (including subdivision,
    rounding or final-ring certificate failure). Failure leaves output unchanged.
*/
template <typename Segment, typename Polygon, typename Radius, typename Error>
[[nodiscard]] inline certified_buffer_status buffer_segment_certified(
    Segment const& input, Polygon& output, Radius const& radius, Error const& max_error)
{
    namespace impl = detail::certified_segment_buffer;
    impl::check_types<segment_tag, Segment, Polygon>();
    impl::rational_xy a, b;
    impl::exact_number r, error;
    try
    {
        a = {impl::rational_value(geometry::get<0, 0>(input)), impl::rational_value(geometry::get<0, 1>(input))};
        b = {impl::rational_value(geometry::get<1, 0>(input)), impl::rational_value(geometry::get<1, 1>(input))};
        r = impl::rational_value(radius);
        error = impl::rational_value(max_error);
    }
    catch (std::domain_error const&)
    {
        return certified_buffer_status::invalid_input;
    }
    return impl::build(a, b, output, r, error);
}

/*!
\brief Certified buffer of a nonempty, exactly collinear 2D Cartesian Linestring.
\details Its trace is the segment between the two extreme vertices, even
    with repeated vertices and backtracking. A singleton trace is a Point.
    Noncollinear traces return cannot_certify; empty inputs are invalid.
    On failure output is unchanged. No noding or union is performed.
*/
template <typename Linestring, typename Polygon, typename Radius, typename Error>
[[nodiscard]] inline certified_buffer_status buffer_collinear_linestring_certified(
    Linestring const& input, Polygon& output, Radius const& radius, Error const& max_error)
{
    namespace impl = detail::certified_segment_buffer;
    using status = certified_buffer_status;
    impl::check_types<linestring_tag, Linestring, Polygon>();
    auto it = boost::begin(input);
    auto const end = boost::end(input);
    if (it == end) { return status::invalid_input; }

    impl::rational_xy low, high;
    impl::exact_number r, error;
    bool collinear = true;
    try
    {
        r = impl::rational_value(radius);
        error = impl::rational_value(max_error);
        if (r <= 0 || error < 0) { return status::invalid_input; }
        auto const origin = impl::read_point(*it++);
        low = high = origin;
        impl::rational_xy axis{0, 0};
        impl::exact_number lo = 0, hi = 0;
        for (; it != end; ++it)
        {
            auto const p = impl::read_point(*it);
            auto const offset = impl::difference(p, origin);
            if (impl::same_point(low, high)) { axis = offset; }
            if (impl::cross(axis, offset) != 0) { collinear = false; }
            impl::exact_number const projection = impl::dot(axis, offset);
            if (projection < lo) { lo = projection; low = p; }
            if (projection > hi) { hi = projection; high = p; }
        }
    }
    catch (std::domain_error const&)
    {
        return status::invalid_input;
    }
    return collinear ? impl::build(low, high, output, r, error) : status::cannot_certify;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_SEGMENT_HPP
