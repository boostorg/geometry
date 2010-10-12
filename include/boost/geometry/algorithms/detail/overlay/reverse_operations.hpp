// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_REVERSE_OPERATIONS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_REVERSE_OPERATIONS_HPP


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename Turns>
void reverse_operations(Turns& turns)
{
    for(typename boost::range_iterator<Turns>::type it
        = boost::begin(turns); it != boost::end(turns); ++it)
    {
        for (unsigned int i = 0; i < it->operations.size(); i++)
        {
            operation_type& op = it->operations[i].operation;
            switch(op)
            {
                case operation_union : op = operation_intersection; break;
                case operation_intersection : op = operation_union; break;
            }
        }
    }
}


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_REVERSE_OPERATIONS_HPP
