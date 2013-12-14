// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2013 Bruno Lalande, Paris, France.
// Copyright (c) 2013 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESCALE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESCALE_HPP


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL

// To be removed later, when we will initialize a real policy
namespace detail
{

struct no_rescale_policy
{
    // We don't rescale but return the reference. zero cost.
    template <std::size_t Dimension, typename Value>
    inline Value const& apply(Value const& value) const
    {
        return value;
    }
};

} // namespace detail


// By default, the robust type is the point type itself.
// A policy can specialize this.
template <typename Point, typename Policy>
struct robust_point_type
{
    typedef Point type;
};


#endif

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESCALE_HPP
