// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - translators helper functions
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_HELPERS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_HELPERS_HPP

#include <utility>

#include <boost/static_assert.hpp>

#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/and.hpp>

#include <boost/geometry/extensions/index/indexable.hpp>

namespace boost { namespace geometry { namespace index { namespace translator {

namespace dispatch {

// Extract object from std::pair by non-void tag

template <typename Pair, typename FirstTag, typename SecondTag>
struct choose_pair_element
{
    typedef typename Pair::first_type type;
    static type const& get(Pair const& p) { return p.first; }
};

template <typename Pair, typename FirstTag>
struct choose_pair_element<Pair, FirstTag, void>
{
    typedef typename Pair::first_type type;
    static type const& get(Pair const& p) { return p.first; }
};

template <typename Pair, typename Second>
struct choose_pair_element<Pair, void, Second>
{
    typedef typename Pair::second_type type;
    static type const& get(Pair const& p) { return p.second; }
};

template <typename Pair>
struct choose_pair_element<Pair, void, void>
{
    typedef void type;
};

} // namespace dispatch

namespace detail {

template <typename Geometry>
struct indexable_not_found_error
{
    static const bool value = false;
};
template <>
struct indexable_not_found_error<void>
{
    static const bool value = true;
};

// Extract indexable

template <typename Value>
struct extract_indexable
{
    typedef Value type;

    BOOST_STATIC_ASSERT(!indexable_not_found_error<type>::value);

    static type const& get(Value const& v) { return v; }
};

template <typename First, typename Second>
struct extract_indexable< std::pair<First, Second> >
{
    typedef typename dispatch::choose_pair_element<
        std::pair<First, Second>,
        typename traits::indexable_type<First>::type,
        typename traits::indexable_type<Second>::type
    > cp;

    typedef typename cp::type type;

    BOOST_STATIC_ASSERT(!indexable_not_found_error<type>::value);

    static type const& get(std::pair<First, Second> const& v)
    {
        return cp::get(v);
    }
};

// Recognize iterators and smart pointers

BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator_category)
BOOST_MPL_HAS_XXX_TRAIT_DEF(value_type)
BOOST_MPL_HAS_XXX_TRAIT_DEF(difference_type)
BOOST_MPL_HAS_XXX_TRAIT_DEF(pointer)
BOOST_MPL_HAS_XXX_TRAIT_DEF(reference)

BOOST_MPL_HAS_XXX_TRAIT_DEF(element_type)

// TODO
// use has_operator_xxx in the future

template <typename T>
struct is_iterator
{
    static const bool value = boost::mpl::and_
        <
            has_iterator_category<T>,
            has_value_type<T>,
            has_difference_type<T>,
            has_pointer<T>,
            has_reference<T>
        >::value;
};

template <typename T>
struct is_smart_ptr
{
    static const bool value = has_element_type<T>::value;
};

} // namespace detail

//namespace dispatch {
//
//template <typename Geometry, typename Tag>
//struct equals
//{
//    static bool apply(Geometry const& g1, Geometry const& g2)
//    {
//        return geometry::equals(g1, g2);
//    }
//};
//
//template <typename T>
//struct equals<T, void>
//{
//    static bool apply(T const& v1, T const& v2)
//    {
//        return v1 == v2;
//    }
//};
//
//} // namespace dispatch
//
//namespace detail {
//
//template <typename Geometry>
//struct equals
//{
//    static bool apply(Geometry const& g1, Geometry const& g2)
//    {
//        return geometry::equals(g1, g2);
//    }
//};
//
//template <typename First, typename Second>
//struct equals< std::pair<First, Second> >
//{
//    static bool apply(std::pair<First, Second> const& p1, std::pair<First, Second> const& p2)
//    {
//        return
//			dispatch::equals<
//				First,
//				typename traits::tag<First>::type
//			>::apply(p1.first, p2.first)
//			&&
//            dispatch::equals<
//				Second,
//				typename traits::tag<Second>::type
//			>::apply(p1.second, p2.second);
//    }
//};
//
//} // namespace detail



}}}} // namespace boost::geometry::index::translator

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_HELPERS_HPP
