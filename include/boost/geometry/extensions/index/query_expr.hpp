// Boost.Geometry Index
//
// Query expression
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// TODO - CHANGE FILENAME TO query_expr.hpp
// create separate namespace etc.

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_QUERY_EXPR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_QUERY_EXPR_HPP

namespace boost { namespace geometry { namespace index {

namespace query_expr {

// predicates tags

struct empty_tag {};
struct value_tag {};

struct covered_by_tag {};
struct disjoint_tag {};
struct intersects_tag {};
struct overlaps_tag {};
//struct touches_tag {};
struct within_tag {};

struct not_covered_by_tag {};
struct not_disjoint_tag {};
struct not_intersects_tag {};
struct not_overlaps_tag {};
//struct not_touches_tag {};
struct not_within_tag {};

struct nearest_tag {};

// closer_than_tag ?
// further_than_tag ?

// predicates data

template <typename Geometry, typename Tag>
struct predicate
{
    explicit predicate(Geometry const& geometry_) : geometry(geometry_) {}
    Geometry geometry;
};

template <typename ValuePredicate>
struct predicate<ValuePredicate, value_tag>
{
    explicit predicate(ValuePredicate const& value_predicate_) : value_predicate(value_predicate_) {}
    ValuePredicate value_predicate;
};

template <typename Dummy>
struct predicate<Dummy, empty_tag>
{};

template <typename DistancesPredicates>
struct predicate<DistancesPredicates, nearest_tag>
{
    predicate(size_t k_, DistancesPredicates const& distances_predicates_)
        : k(k_), distances_predicates(distances_predicates_)
    {}

    size_t k;
    DistancesPredicates distances_predicates;
};

// expression &&

template <typename Left, typename Right>
struct and_expr
{
    typedef Left left_type;
    typedef Right right_type;

    and_expr(Left const& left_, Right const& right_)
        : left(left_), right(right_)
    {}
    
    Left const& left;
    Right const& right;
};

// predicate_tag
template <typename Predicate>
struct predicate_tag
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_VALID_PREDICATE,
        (Predicate));
};
template <typename Data, typename Tag>
struct predicate_tag< predicate<Data, Tag> >
{
    typedef Tag type;
};

// predicates_length

template <typename Expr>
struct predicates_length
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_VALID_QUERY_EXPRESSION,
        (Expr));
};
template <typename Left, typename Right>
struct predicates_length< and_expr<Left, Right> >
{
    static const size_t value = predicates_length<Left>::value + predicates_length<Right>::value;
};
template <typename Data, typename Tag>
struct predicates_length< predicate<Data, Tag> >
{
    static const size_t value = 1;
};

// if_v

template <bool Cond, size_t V1, size_t V2>
struct if_v
{
    static const size_t value = V1;
};
template <size_t V1, size_t V2>
struct if_v<false, V1, V2>
{
    static const size_t value = V2;
};

// if_t

template <bool Cond, typename T1, typename T2>
struct if_t
{
    typedef T1 type;
};
template <typename T1, typename T2>
struct if_t<false, T1, T2>
{
    typedef T2 type;
};

// if_ret_t
template <bool Cond, typename T>
struct if_ret_t
{
    typedef T const& type;
};
template <typename T>
struct if_ret_t<false, T>
{
    typedef void type;
};

// if_ret
template <bool Cond>
struct if_ret
{
    template <typename T>
    static inline T const& apply(T const& v) { return v; }
};
template <>
struct if_ret<false>
{
    template <typename T>
    void apply(T const&) {}
};

// if_ret2
template <bool Cond, typename L, typename R>
struct if_ret2
{
    template <typename Expr>
    static inline typename L::return_type
    apply(Expr const& e) { return L::apply(e.left); }
};
template <typename L, typename R>
struct if_ret2<false, L, R>
{
    template <typename Expr>
    static inline typename R::return_type
    apply(Expr const& e) { return R::apply(e.right); }
};

// types_equal
template <typename T1, typename T2>
struct types_equal
{
    static const bool value = false;
};
template <typename T>
struct types_equal<T, T>
{
    static const bool value = true;
};

// predicate_access_impl - used by predicate_type and get_predicate()

template <typename Expr, size_t I, size_t Curr = 0>
struct predicate_access
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_VALID_QUERY_EXPRESSION,
        (Expr));
};

