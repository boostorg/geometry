// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_CLIPPING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_CLIPPING_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/range/size.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_predicates.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root>
class borrowed_clipping
{
public:
    using point_type = borrowed_point<Root>;

private:
    borrowed_context<Root>& m_context;
    std::size_t m_box_range;
    point_type m_lower, m_upper;
    mutable original_edge m_input;

    struct construction
    {
        borrowed_stage<Root> stage;
        std::array<typename Root::operation_type, 2> operations;
        borrowed_record<Root> record;

        template <typename Box>
        construction(borrowed_context<Root>& context, Box const& box,
                     original_edge input, original_edge face)
            : stage(context, box, box)
        {
            // These IDs already name original global ranges, not model-local
            // sources or the possibly clipped endpoint objects.
            stage.finalized = true;
            stage.sources = {};
            operations[0].seg_id = segment_identifier(input.range, -1, -1, input.segment);
            operations[1].seg_id = segment_identifier(face.range, -1, -1, face.segment);
            record.stage = &stage;
            record.operations = {{&operations[0], &operations[1]}};
            point_type const point(record);
            record.numeric.init(point.producers());
        }
    };

    template <std::size_t Dimension, int Direction, std::size_t Face, typename Box>
    bool clip_halfplane(Box const& box, point_type const& bound,
                        point_type& a, point_type& b, bool& clipped_a, bool& clipped_b) const
    {
        int const ca = Direction * root_compare_coordinate<Dimension, Root>(a, bound);
        int const cb = Direction * root_compare_coordinate<Dimension, Root>(b, bound);
        if (ca < 0 && cb < 0) return false;
        if (ca >= 0 && cb >= 0) return true;
        if (ca == 0) { b = a; clipped_b = true; return true; }
        if (cb == 0) { a = b; clipped_a = true; return true; }
        // A zero-length face is an original box corner, not a line producer.
        point_type intersection(bound);
        if (root_compare_coordinate<1 - Dimension, Root>(m_lower, m_upper) != 0)
        {
            auto& retained = m_context.template retain<construction>(
                m_context, box, m_input, original_edge{m_box_range, Face});
            intersection = point_type(retained.record);
        }
        if (ca < 0) { a = std::move(intersection); clipped_a = true; }
        else { b = std::move(intersection); clipped_b = true; }
        return true;
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
    borrowed_clipping(borrowed_context<Root>& context, Box const& box)
        : m_context(context), m_box_range(context.register_range(box))
        , m_lower(context.native_vertex(box, 0)), m_upper(context.native_vertex(box, 2))
    {
        static_assert(std::is_same<geometry::tag_t<Box>, box_tag>::value
            && fixed_native_supported<geometry::coordinate_type_t<Box>>::value,
            "Root clipping requires the original native box");
    }

    template <typename Range>
    void prepare_segment(Range const& range, signed_size_type raw_index) const
    {
        BOOST_ASSERT(raw_index >= 0);
        auto const index = static_cast<std::size_t>(raw_index);
        auto const size = borrowed_detail::range_size(range);
        BOOST_ASSERT(index < size);
        if constexpr (std::is_same<typename boost::range_value<Range>::type, point_type>::value)
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

    template <typename Source, typename Destination>
    void convert_geometry(Source const& source, Destination& destination) const
    {
        borrowed_geometry_converter<Root>(m_context).apply(source, destination);
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
            visible = clip_halfplane<1, 1, 3>(box, m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<1, -1, 1>(box, m_upper, a, b, clipped_a, clipped_b)
                && clip_halfplane<0, 1, 0>(box, m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<0, -1, 2>(box, m_upper, a, b, clipped_a, clipped_b);
        }
        else
        {
            visible = clip_halfplane<0, 1, 0>(box, m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<0, -1, 2>(box, m_upper, a, b, clipped_a, clipped_b)
                && clip_halfplane<1, 1, 3>(box, m_lower, a, b, clipped_a, clipped_b)
                && clip_halfplane<1, -1, 1>(box, m_upper, a, b, clipped_a, clipped_b);
        }
        if (!visible) return false;
        if (Construct)
        {
            first_clipped = clipped_a;
            second_clipped = clipped_b;
            if (clipped_a) segment.first = std::move(a);
            if (clipped_b) segment.second = std::move(b);
        }
        return true;
    }

    template <typename Linestring, typename Iterator>
    static void apply(Linestring& line, Iterator& output)
    {
        if (boost::size(line) > 1)
        {
            *output++ = line;
        }
        geometry::clear(line);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_CLIPPING_HPP
