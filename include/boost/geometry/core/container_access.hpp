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


/*!
\brief Traits class to clear a geometry
\ingroup traits
 */
template <typename Container>
struct clear
{
    static inline void apply(Container& container)
    {
        // The default action: act as it it is a std:: container
        container.clear();
    }
};


/*!
\brief Traits class to append a point to a container (ring, linestring, multi*)
\ingroup traits
 */
template <typename Container>
struct push_back
{
    static inline void apply(Container& container,
        typename boost::range_value<Container>::type const& item)
    {
        // The default action: act as it it is a std:: container
        container.push_back(item);
    }
};



/*!
\brief Traits class to append a point to a container (ring, linestring, multi*)
\ingroup traits
 */
template <typename Container>
struct resize
{
    static inline void apply(Container& container, std::size_t new_size)
    {
        // The default action: act as it it is a std:: container
        container.resize(new_size);
    }
};



} // namespace traits


namespace write
{

// Free functions to conveniently avoid complex metafunctions
// (Mainly) or internal usage


template <typename Container>
inline void clear(Container& container)
{
    traits::clear<Container>::apply(container);
}

template <typename Container>
inline void resize(Container& container, std::size_t new_size)
{
    traits::resize<Container>::apply(container, new_size);
}

template <typename Container>
inline void push_back(Container& container, typename boost::range_value<Container>::type const& item)
{
    traits::push_back<Container>::apply(container, item);
}


}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP
