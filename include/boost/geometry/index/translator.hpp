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
\brief The function object extracting Indexable from Value.

It translates Value object to Indexable object. The default version handles Values which are Indexables.
This template is also specialized for std::pair<Indexable, T2> and boost::tuple<Indexable, ...>.

\tparam Value       The Value type which may be translated directly to the Indexable.
*/
template <typename Value>
struct indexable
{
    BOOST_MPL_ASSERT_MSG(
        (!detail::translator::indexable_not_found_error<
            typename detail::traits::indexable_type<Value>::type
         >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Value)
    );

    typedef Value const& result_type;
    result_type operator()(Value const& v) const
    {
        return v;
    }
};

/*!
\brief The function object extracting Indexable from Value.

This specialization translates from std::pair<Indexable, T2>.

\tparam Indexable       The Indexable type.
\tparam T2              The second type.
*/
template <typename Indexable, typename T2>
struct indexable< std::pair<Indexable, T2> >
{
    BOOST_MPL_ASSERT_MSG(
        (!detail::translator::indexable_not_found_error<
            typename detail::traits::indexable_type<Indexable>::type
         >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Indexable)
    );

    typedef Indexable const& result_type;
    result_type operator()(std::pair<Indexable, T2> const& v) const
    {
        return v.first;
    }
};

/*!
\brief The function object extracting Indexable from Value.

This specialization translates from boost::tuple<Indexable, ...>.

\tparam Indexable   The Indexable type.
*/
template <typename Indexable, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
struct indexable< boost::tuple<Indexable, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
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
    result_type operator()(value_type const& v) const
    {
        return boost::get<0>(v);
    }
};

// equal_to

/*!
\brief The function object comparing Values.

It compares Geometries using geometry::equals() function. Other types are compared using operator==.
The default version handles Values which are Indexables.
This template is also specialized for std::pair<T1, T2> and boost::tuple<...>.

\tparam Value       The type of objects which are compared by this function object.
*/
template <typename Value>
struct equal_to
{
    typedef bool result_type;
    bool operator()(Value const& l, Value const& r) const
    {
        return detail::translator::equals<Value, typename geometry::traits::tag<Value>::type>::apply(l ,r);
    }
};

/*!
\brief The function object comparing Values.

This specialization compares values of type std::pair<T1, T2>.
It compares pairs' first values, then second values.

\tparam T1       The first type.
\tparam T2       The second type.
*/
template <typename T1, typename T2>
struct equal_to< std::pair<T1, T2> >
{
    typedef bool result_type;
    bool operator()(std::pair<T1, T2> const& l, std::pair<T1, T2> const& r) const
    {
        typedef detail::translator::equals<T1, typename geometry::traits::tag<T1>::type> equals1;
        typedef detail::translator::equals<T2, typename geometry::traits::tag<T2>::type> equals2;

        return equals1::apply(l.first, r.first) && equals2::apply(l.second, r.second);
    }
};

/*!
\brief The function object comparing Values.

This specialization compares values of type boost::tuple<...>.
It compares values stored in tuple in range [0, length<tuple<...>>::value).
*/
template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
struct equal_to< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> value_type;

    typedef bool result_type;
    bool operator()(value_type const& l, value_type const& r) const
    {
        return detail::translator::compare_tuples<
            value_type, 0, boost::tuples::length<value_type>::value
        >::apply(l ,r);
    }
};

namespace detail { namespace translator {

template <typename IndexableGetter, typename EqualTo>
struct translator
    : public IndexableGetter
    , public EqualTo
{
    typedef typename IndexableGetter::result_type result_type;

    translator(IndexableGetter const& i, EqualTo const& e)
        : IndexableGetter(i), EqualTo(e)
    {}

    template <typename Value>
    result_type operator()(Value const& value) const
    {
        return IndexableGetter::operator()(value);
    }

    template <typename Value>
    bool equals(Value const& v1, Value const& v2) const
    {
        return EqualTo::operator()(v1, v2);
    }
};

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
