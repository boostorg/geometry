// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP
#define BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP

#include <cstddef>

#include <boost/type_traits/remove_const.hpp>
#include <boost/concept_check.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>


namespace boost { namespace geometry
{


namespace traits
{

template <typename Range>
struct rvalue_type
{
    typedef typename boost::remove_reference<Range>::type& type;
};

/*!
\brief Traits class to clear a geometry
\ingroup traits
 */
template <typename Range>
struct clear
{
    static inline void apply(typename rvalue_type<Range>::type range)
    {
        // The default action: act as it it is a std:: container
        range.clear();
    }
};


/*!
\brief Traits class to append a point to a range (ring, linestring, multi*)
\ingroup traits
 */
template <typename Range>
struct push_back
{
    static inline void apply(typename rvalue_type<Range>::type range,
                typename boost::range_value
                    <
                        typename boost::remove_reference<Range>::type
                    >::type const& item)
    {
        // The default action: act as it it is a std:: container
        range.push_back(item);
    }
};



/*!
\brief Traits class to append a point to a range (ring, linestring, multi*)
\ingroup traits
 */
template <typename Range>
struct resize
{
    static inline void apply(typename rvalue_type<Range>::type range, 
                std::size_t new_size)
    {
        // The default action: act as it it is a std:: container
        range.resize(new_size);
    }
};


} // namespace traits


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP
