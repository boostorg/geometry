// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGY_CARTESIAN_ENVELOPE_BOXES_HPP
#define BOOST_GEOMETRY_STRATEGY_CARTESIAN_ENVELOPE_BOXES_HPP

#include <boost/geometry/algorithms/detail/envelope/initialize.hpp>
#include <boost/geometry/strategy/cartesian/expand_box.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

namespace box_accumulation_detail
{

template <typename Policy>
class box_accumulator
{
public:
    template <typename Box>
    class state
    {
        friend box_accumulator;

        Box m_box;
        bool m_initialized = false;
    };

    template <typename Box, typename Input>
    static void apply(state<Box> & st, Input const& box)
    {
        if (! st.m_initialized)
        {
            Policy::start(st.m_box, box);
            st.m_initialized = true;
        }
        else
        {
            Policy::merge(st.m_box, box);
        }
    }

    template <typename Box>
    static void result(state<Box> const& st, Box & box)
    {
        if (st.m_initialized)
        {
            box = st.m_box;
        }
        else
        {
            Policy::initialize(box);
        }
    }
};

struct cartesian_box_accumulation
{
    template <typename Box>
    static void start(Box& result, Box const& input) { result = input; }

    template <typename Box>
    static void merge(Box& result, Box const& input)
    { strategy::expand::cartesian_box::apply(result, input); }

    template <typename Box>
    static void initialize(Box& box)
    { geometry::detail::envelope::initialize<Box, 0, dimension<Box>::value>::apply(box); }
};

} // namespace box_accumulation_detail

class cartesian_boxes : public box_accumulation_detail::box_accumulator
    <box_accumulation_detail::cartesian_box_accumulation> {};

}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGY_CARTESIAN_ENVELOPE_BOXES_HPP
