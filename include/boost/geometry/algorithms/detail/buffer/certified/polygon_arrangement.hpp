// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_ARRANGEMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_ARRANGEMENT_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/linestring_arrangement.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_polygon_buffer
{

template <typename Polygon>
inline status read_arrangement_polygon(Polygon const& input, std::vector<ring>& rings, std::size_t& count)
{
    bool constexpr ccw = geometry::point_order<Polygon>::value == counterclockwise;
    bool constexpr closed = closure<Polygon>::value == geometry::closed;
    auto result = read_ring(exterior_ring(input), closed, ccw, rings, count);
    if (result != status::success) { return result; }
    for (auto const& h : interior_rings(input))
    {
        result = read_ring(h, closed, !ccw, rings, count);
        if (result != status::success) { return result; }
    }
    return valid_source(rings) ? status::success : status::invalid_input;
}

}} // namespace detail::certified_polygon_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Bounded native round dilation or regularized erosion of a Cartesian Polygon.
\details Positive distance dilates; negative distance erodes. Exact rational
    source rings and algebraic inscribed boundary capsules are noded before
    conversion. Input requires a simple shell and simple, disjoint holes.
    Output is a MultiPolygon; splits, holes and empty erosion are supported.
    Irrational edge lengths are allowed. Requires 0 < sagitta < abs(distance)
    and 0 <= max_displacement < abs(distance)-sagitta. Neither a common
    interior witness nor lossless vertices are required.
\details The ideal output is the source filled region union the chord band,
    or the regularized difference for erosion. Native export checks exact
    Euclidean boundary displacement and preserves the ideal ring containment
    forest. Dilation has filled-set Hausdorff error at most sagitta plus
    max_displacement. Erosion has radius inclusion bounds, not a general
    Hausdorff guarantee. Neither operation certifies smooth-buffer topology.
\return success, invalid_input or cannot_certify. Failure leaves output
    unchanged. Work limits, exact boundary contacts, unrepresentable native
    geometry or a failed topology or displacement test are refused.
*/
template <typename Polygon, typename MultiPolygon, typename Distance, typename Error, typename Displacement>
[[nodiscard]] inline certified_buffer_status buffer_polygon_arrangement_certified(
    Polygon const& input, MultiPolygon& output, Distance const& distance,
    Error const& sagitta, Displacement const& max_displacement)
{
    namespace impl = detail::certified_star_buffer;
    namespace source = detail::certified_polygon_buffer;
    using status = certified_buffer_status;
    static_assert(std::is_same<tag_t<Polygon>, polygon_tag>::value
               && std::is_same<tag_t<MultiPolygon>, multi_polygon_tag>::value,
                  "Certified polygon arrangement requires a Polygon and a MultiPolygon");
    static_assert(dimension<Polygon>::value == 2 && dimension<MultiPolygon>::value == 2
               && std::is_same<cs_tag_t<Polygon>, cartesian_tag>::value
               && std::is_same<cs_tag_t<MultiPolygon>, cartesian_tag>::value,
                  "Certified polygon arrangement requires 2D Cartesian geometries");
    concepts::check<Polygon const>();
    concepts::check<MultiPolygon>();

    std::vector<impl::rational_arr::ring> rings;
    impl::exact_number r, e, d;
    bool erosion;
    try
    {
        r = impl::common::rational_value(distance);
        e = impl::common::rational_value(sagitta);
        d = impl::common::rational_value(max_displacement);
        erosion = r < 0;
        if (erosion) { r = -r; }
        if (r == 0 || e < 0 || e >= r || d < 0 || d >= r - e) { return status::invalid_input; }
        if (e == 0) { return status::cannot_certify; }

        std::size_t count = 0;
        auto const result = source::read_arrangement_polygon(input, rings, count);
        if (result != status::success) { return result; }
    }
    catch (std::domain_error const&) { return status::invalid_input; }

    try
    {
        impl::algebraic_chord_union arrangement;
        auto const& anchor = rings.front().points.front();
        arrangement.append_polygon(rings, anchor, r, e);
        return arrangement.build_polygon(anchor, d, erosion, output) ? status::success : status::cannot_certify;
    }
    catch (impl::uncertified const&) { return status::cannot_certify; }
    catch (std::overflow_error const&) { return status::cannot_certify; }
    catch (std::domain_error const&) { return status::cannot_certify; }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_ARRANGEMENT_HPP
