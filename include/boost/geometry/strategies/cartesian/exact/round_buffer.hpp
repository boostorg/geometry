// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROUND_BUFFER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROUND_BUFFER_HPP

#include <stdexcept>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian { namespace round_buffer
{

struct rational_xy
{
    exact_number x, y;
};

namespace kernel_detail
{

inline exact_number dot(rational_xy const& a, rational_xy const& b)
{
    return a.x * b.x + a.y * b.y;
}

inline int sign(exact_number const& a)
{
    return a > 0 ? 1 : a < 0 ? -1 : 0;
}

// Compare a with b * sqrt(d), for d > 0. Squaring preserves the ordering
// only for same-sign positives; reverse it for same-sign negatives.
inline int compare_radical(exact_number const& a, exact_number const& b,
                           exact_number const& d)
{
    int const sa = sign(a), sb = sign(b);
    if (sa != sb)
    {
        return sa < sb ? -1 : 1;
    }
    return sa == 0 ? 0 : sa * sign(a * a - b * b * d);
}

inline void check_radius(exact_number const& radius)
{
    if (radius < 0)
    {
        throw std::domain_error("Round buffer kernel requires a nonnegative radius");
    }
}

} // namespace kernel_detail

// The exact unit vector numerator() / sqrt(squared_length()). The radical
// stays symbolic, even when the direction happens to be rational.
class circle_direction
{
public:
    explicit circle_direction(rational_xy const& vector)
        : m_numerator(vector)
        , m_squared_length(kernel_detail::dot(vector, vector))
    {
        if (m_squared_length == 0)
        {
            throw std::domain_error("Round buffer direction must be nonzero");
        }
    }

    rational_xy const& numerator() const { return m_numerator; }
    exact_number const& squared_length() const { return m_squared_length; }

private:
    rational_xy m_numerator;
    exact_number m_squared_length;
};

// Exact endpoint: base + radius * normal. This is not a rational BG point.
struct offset_endpoint
{
    rational_xy base;
    exact_number radius;
    circle_direction normal;

    // Sign of the endpoint coordinate minus the rational query.
    int compare_x(exact_number const& x) const
    {
        return kernel_detail::compare_radical(radius * normal.numerator().x,
                                              x - base.x, normal.squared_length());
    }

    int compare_y(exact_number const& y) const
    {
        return kernel_detail::compare_radical(radius * normal.numerator().y,
                                              y - base.y, normal.squared_length());
    }
};

enum class offset_side { left, right };

class straight_offset
{
public:
    straight_offset(rational_xy const& from, rational_xy const& to,
                    exact_number const& radius, offset_side side)
        : m_from(from), m_to(to), m_radius(radius)
        , m_normal(side == offset_side::left ? rational_xy{from.y - to.y, to.x - from.x}
                                             : rational_xy{to.y - from.y, from.x - to.x})
    {
        kernel_detail::check_radius(radius);
    }

    circle_direction const& normal() const { return m_normal; }
    offset_endpoint start() const { return {m_from, m_radius, m_normal}; }
    offset_endpoint end() const { return {m_to, m_radius, m_normal}; }

    // Side of the infinite support, positive further along the chosen normal.
    // For raw normal n, the line equation is n.(p - from) = r * sqrt(n.n).
    int side_of_line(rational_xy const& point) const
    {
        rational_xy const delta{point.x - m_from.x, point.y - m_from.y};
        return kernel_detail::compare_radical(kernel_detail::dot(m_normal.numerator(), delta),
                                              m_radius, m_normal.squared_length());
    }

private:
    rational_xy m_from, m_to;
    exact_number m_radius;
    circle_direction m_normal;
};

// Unit circle sample in quadrant 0..3, counterclockwise from its first axis
// (t=0) to its next axis (t=1). Adjacent charts share the same exact endpoint.
// Scale by a rational radius and translate by the center for an arc sample.
inline rational_xy rational_arc_sample(unsigned quadrant, exact_number const& t)
{
    if (quadrant > 3 || t < 0 || t > 1)
    {
        throw std::domain_error("Round buffer sample requires quadrant 0..3 and t in [0,1]");
    }
    exact_number const tt = t * t;
    exact_number const x = (1 - tt) / (1 + tt);
    exact_number const y = 2 * t / (1 + tt);
    switch (quadrant)
    {
    case 0: return { x,  y};
    case 1: return {-y,  x};
    case 2: return {-x, -y};
    default: return { y, -x};
    }
}

// Radius and directions describe two points on a common circle. Certifies the
// maximum radial deficit of their chord for the MINOR arc (either semicircle
// for antipodes); does not select or validate the sweep of a join or cap.
inline bool minor_arc_radial_error_within(circle_direction const& u, circle_direction const& v,
                                         exact_number const& radius, exact_number const& max_error)
{
    kernel_detail::check_radius(radius);
    if (max_error < 0)
    {
        throw std::domain_error("Round buffer chord tolerance must be nonnegative");
    }
    if (max_error >= radius)
    {
        return true;
    }

    // The nearest chord point to the center is its midpoint. Test
    // r^2 * (1 + u.v) / 2 >= (r - max_error)^2; both distances are nonnegative.
    exact_number const r2 = radius * radius;
    exact_number const inner = radius - max_error;
    exact_number const threshold = 2 * inner * inner - r2;
    return kernel_detail::compare_radical(r2 * kernel_detail::dot(u.numerator(), v.numerator()),
                                         threshold, u.squared_length() * v.squared_length()) >= 0;
}

}} // namespace exact_cartesian::round_buffer
}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROUND_BUFFER_HPP
