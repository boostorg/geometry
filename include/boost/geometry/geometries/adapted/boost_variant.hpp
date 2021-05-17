// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2018-2021.
// Modifications copyright (c) 2018-2021, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_VARIANT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_VARIANT_HPP


#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>
//#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/visit.hpp>
#include <boost/geometry/util/sequence.hpp>


namespace boost { namespace geometry
{

namespace detail
{

template <typename ...>
struct parameter_pack_first_type {};

template <typename T, typename ... Ts>
struct parameter_pack_first_type<T, Ts...>
{
    typedef T type;
};

// This utility could be used to get the return type of the visit trait in order to implement
//   returning visit for types that don't support it by default like boost::variant or boost::any.
// The return type is the return type of the Function
//   with arguments being the first Variant types preserving constness and references.
template <typename F, typename ...Vs>
struct visit_return_type
{
    template <typename V>
    using first_t = typename util::sequence_front<typename traits::geometry_types<util::remove_cref_t<V>>::type>::type;

    using type = decltype(std::declval<F>()(std::declval<util::transcribe_cref_t<Vs, first_t<Vs>>>()...));
};

} // namespace detail


// TODO: This is not used anywhere in the header files. Only in tests.
template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct point_type<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
    : point_type
        <
            typename detail::parameter_pack_first_type
                <
                    BOOST_VARIANT_ENUM_PARAMS(T)
                >::type
        >
{};


namespace traits
{

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct tag<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
{
    typedef dynamic_geometry_tag type;
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct visit<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
{
    template <typename Function, typename Variant>
    struct visitor
        : boost::static_visitor<>
    {
        visitor(Function function)
            : m_function(function)
        {}

        template <typename Geometry>
        void operator()(Geometry & geometry)
        {
            m_function(geometry);
        }

        Function m_function;
    };

    template <typename Function, typename Variant>
    static void apply(Function function, Variant & variant)
    {
        visitor<Function, Variant &> visitor(function);
        boost::apply_visitor(visitor, variant);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T), BOOST_VARIANT_ENUM_PARAMS(typename U)>
struct visit<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>, boost::variant<BOOST_VARIANT_ENUM_PARAMS(U)>>
{
    template <typename Function, typename Variant1, typename Variant2>
    struct visitor
        : boost::static_visitor<>
    {
        visitor(Function function)
            : m_function(function)
        {}

        template <typename Geometry1, typename Geometry2>
        void operator()(Geometry1 & geometry1, Geometry2 & geometry2)
        {
            m_function(geometry1, geometry2);
        }

        Function m_function;
    };

    template <typename Function, typename Variant1, typename Variant2>
    static void apply(Function function, Variant1 & variant1, Variant2 & variant2)
    {
        visitor<Function, Variant1 &, Variant2 &> visitor(function);
        boost::apply_visitor(visitor, variant1, variant2);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct geometry_types<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
{
    // TODO: not correct, this will put here null_types in VS2015
    typedef util::type_sequence<BOOST_VARIANT_ENUM_PARAMS(T)> type;
};

} // namespace traits


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_VARIANT_HPP
