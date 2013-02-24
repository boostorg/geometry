// Boost.Geometry Index
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_TRANSLATOR_TRANSLATOR_HPP
#define BOOST_GEOMETRY_INDEX_TRANSLATOR_TRANSLATOR_HPP

// move index::translator::def<> to index::translator<>
// make all index::translator<> methods static
// remove translator::index
// move helpers to index::default::translator

//#include <boost/geometry/index/translator/def.hpp>
//#include <boost/geometry/index/translator/index.hpp>

#include <boost/mpl/assert.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/index/detail/indexable.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace translator {

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

template <typename Geometry, typename Tag>
struct equals
{
    static bool apply(Geometry const& g1, Geometry const& g2)
    {
        return geometry::equals(g1, g2);
    }
};

template <typename T>
struct equals<T, void>
{
    static bool apply(T const& v1, T const& v2)
    {
        return v1 == v2;
    }
};

}} // namespace detail::translator

/*!
\brief The default translator.

It translates Value object to Indexable object. The default version handles Values which are Indexables.
This translator is also specialized for std::pair<Indexable, Second> and boost::tuple<Indexable, ...>.

\tparam Value       The Value type which may be translated directly to the Indexable.
*/
template <typename Value>
struct translator
{
    BOOST_MPL_ASSERT_MSG(
        (!detail::translator::indexable_not_found_error<
            typename detail::traits::indexable_type<Value>::type
         >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Value)
    );

    typedef Value const& result_type;

    result_type operator()(Value const& value) const
    {
        return value;
    }

    bool equals(Value const& v1, Value const& v2) const
    {
        return geometry::equals(v1, v2);
    }
};

/*!
\brief The default translator.

This specialization translates from std::pair<Indexable, Second>.

\tparam Indexable       The Indexable type.
\tparam Second          The second type.
*/
template <typename Indexable, typename Second>
struct translator< std::pair<Indexable, Second> >
{
    BOOST_MPL_ASSERT_MSG(
        (!detail::translator::indexable_not_found_error<
            typename detail::traits::indexable_type<Indexable>::type
         >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Indexable)
    );

    typedef Indexable const& result_type;

    result_type operator()(std::pair<Indexable, Second> const& value) const
    {
        return value.first;
    }

    bool equals(std::pair<Indexable, Second> const& v1, std::pair<Indexable, Second> const& v2) const
    {
        return geometry::equals(v1.first, v2.first)
            &&
            detail::translator::equals<
                Second,
                typename geometry::traits::tag<Second>::type
            >::apply(v1.second, v2.second);
    }
};

namespace detail { namespace translator {
    
template <typename Tuple, size_t I, size_t N>
struct compare_tuples
{
    inline static bool apply(Tuple const& t1, Tuple const& t2)
    {
        typedef typename boost::tuples::element<I, Tuple>::type T;
        return detail::translator::equals<
                    T,
                    typename geometry::traits::tag<T>::type
                >::apply(boost::get<I>(t1), boost::get<I>(t2))
            &&
               compare_tuples<Tuple, I+1, N>::apply(t1, t2);
    }
};

template <typename Tuple, size_t I>
struct compare_tuples<Tuple, I, I>
{
    inline static bool apply(Tuple const&, Tuple const&)
    {
        return true;
    }
};

}} // namespace detail::translator

/*!
\brief The default translator.

This specialization translates from boost::tuple<Indexable, ...>.

\tparam Indexable   The Indexable type.
*/
template <typename Indexable, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
struct translator< boost::tuple<Indexable, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    typedef boost::tuple<Indexable, T1, T2, T3, T4, T5, T6, T7, T8, T9> value_type;

    BOOST_MPL_ASSERT_MSG(
        (!detail::translator::indexable_not_found_error<
            typename detail::traits::indexable_type<Indexable>::type
        >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Indexable)
        );

    typedef Indexable const& result_type;

    result_type operator()(value_type const& value) const
    {
        return boost::get<0>(value);
    }

    bool equals(value_type const& v1, value_type const& v2) const
    {
        return detail::translator::compare_tuples<value_type, 0, boost::tuples::length<value_type>::value>
            ::apply(v1, v2);
    }
};

namespace detail { namespace translator {

template <typename Translator>
struct result_type
{
    typedef typename Translator::result_type type;
};

template <typename Translator>
struct indexable_type
{
    typedef typename boost::remove_const<
        typename boost::remove_reference<
            typename result_type<Translator>::type
        >::type
    >::type type;
};

}} // namespace detail::translator

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_TRANSLATOR_TRANSLATOR_HPP
