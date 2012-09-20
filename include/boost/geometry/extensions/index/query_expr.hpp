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
struct nearest_tag {};

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

// predicates_count

template <typename Expr>
struct predicates_count
{
    BOOST_MPL_ASSERT(
        (false),
        NOT_VALID_QUERY_EXPRESSION,
        (Expr));
};

template <typename Left, typename Right>
struct predicates_count< and_expr<Left, Right> >
{
    static const size_t value = predicates_count<Left>::value + predicates_count<Right>::value;
};

template <typename Data, typename Tag>
struct predicates_count< predicate<Data, Tag> >
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

// predicate_access_impl - used by predicate_type and get_predicate()

template <typename Expr, size_t I, size_t Curr = 0>
struct predicate_access
{
    BOOST_MPL_ASSERT(
        (false),
        NOT_VALID_QUERY_EXPRESSION,
        (Expr));
};

template <typename Left, typename Right, size_t I, size_t Curr>
struct predicate_access<and_expr<Left, Right>, I, Curr>
{
    static const size_t curr = if_v<
        I == predicate_access<Left, I, Curr>::curr,
        predicate_access<Left, I, Curr>::curr,
        predicate_access<Right, I, predicate_access<Left, I, Curr>::curr + 1>::curr
    >::value;

    typedef typename if_t<
        I == predicate_access<Left, I, Curr>::curr,
        typename predicate_access<Left, I, Curr>::type,
        typename predicate_access<Right, I, predicate_access<Left, I, Curr>::curr + 1>::type
    >::type type;

    //TODO

    /*static inline
    typename if_t<
        I == predicate_access<Left, I, Curr>::curr,
        typename predicate_access<Left, I, Curr>::return_type,
        typename predicate_access<Right, I, predicate_access<Left, I, Curr>::curr + 1>::return_type
    >::type
    apply(and_expr<Left, Right> const& e)
    {
        return if_ret<I == Curr>::apply(p);
    }*/
};

template <typename Data, typename Tag, size_t I, size_t Curr>
struct predicate_access<predicate<Data, Tag>, I, Curr>
{
    static const size_t curr = Curr;

    typedef typename if_t<I == Curr, predicate<Data, Tag>, void>::type type;

    typedef typename if_t<I == Curr, type const&, void>::type return_type;

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
typename predicate_type<Expr, I>::type const&
get_predicate(Expr const& expr)
{
    return predicate_access<Expr, I>::apply(expr);
};

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
