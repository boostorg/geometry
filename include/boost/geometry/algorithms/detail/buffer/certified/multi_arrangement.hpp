// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_MULTI_ARRANGEMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_MULTI_ARRANGEMENT_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/polygon_arrangement.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_multi_buffer
{

namespace algebraic = certified_star_buffer;
namespace source = certified_polygon_buffer;
namespace rational = rational_chord_arrangement;
using algebraic::common::rational_value;
using algebraic::exact_number;
using algebraic::rational_xy;
using status = certified_buffer_status;

struct invalid_coordinate {};

template <typename Point>
inline rational_xy read_point(Point const& p)
{
    try { return algebraic::common::read_point(p); }
    catch (std::domain_error const&) { throw invalid_coordinate{}; }
}

// All members append ideal producers to this one arrangement. The anchor
// is a coordinate translation, not a witness of any containment property.
class linear_members
{
public:
    linear_members(algebraic::algebraic_chord_union& arrangement,
                   exact_number const& radius, exact_number const& error)
        : m_arrangement(arrangement), m_radius(radius), m_error(error) {}

    template <typename Point>
    void append_point(Point const& p)
    {
        m_arrangement.append_disk(relative(p), m_radius, m_error);
    }

    template <typename Linestring>
    void append_trace(Linestring const& trace)
    {
        auto it = boost::begin(trace);
        auto const end = boost::end(trace);
        if (it == end) { return; }
        auto previous = relative(*it++);
        bool nonzero = false;
        for (; it != end; ++it)
        {
            auto const p = relative(*it);
            if (algebraic::capsule::same_point(p, previous)) { continue; }
            m_arrangement.append(algebraic::capsule::rational_segment(previous, p), m_radius, m_error);
            previous = p;
            nonzero = true;
        }
        if (!nonzero) { m_arrangement.append_disk(previous, m_radius, m_error); }
    }

    rational_xy const& anchor() const { return m_anchor; }

private:
    template <typename Point>
    rational_xy relative(Point const& p)
    {
        auto const exact = read_point(p);
        if (!m_have_anchor) { m_anchor = exact; m_have_anchor = true; }
        return algebraic::difference(exact, m_anchor);
    }

    algebraic::algebraic_chord_union& m_arrangement;
    exact_number const& m_radius;
    exact_number const& m_error;
    rational_xy m_anchor{0, 0};
    bool m_have_anchor = false;
};

using polygon_rings = std::vector<rational::ring>;

inline bool in_material(polygon_rings const& rings, rational_xy const& p)
{
    if (rational::locate(rings.front(), p) != 1) { return false; }
    for (std::size_t i = 1; i < rings.size(); ++i)
    {
        if (rational::locate(rings[i], p) == 1) { return false; }
    }
    return true;
}

inline bool disjoint_members(polygon_rings const& a, polygon_rings const& b)
{
    if (!a.front().box.overlaps(b.front().box)) { return true; }
    for (auto const& u : a)
    {
        for (auto const& v : b)
        {
            if (!u.box.overlaps(v.box)) { continue; }
            for (auto const& x : u.edges)
            {
                for (auto const& y : v.edges)
                {
                    rational::intersection hit;
                    if (rational::intersect(x, y, hit)) { return false; }
                }
            }
        }
    }
    // With disjoint boundaries, overlap requires one shell to sit in
    // the other's material. A shell in a hole is a valid disjoint island.
    return !in_material(a, b.front().points.front()) && !in_material(b, a.front().points.front());
}

template <typename MultiPolygon>
status read_members(MultiPolygon const& input, std::vector<polygon_rings>& members)
{
    std::size_t count = 0;
    try
    {
        for (auto const& p : input)
        {
            auto const& shell = exterior_ring(p);
            auto const& holes = interior_rings(p);
            if (boost::begin(shell) == boost::end(shell) && boost::begin(holes) == boost::end(holes)) { continue; }
            polygon_rings current;
            auto const result = source::read_arrangement_polygon(p, current, count);
            if (result != status::success) { return result; }
            for (auto const& other : members)
            {
                if (!disjoint_members(other, current)) { return status::invalid_input; }
            }
            members.push_back(std::move(current));
        }
    }
    catch (std::domain_error const&) { return status::invalid_input; }
    return status::success;
}

template <typename MultiPolygon>
status build_areal_members(std::vector<polygon_rings> const& members, MultiPolygon& output,
                           exact_number const& r, exact_number const& e, exact_number const& d, bool erosion)
{
    algebraic::algebraic_chord_union arrangement;
    rational_xy const anchor = members.empty() ? rational_xy{0, 0} : members.front().front().points.front();
    for (auto const& p : members) { arrangement.append_polygon(p, anchor, r, e); }
    return arrangement.build_polygon(anchor, d, erosion, output) ? status::success : status::cannot_certify;
}

}} // namespace detail::certified_multi_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Bounded native round buffer of a 2D Cartesian MultiPoint, MultiLineString, or MultiPolygon.
\details For points and lines the distance must be positive. MultiPolygon
    admits dilation and regularized erosion of valid members with disjoint
    filled sets and non-touching boundaries. Empty members are ignored;
    singleton and all-equal linestrings are point traces. A wholly empty
    collection gives empty output on valid budgets. All producers participate
    in one exact arrangement before native conversion.
