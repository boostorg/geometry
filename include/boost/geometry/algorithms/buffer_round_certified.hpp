// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_BUFFER_ROUND_CERTIFIED_HPP
#define BOOST_GEOMETRY_ALGORITHMS_BUFFER_ROUND_CERTIFIED_HPP

#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/algorithms/detail/buffer/certified/dispatch.hpp>
#include <boost/geometry/algorithms/is_empty.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

namespace boost { namespace geometry
{

/*!
\brief Opt-in certified round buffer for a 2D Cartesian Point, Segment,
    Linestring, Polygon, MultiPoint, MultiLineString or MultiPolygon.
    Include \c boost/geometry/algorithms/buffer_round_certified.hpp explicitly.
    Requires C++17 and strict floating-point evaluation (no reassociation,
    contraction or flush-to-zero). Does not change buffer().
\details Positive distance dilates. Arc sagitta bounds chord construction
    before export; max_displacement separately bounds Euclidean movement of
    each ideal chord vertex in the stored output. The stored topology is
    certified against the constructed geometry, not against unchorded arcs.
    The native convex dilation path constructs both endpoints of every
    exact straight offset before approximating the intervening arcs.
    Convex erosion uses exact straight offsets and no arcs. Other erosion
    targets the regularized difference with the inscribed boundary band.
    This gives radius-inclusion bounds, not a smooth erosion Hausdorff or
    component bound. A nonempty chord erosion need not imply nonempty
    smooth erosion. MultiPolygon requires pairwise disjoint filled members
    with disjoint boundaries, including at zero distance. Multi inputs use
    one exact arrangement before native conversion. At zero distance the
    admitted areal sources are copied losslessly with the output's
    orientation and closure. Nonpositive point and linear buffers regularize
    to empty. Empty multi inputs or members are allowed.
\param input Source geometry.
\param distance Finite signed radius.
\param arc_sagitta Finite nonnegative chord error budget in coordinate units.
\param max_displacement Finite nonnegative vertex export budget in coordinate units.
\param output Polygon or MultiPolygon with integral, float, double or
    cpp_rational coordinates, or exact_coordinate<std::string>, replaced only
    on success. Exact-coordinate inputs and budgets use their authoritative
    rational value.
    Text output uses the rational backend and requires lossless finite decimal
    vertices regardless of max_displacement. An empty successful output is
    true regularized empty. For chord erosion this
    follows because the ideal chord result contains the smooth erosion
    and export cannot drop positive-area faces.
\return invalid_input for detected malformed input (including an empty
    Linestring or Polygon) or invalid budgets;
    cannot_certify for unsupported cases, representation or capacity limits,
    or any failed construction, topology or export certificate. Numerators
    and denominators of inputs and budgets are limited to 4096 bits. For example,
    zero sagitta cannot produce round arcs, and zero displacement requires
    lossless export. Algebraic line and polygon chord export requires native
    coordinates. The rational chord arrangement is lossless and requires
    rational edge lengths (and for a Linestring, a single shell without holes).
    Nonzero multi chord buffering requires native output coordinates and
    positive sagitta. A Polygon output cannot hold multiple components.
*/
template
<
    typename Geometry, typename Output, typename Distance,
    typename Sagitta, typename Displacement
>
[[nodiscard]] inline certified_buffer_status buffer_round_certified(
    Geometry const& input, Output& output, Distance const& distance,
    Sagitta const& arc_sagitta, Displacement const& max_displacement)
{
    namespace impl = detail::certified_round_buffer;
    using status = certified_buffer_status;
    using tag = tag_t<Geometry>;
    constexpr bool single = std::is_same_v<tag_t<Output>, polygon_tag>;
    static_assert(std::is_same_v<tag, point_tag> || std::is_same_v<tag, segment_tag>
               || std::is_same_v<tag, linestring_tag> || std::is_same_v<tag, polygon_tag>
               || impl::multi_buffer_input<tag>,
                  "Certified round buffer requires a supported 2D Cartesian geometry");
    static_assert(single || std::is_same_v<tag_t<Output>, multi_polygon_tag>,
                  "Certified round buffer requires Polygon or MultiPolygon output");
    static_assert(dimension<Geometry>::value == 2 && dimension<Output>::value == 2
               && std::is_same_v<cs_tag_t<Geometry>, cartesian_tag>
               && std::is_same_v<cs_tag_t<Output>, cartesian_tag>,
                  "Certified round buffer requires 2D Cartesian geometries");
    concepts::check<Geometry const>();
    concepts::check<Output>();

    impl::budget b;
    try
    {
        b = {impl::rational_value(distance), impl::rational_value(arc_sagitta),
             impl::rational_value(max_displacement)};
    }
    catch (std::domain_error const&)
    {
        return status::invalid_input;
    }

    if (b.sagitta < 0 || b.displacement < 0)
    {
        return status::invalid_input;
    }
    using detail::certified_point_buffer::bounded;
    if (! bounded(b.distance) || ! bounded(b.sagitta) || ! bounded(b.displacement))
    {
        return status::cannot_certify;
    }

    if constexpr (std::is_same_v<coordinate_type_t<Output>, impl::common::decimal_coordinate>)
    {
        // Finite decimal text has no nearest grid point. Require exact export,
        // including when the convex erosion producer proposes a rational vertex.
        b.displacement = 0;
    }

    typename impl::output_stage<Output>::type staged;
    try
    {
        auto const result = impl::dispatch(input, staged, b);
        if (result != status::success)
        {
            return result;
        }
        if (b.distance > 0 && geometry::is_empty(staged))
        {
            if constexpr (! impl::multi_buffer_input<tag>)
            {
                return status::cannot_certify;
            }
            else if (! geometry::is_empty(input))
            {
                return status::cannot_certify;
            }
        }
        if constexpr (single)
        {
            if (staged.size() > 1)
            {
                return status::cannot_certify;
            }
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

    if constexpr (single)
    {
        output = staged.empty() ? Output() : std::move(staged.front());
    }
    else
    {
        output = std::move(staged);
    }
    return status::success;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_BUFFER_ROUND_CERTIFIED_HPP