template <typename Left, typename Right, size_t I, size_t Curr>
struct predicate_access<and_expr<Left, Right>, I, Curr>
{
    typedef predicate_access<Left, I, Curr> L;
    typedef predicate_access<Right, I, predicate_access<Left, I, Curr>::curr + 1> R;

    static const size_t curr = if_v<
        I == L::curr,
        L::curr,
        R::curr
    >::value;

    typedef typename if_t<
        I == L::curr,
        typename L::type,
        typename R::type
    >::type type;

    typedef typename if_t<
        I == L::curr,
        typename L::return_type,
        typename R::return_type
    >::type return_type;

    static inline return_type
    apply(and_expr<Left, Right> const& e)
    {
        return if_ret2<I == L::curr, L, R>::apply(e);
    }
};

template <typename Data, typename Tag, size_t I, size_t Curr>
struct predicate_access<predicate<Data, Tag>, I, Curr>
{
    static const size_t curr = Curr;

    typedef typename if_t<I == Curr, predicate<Data, Tag>, void>::type type;

    typedef typename if_ret_t<I == Curr, type>::type return_type;

    static inline return_type apply(predicate<Data, Tag> const& p)
    {
        return if_ret<I == Curr>::apply(p);
    }
};

// predicate_type

template <typename Expr, size_t I>
struct predicate_type
{
    typedef typename predicate_access<Expr, I>::type type;
};

// get_predicate

template <size_t I, typename Expr>
typename predicate_access<Expr, I>::return_type
get_predicate(Expr const& expr)
{
    return predicate_access<Expr, I>::apply(expr);
};

// predicate_index
template <typename Expr, typename Tag, size_t I, size_t N>
struct predicate_index_impl
{
    static const size_t value = if_v<
        types_equal<
            Tag,
            typename predicate_tag<
                typename predicate_type<Expr, I>::type
            >::type
        >::value,
        I,
        predicate_index_impl<Expr, Tag, I+1, N>::value
    >::value;
};
template <typename Expr, typename Tag, size_t N>
struct predicate_index_impl<Expr, Tag, N, N>
{
    static const size_t value = N;
};

template <typename Expr, typename Tag, size_t I = 0>
struct predicate_index
{
    static const size_t value =
        predicate_index_impl<
            Expr, Tag, I, predicates_length<Expr>::value
        >::value;
};

// count_predicate
template <typename Expr, typename Tag, size_t I, size_t N>
struct count_predicate_impl
{
    static const size_t value =
        count_predicate_impl<Expr, Tag, I+1, N>::value +
        if_v<
            types_equal<
                Tag,
                typename predicate_tag<
                    typename predicate_type<Expr, I>::type
                >::type
            >::value,
            1,
            0
        >::value;
};
template <typename Expr, typename Tag, size_t N>
struct count_predicate_impl<Expr, Tag, N, N>
{
    static const size_t value = 0;
};

template <typename Expr, typename Tag, size_t I = 0>
struct count_predicate
{
    static const size_t value =
        count_predicate_impl<
            Expr, Tag, I, predicates_length<Expr>::value
        >::value;
};

// count_predicate

} // namespace query_expr

}}} // namespace boost::geometry::index

template <typename Data1, typename Tag1, typename Data2, typename Tag2>
boost::geometry::index::query_expr::and_expr<
    boost::geometry::index::query_expr::predicate<Data1, Tag1>,
    boost::geometry::index::query_expr::predicate<Data2, Tag2>
>
operator&&(boost::geometry::index::query_expr::predicate<Data1, Tag1> const& left,
           boost::geometry::index::query_expr::predicate<Data2, Tag2> const& right)
{
    using namespace boost::geometry::index::query_expr;

    return and_expr< predicate<Data1, Tag1>, predicate<Data2, Tag2> >(left, right);
}

template <typename Left, typename Right, typename Data, typename Tag>
boost::geometry::index::query_expr::and_expr<
    boost::geometry::index::query_expr::and_expr<Left, Right>,
    boost::geometry::index::query_expr::predicate<Data, Tag>
>
operator&&(boost::geometry::index::query_expr::and_expr<Left, Right> const& left,
           boost::geometry::index::query_expr::predicate<Data, Tag> const& right)
{
    using namespace boost::geometry::index::query_expr;

    return and_expr< and_expr<Left, Right>, predicate<Data, Tag> >(left, right);
}

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_QUERY_EXPR_HPP