\details For r=abs(distance), requires 0 < sagitta < r and
    0 <= max_displacement < r-sagitta. The existing arrangement limits
    apply to the entire input. The native MultiPolygon preserves the ideal
    chord result's disjoint rings and containment, with separately checked
    Euclidean displacement. Dilation has filled-set Hausdorff error at most
    sagitta plus max_displacement. Erosion has radius inclusion bounds only.
    No claim is made about topology of the smooth buffer.
\return success, invalid_input or cannot_certify. Failure leaves output unchanged.
*/
template <typename Geometry, typename MultiPolygon, typename Distance, typename Error, typename Displacement>
[[nodiscard]] inline certified_buffer_status buffer_multi_arrangement_certified(
    Geometry const& input, MultiPolygon& output, Distance const& distance,
    Error const& sagitta, Displacement const& max_displacement)
{
    namespace impl = detail::certified_multi_buffer;
    using status = certified_buffer_status;
    using tag = tag_t<Geometry>;
    bool constexpr areal = std::is_same<tag, multi_polygon_tag>::value;
    static_assert((areal || std::is_same<tag, multi_point_tag>::value || std::is_same<tag, multi_linestring_tag>::value)
               && std::is_same<tag_t<MultiPolygon>, multi_polygon_tag>::value,
                  "Certified multi buffer requires MultiPoint, MultiLineString or MultiPolygon into MultiPolygon");
    static_assert(dimension<Geometry>::value == 2 && dimension<MultiPolygon>::value == 2
               && std::is_same<cs_tag_t<Geometry>, cartesian_tag>::value
               && std::is_same<cs_tag_t<MultiPolygon>, cartesian_tag>::value,
                  "Certified multi buffer requires 2D Cartesian geometries");
    concepts::check<Geometry const>();
    concepts::check<MultiPolygon>();

    impl::exact_number r, e, d;
    bool erosion;
    try
    {
        r = impl::rational_value(distance);
        e = impl::rational_value(sagitta);
        d = impl::rational_value(max_displacement);
        erosion = r < 0;
        if (!areal && erosion) { return status::invalid_input; }
        if (erosion) { r = -r; }
        if (r == 0 || e < 0 || e >= r || d < 0 || d >= r - e) { return status::invalid_input; }
        if (e == 0) { return status::cannot_certify; }
    }
    catch (std::domain_error const&) { return status::invalid_input; }

    try
    {
        if constexpr (areal)
        {
            std::vector<impl::polygon_rings> members;
            auto const result = impl::read_members(input, members);
            if (result != status::success) { return result; }
            return impl::build_areal_members(members, output, r, e, d, erosion);
        }
        else
        {
            impl::algebraic::algebraic_chord_union arrangement;
            impl::linear_members members(arrangement, r, e);
            for (auto const& member : input)
            {
                if constexpr (std::is_same<tag, multi_point_tag>::value) { members.append_point(member); }
                else { members.append_trace(member); }
            }
            return arrangement.build_band(members.anchor(), d, output) ? status::success : status::cannot_certify;
        }
    }
    catch (impl::invalid_coordinate const&) { return status::invalid_input; }
    catch (impl::algebraic::uncertified const&) { return status::cannot_certify; }
    catch (std::overflow_error const&) { return status::cannot_certify; }
    catch (std::domain_error const&) { return status::cannot_certify; }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_MULTI_ARRANGEMENT_HPP
