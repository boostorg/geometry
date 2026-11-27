// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_BOX_CLIPPING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_BOX_CLIPPING_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/range/empty.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/geometry_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/root_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/box_clipping.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root>
class root_box_clipper
{
public:
    using point_type = point_ref<Root>;

private:
    geometry_context<Root>& m_context;
    std::size_t m_box_range;
    point_type m_lower, m_upper;
    mutable original_edge m_input;

    template <std::size_t Dimension, int Direction, std::size_t Face>
    bool clip_halfplane(point_type const& bound,
                        point_type& a, point_type& b, bool& clipped_a, bool& clipped_b) const
    {
        int const ca = Direction * root_compare_coordinate<Dimension, Root>(a, bound);
        int const cb = Direction * root_compare_coordinate<Dimension, Root>(b, bound);
        return box_clip_detail::clip_halfplane(ca, cb, a, b, clipped_a, clipped_b, [&]
        {
            // A zero-length face is an original box corner, not a line producer.
            if (root_compare_coordinate<1 - Dimension, Root>(m_lower, m_upper) == 0)
                return bound;
            auto& record = m_context.template retain<intersection_record<Root>>();
            record.context = &m_context;
            record.supports = {{m_input, {m_box_range, Face}}};
            point_type const point(record);
            record.numeric.init(point.producers());
            return point;
        });
    }

    template <bool Construct>
    bool clip_point_box(model::referring_segment<point_type>& segment,
                        bool& first_clipped, bool& second_clipped) const
    {
        if (root_orientation<Root>(segment.first, segment.second, m_lower) != 0
            || root_compare_coordinate<0, Root>(segment.first, m_lower)
                * root_compare_coordinate<0, Root>(segment.second, m_lower) > 0
            || root_compare_coordinate<1, Root>(segment.first, m_lower)
                * root_compare_coordinate<1, Root>(segment.second, m_lower) > 0)
        {
            return false;
        }
        if (Construct)
        {
            first_clipped = !root_equal<Root>(segment.first, m_lower);
            second_clipped = !root_equal<Root>(segment.second, m_lower);
            if (first_clipped) segment.first = m_lower;
            if (second_clipped) segment.second = m_lower;
        }
        return true;
    }

public:
    template <typename Box>
    root_box_clipper(geometry_context<Root>& context, Box const& box)
        : m_context(context), m_box_range(context.register_range(box))
        , m_lower(context.native_vertex(box, 0)), m_upper(context.native_vertex(box, 2))
    {
        static_assert(std::is_same<geometry::tag_t<Box>, box_tag>::value
            && fixed_native_supported<geometry::coordinate_type_t<Box>>::value,
            "Root clipping requires the original native box");
    }

    template <typename Range, typename Point>
    void prepare_segment(Range const& range, signed_size_type raw_index,
                         Point const& first, Point const& second,
                         point_type& p1, point_type& p2) const
    {
        BOOST_ASSERT(raw_index >= 0);
        auto const index = static_cast<std::size_t>(raw_index);
        auto const size = source_geometry_detail::range_size(range);
        BOOST_ASSERT(index < size);
        if constexpr (std::is_same<geometry::point_type_t<Range>, point_type>::value)
        {
            auto next = index + 1;
            if constexpr (std::is_same<geometry::tag_t<Range>, ring_tag>::value)
            {
                next %= size;
                if constexpr (geometry::closure<Range>::value == closed)
                    if (next == 0) next = size;
            }
            BOOST_ASSERT(next < range.incoming.size());
            m_input = range.incoming[next];
        }
        else m_input = {m_context.register_range(range), index};
        BOOST_ASSERT(m_input.valid());

        if constexpr (std::is_same<geometry::tag_t<Range>, segment_tag>::value)
        {
            p1 = m_context.native_vertex(range, 0);
            p2 = m_context.native_vertex(range, 1);
        }
        else
        {
            geometry_converter<Root> converter(m_context);
            converter.apply(first, p1);
            converter.apply(second, p2);
        }
    }

    struct equals_point_point_strategy_type
    {
        using cs_tag = cartesian_tag;
        original_edge input;

        template <typename A, typename B>
        static bool apply(A const& a, B const& b) { return root_equal<Root>(a, b); }

        template <typename Range>
        void append_no_duplicates(Range& out, point_type const& point) const
        {
            out.pending = input;
            if (out.empty() || !root_equal<Root>(out.back(), point)) out.push_back(point);
        }
    };

    equals_point_point_strategy_type get_equals_point_point_strategy() const
    {
        BOOST_ASSERT(m_input.valid());
        return {m_input};
    }

    template <bool Construct = true, typename Box>
    bool clip_segment(Box const& box, model::referring_segment<point_type>& segment,
                      bool& first_clipped, bool& second_clipped) const
    {
        BOOST_ASSERT(m_input.valid() && m_context.register_range(box) == m_box_range);
        int const width = root_compare_coordinate<0, Root>(m_upper, m_lower);
        int const height = root_compare_coordinate<1, Root>(m_upper, m_lower);
        if (width < 0 || height < 0) return false;
        if (width == 0 && height == 0)
            return clip_point_box<Construct>(segment, first_clipped, second_clipped);

        auto a = segment.first, b = segment.second;
        bool clipped_a = false, clipped_b = false;
        bool visible;
        if (height == 0)
        {
            // Collapse onto the nondegenerate original horizontal faces before
            // testing the zero-length vertical faces.
            visible = clip_halfplane<1, 1, 3>(m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<1, -1, 1>(m_upper, a, b, clipped_a, clipped_b)
                && clip_halfplane<0, 1, 0>(m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<0, -1, 2>(m_upper, a, b, clipped_a, clipped_b);
        }
        else
        {
            visible = clip_halfplane<0, 1, 0>(m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<0, -1, 2>(m_upper, a, b, clipped_a, clipped_b)
                && clip_halfplane<1, 1, 3>(m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<1, -1, 1>(m_upper, a, b, clipped_a, clipped_b);
        }
        if (!visible) return false;
        box_clip_detail::publish<Construct>(segment, a, b, clipped_a, clipped_b, first_clipped, second_clipped);
        return true;
    }

    template <typename Linestring, typename Iterator>
    static void apply(Linestring& line, Iterator& output)
    {
        if (!boost::empty(line))
        {
            *output++ = line;
        }
        geometry::clear(line);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_BOX_CLIPPING_HPP
