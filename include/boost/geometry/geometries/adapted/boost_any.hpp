// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ANY_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ANY_HPP


#include <utility>

#include <boost/any.hpp>

//#include <boost/geometry/core/geometry_types.hpp>
//#include <boost/geometry/core/tag.hpp>
//#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/visit.hpp>
#include <boost/geometry/util/sequence.hpp>
#include <boost/geometry/util/type_traits_std.hpp>


namespace boost { namespace geometry
{

namespace traits
{

// This is done in REGISTER macro. Alternatively this could be done here and boost::any-specific
// macro could only define the types.
/*template <>
struct tag<boost::any>
{
    typedef dynamic_geometry_tag type;
};*/

template <typename TypeSequence, std::size_t N = util::sequence_size<TypeSequence>::value>
struct visit_boost_any
{
    static const std::size_t M = N / 2;

    template <std::size_t Offset, typename Function, typename Any>
    static bool apply(Function function, Any & any)
    {
        return visit_boost_any<TypeSequence, M>::template apply<Offset>(function, any)
            || visit_boost_any<TypeSequence, N - M>::template apply<Offset + M>(function, any);
    }
};

template <typename TypeSequence>
struct visit_boost_any<TypeSequence, 1>
{
    template <std::size_t Offset, typename Function, typename Any>
    static bool apply(Function function, Any & any)
    {
        using elem_t = typename util::sequence_element<Offset, TypeSequence>::type;
        using geom_t = util::transcribe_const_t<Any, elem_t>;
        geom_t * g = boost::any_cast<geom_t>(boost::addressof(any));
        if (g != nullptr)
        {
            function(*g);
            return true;
        }
        else
        {
            return false;
        }
    }
};

template <typename TypeSequence>
struct visit_boost_any<TypeSequence, 0>
{
    template <std::size_t Offset, typename Function, typename Any>
    static bool apply(Function , Any & )
    {
        return false;
    }
};

// TODO: Returning from function

template <>
struct visit<boost::any>
{
    template <typename Function, typename Any>
    static void apply(Function function, Any & any)
    {
        using types_t = typename geometry_types<std::remove_const_t<Any>>::type;
        visit_boost_any<types_t>::template apply<0>(function, any);
    }
};


} // namespace traits


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ANY_HPP
