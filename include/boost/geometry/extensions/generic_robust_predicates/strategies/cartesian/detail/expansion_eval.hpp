// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPANSION_EVAL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPANSION_EVAL_HPP

#include <cstddef>
#include <array>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expansion_arithmetic.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template
<
    typename Expression,
    bool StageB = false,
    operator_types Op = Expression::operator_type
>
struct expansion_size_impl {};

template <typename Expression, bool StageB>
struct expansion_size_impl<Expression, StageB, operator_types::no_op>
{
    static constexpr std::size_t value = 1;
};

template <typename Expression, bool StageB>
struct expansion_size_impl<Expression, StageB, operator_types::sum>
{
private:
    static constexpr std::size_t left_size =
        expansion_size_impl<typename Expression::left, StageB>::value;
    static constexpr std::size_t right_size =
        expansion_size_impl<typename Expression::right, StageB>::value;
public:
    static constexpr std::size_t value = left_size + right_size;
};

template <typename Expression, bool StageB>
struct expansion_size_impl<Expression, StageB, operator_types::difference>
{
private:
    static constexpr std::size_t left_size =
        expansion_size_impl<typename Expression::left, StageB>::value;
    static constexpr std::size_t right_size =
        expansion_size_impl<typename Expression::right, StageB>::value;
public:
    static constexpr std::size_t value =
        StageB && Expression::left::is_leaf && Expression::right::is_leaf ?
              1
            : left_size + right_size;
};

template <typename Expression, bool StageB>
struct expansion_size_impl<Expression, StageB, operator_types::product>
{
private:
    static constexpr std::size_t left_size =
        expansion_size_impl<typename Expression::left, StageB>::value;
    static constexpr std::size_t right_size =
        expansion_size_impl<typename Expression::right, StageB>::value;
public:
    static constexpr std::size_t value =
        std::is_same
            <
                typename Expression::left,
                typename Expression::right
            >::value && left_size == 2 && right_size == 2 ?
              6
            : 2 * left_size * right_size;
};

template <typename Expression> using expansion_size =
    boost::mp11::mp_size_t< expansion_size_impl<Expression, false>::value >;

template <typename Expression> using expansion_size_stage_b =
    boost::mp11::mp_size_t< expansion_size_impl<Expression, true>::value >;

template
<
    typename Expression,
    std::size_t LeftLength,
    std::size_t RightLength,
    operator_types Op = Expression::operator_type
>
constexpr std::size_t final_expansion_size()
{
    if (Op == operator_types::product)
    {
        return 2 * LeftLength * RightLength;
    }
    return 1;
};

template <int> using no_zero_elimination_policy = boost::mp11::mp_false;

template
<
    operator_types Op,
    std::size_t LeftLength,
    std::size_t RightLength,
    bool Inplace,
    typename Iter,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB = false,
    bool LeftEqualsRight = false,
    bool MostSigOnly = false
>
struct perform_op_impl {};

template
<
    std::size_t LeftLength,
    std::size_t RightLength,
    bool Inplace,
    typename Iter,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct perform_op_impl
    <
        operator_types::sum,
        LeftLength,
        RightLength,
        Inplace,
        Iter,
        ZEPolicy,
        FEPolicy,
        StageB,
        LeftEqualsRight,
        MostSigOnly
    >
{
    template <typename ...Args>
    static constexpr Iter apply(Args...args)
    {
        return expansion_plus
            <
                LeftLength,
                RightLength,
                Inplace,
                ZEPolicy,
                FEPolicy,
                MostSigOnly
            >(args...);
    }
};

template
<
    std::size_t LeftLength,
    std::size_t RightLength,
    bool Inplace,
    typename Iter,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct perform_op_impl
    <
        operator_types::difference,
        LeftLength,
        RightLength,
        Inplace,
        Iter,
        ZEPolicy,
        FEPolicy,
        StageB,
        LeftEqualsRight,
        MostSigOnly
    >
{
    template <typename ...Args>
    static constexpr Iter apply(Args...args)
    {
        return expansion_minus
            <
                LeftLength,
                RightLength,
                Inplace,
                StageB,
                ZEPolicy,
                FEPolicy,
                MostSigOnly
            >(args...);
    }
};

template
<
    std::size_t LeftLength,
    std::size_t RightLength,
    bool Inplace,
    typename Iter,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct perform_op_impl
    <
        operator_types::product,
        LeftLength,
        RightLength,
        Inplace,
        Iter,
        ZEPolicy,
        FEPolicy,
        StageB,
        LeftEqualsRight,
        MostSigOnly
    >
{
    template <typename ...Args>
    static constexpr Iter apply(Args...args)
    {
        return expansion_times
            <
                LeftLength,
                RightLength,
                Inplace,
                ZEPolicy,
                FEPolicy,
                LeftEqualsRight
            >(args...);
    }
};

template
<
    typename Evals,
    typename Eval,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB = false,
    operator_types Op = Eval::operator_type,
    bool LeftLeaf = Eval::left::is_leaf,
    bool RightLeaf = Eval::right::is_leaf,
    bool LeftEqualsRight = false,
    bool MostSigOnly = false
>
struct eval_expansion_impl {};

template <bool Ze, std::size_t>
struct set_exp_end
{
    template <typename Iter, typename IterArr>
    static inline void apply(Iter const&, IterArr&) {}
};

template <std::size_t I>
struct set_exp_end<true, I>
{
    template <typename Iter, typename IterArr>
    static inline void apply(Iter const& it, IterArr& ze_ends)
    {
        ze_ends[I] = it;
    }
};

template
<
    typename Evals,
    typename Eval,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    operator_types Op,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct eval_expansion_impl
    <
        Evals,
        Eval,
        Sizes,
        AccumulatedSizes,
        ZEEvals,
        Iter,
        Real,
        ZEPolicy,
        FEPolicy,
        StageB,
        Op,
        true,
        true,
        LeftEqualsRight,
        MostSigOnly
    >
{
private:
    using left = typename Eval::left;
    using right = typename Eval::right;
    using eval_index = boost::mp11::mp_find<Evals, Eval>;
    static constexpr std::size_t size =
        boost::mp11::mp_at<Sizes, eval_index>::value;
    static constexpr std::size_t start =
        boost::mp11::mp_at<AccumulatedSizes, eval_index>::value;
public:
    template <typename IterArr, typename ...Reals>
    static constexpr Iter apply(Iter begin,
                                Iter,
                                IterArr& ze_ends,
                                Reals const&... args)
    {
        std::array<Real, sizeof...(Reals)> input
            {{ static_cast<Real>(args)... }};
        Real left_val = input[left::argn - 1];
        Real right_val = input[right::argn - 1];
        auto end = perform_op_impl
            <
                Op,
                1,
                1,
                false,
                Iter,
                ZEPolicy,
                FEPolicy,
                StageB,
                LeftEqualsRight,
                MostSigOnly
            >::apply(left_val,
                     right_val,
                     begin + start,
                     begin + start + size);
        set_exp_end
            <
                (  boost::mp11::mp_find<ZEEvals, Eval>::value
                 < std::tuple_size<IterArr>::value),
                boost::mp11::mp_find<ZEEvals, Eval>::value
            >::apply(end, ze_ends);
        return end;
    }
};

template <bool Ze, std::size_t>
struct get_exp_end
{
    template <typename Iter, typename IterArr>
    static inline Iter apply(Iter default_end,
                             IterArr const& ze_ends)
    {
        return default_end;
    }
};

template <std::size_t I> struct get_exp_end<true, I>
{
    template <typename Iter, typename IterArr>
    static inline Iter apply(Iter, IterArr const& ze_ends)
    {
        return ze_ends[I];
    }
};

template
<
    typename Evals,
    typename Eval,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    operator_types Op,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct eval_expansion_impl
    <
        Evals,
        Eval,
        Sizes,
        AccumulatedSizes,
        ZEEvals,
        Iter,
        Real,
        ZEPolicy,
        FEPolicy,
        StageB,
        Op,
        true,
        false,
        LeftEqualsRight,
        MostSigOnly
    >
{
private:
    using left = typename Eval::left;
    using right = typename Eval::right;
    using eval_index = boost::mp11::mp_find<Evals, Eval>;
    static constexpr std::size_t size =
        boost::mp11::mp_at<Sizes, eval_index>::value;
    static constexpr std::size_t start =
        boost::mp11::mp_at<AccumulatedSizes, eval_index>::value;
    using right_eval_index = boost::mp11::mp_find<Evals, right>;
    static constexpr std::size_t right_size =
        boost::mp11::mp_at<Sizes, right_eval_index>::value;
    static constexpr std::size_t right_start =
        boost::mp11::mp_at<AccumulatedSizes, right_eval_index>::value;
public:
    template <typename IterArr, typename ...Reals>
    static constexpr Iter apply(Iter begin,
                                Iter,
                                IterArr& ze_ends,
                                Reals const&... args)
    {
        std::array<Real, sizeof...(Reals)> input
            {{ static_cast<Real>(args)... }};
        Real left_val = input[left::argn - 1];
        Iter right_end =
            get_exp_end
                <
                    ZEPolicy<right_size>::value,
                    boost::mp11::mp_find<ZEEvals, right>::value
                >::apply(begin + right_start + right_size, ze_ends);
        Iter end = perform_op_impl
            <
                Op,
                1,
                right_size,
                false,
                Iter,
                ZEPolicy,
                FEPolicy,
                StageB,
                LeftEqualsRight
            >::apply(
            left_val,
            begin + right_start,
            right_end,
            begin + start,
            begin + start + size);
        set_exp_end
            <
                (  boost::mp11::mp_find<ZEEvals, Eval>::value
                 < std::tuple_size<IterArr>::value),
                boost::mp11::mp_find<ZEEvals, Eval>::value
            >::apply(end, ze_ends);
        return end;
    }
};

template
<
    typename Evals,
    typename Eval,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    operator_types Op,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct eval_expansion_impl
    <
        Evals,
        Eval,
        Sizes,
        AccumulatedSizes,
        ZEEvals,
        Iter,
        Real,
        ZEPolicy,
        FEPolicy,
        StageB,
        Op,
        false,
        true,
        LeftEqualsRight,
        MostSigOnly
    >
{
private:
    using left = typename Eval::left;
    using right = typename Eval::right;
    using eval_index = boost::mp11::mp_find<Evals, Eval>;
    static constexpr std::size_t size =
        boost::mp11::mp_at<Sizes, eval_index>::value;
    static constexpr std::size_t start =
        boost::mp11::mp_at<AccumulatedSizes, eval_index>::value;
    using left_eval_index = boost::mp11::mp_find<Evals, left>;
    static constexpr std::size_t left_size =
        boost::mp11::mp_at<Sizes, left_eval_index>::value;
    static constexpr std::size_t left_start =
        boost::mp11::mp_at<AccumulatedSizes, left_eval_index>::value;
public:
    template <typename IterArr, typename ...Reals>
    static constexpr Iter apply(Iter begin,
                                Iter,
                                IterArr& ze_ends,
                                Reals const&... args)
    {
        std::array<Real, sizeof...(Reals)> input
            {{ static_cast<Real>(args)... }};
        Real right_val = input[right::argn - 1];
        Iter left_end =
            get_exp_end
                <
                    ZEPolicy<left_size>::value,
                    boost::mp11::mp_find<ZEEvals, left>::value
                >::apply(begin + left_start + left_size, ze_ends);
        auto end = perform_op_impl
            <
                Op,
                left_size,
                1,
                false,
                Iter,
                ZEPolicy,
                FEPolicy,
                StageB,
                LeftEqualsRight
            >::apply(
            begin + left_start,
            left_end,
            right_val,
            begin + start,
            begin + start + size);
        set_exp_end
            <
                (  boost::mp11::mp_find<ZEEvals, Eval>::value
                 < std::tuple_size<IterArr>::value),
                boost::mp11::mp_find<ZEEvals, Eval>::value
            >::apply(end, ze_ends);
        return end;
    }
};

template
<
    typename Evals,
    typename Eval,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB,
    operator_types Op,
    bool LeftEqualsRight,
    bool MostSigOnly
>
struct eval_expansion_impl
    <
        Evals,
        Eval,
        Sizes,
        AccumulatedSizes,
        ZEEvals,
        Iter,
        Real,
        ZEPolicy,
        FEPolicy,
        StageB,
        Op,
        false,
        false,
        LeftEqualsRight,
        MostSigOnly
    >
{
private:
    using left = typename Eval::left;
    using right = typename Eval::right;
    using eval_index = boost::mp11::mp_find<Evals, Eval>;
    static constexpr std::size_t size =
        boost::mp11::mp_at<Sizes, eval_index>::value;
    static constexpr std::size_t start =
        boost::mp11::mp_at<AccumulatedSizes, eval_index>::value;
    using left_eval_index =
        boost::mp11::mp_find<Evals, left>;
    static constexpr std::size_t left_size =
        boost::mp11::mp_at<Sizes, left_eval_index>::value;
    static constexpr std::size_t left_start =
        boost::mp11::mp_at<AccumulatedSizes, left_eval_index>::value;
    using right_eval_index = boost::mp11::mp_find<Evals, right>;
    static constexpr std::size_t right_size =
        boost::mp11::mp_at<Sizes, right_eval_index>::value;
    static constexpr std::size_t right_start =
        boost::mp11::mp_at<AccumulatedSizes, right_eval_index>::value;
public:
    template <typename IterArr, typename ...Reals>
    static constexpr Iter apply(Iter begin, Iter,
                                IterArr& ze_ends,
                                Reals const&...)
    {
        Iter left_end =
            get_exp_end
                <
                    (  boost::mp11::mp_find<ZEEvals, left>::value
                     < std::tuple_size<IterArr>::value),
                    boost::mp11::mp_find<ZEEvals, left>::value
                >::apply(begin + left_start + left_size, ze_ends);
        Iter right_end =
            get_exp_end
                <
                    (  boost::mp11::mp_find<ZEEvals, right>::value
                     < std::tuple_size<IterArr>::value),
                    boost::mp11::mp_find<ZEEvals, right>::value
                >::apply(begin + right_start + right_size, ze_ends);
        auto end = perform_op_impl
            <
                Op,
                left_size,
                right_size,
                false,
                Iter,
                ZEPolicy,
                FEPolicy,
                StageB,
                LeftEqualsRight,
                MostSigOnly
            >::apply(
                begin + left_start,
                left_end,
                begin + right_start,
                right_end,
                begin + start,
                begin + start + size);
        set_exp_end
            <
                (  boost::mp11::mp_find<ZEEvals, Eval>::value
                 < std::tuple_size<IterArr>::value),
                boost::mp11::mp_find<ZEEvals, Eval>::value
            >::apply(end, ze_ends);
        return end;
    }
};

template
<
    typename Evals,
    typename RemainingEvals,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB = false,
    std::size_t RemainingSize = boost::mp11::mp_size<RemainingEvals>::value
>
struct eval_expansions_impl
{
    template <typename IterArr, typename ...Reals>
    static constexpr Iter apply(Iter begin,
                                Iter end,
                                IterArr& ze_ends,
                                Reals const&... args)
    {
        using eval = boost::mp11::mp_front<RemainingEvals>;
        eval_expansion_impl
            <
                Evals,
                eval,
                Sizes,
                AccumulatedSizes,
                ZEEvals,
                Iter,
                Real,
                ZEPolicy,
                FEPolicy,
                StageB,
                eval::operator_type,
                eval::left::is_leaf,
                eval::right::is_leaf,
                std::is_same<typename eval::left, typename eval::right>::value,
                false
            >::apply(begin, end, ze_ends, args...);
        return eval_expansions_impl
            <
                Evals,
                boost::mp11::mp_pop_front<RemainingEvals>,
                Sizes,
                AccumulatedSizes,
                ZEEvals,
                Iter,
                Real,
                ZEPolicy,
                FEPolicy,
                StageB
            >::apply(begin, end, ze_ends, args...);
    }
};

template <int>
using force_zero_elimination_policy = boost::mp11::mp_true;

template
<
    typename Evals,
    typename RemainingEvals,
    typename Sizes,
    typename AccumulatedSizes,
    typename ZEEvals,
    typename Iter,
    typename Real,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    bool StageB
>
struct eval_expansions_impl
    <
        Evals,
        RemainingEvals,
        Sizes,
        AccumulatedSizes,
        ZEEvals,
        Iter,
        Real,
        ZEPolicy,
        FEPolicy,
        StageB,
        1
    >
{
    template <typename IterArr, typename ...Reals>
    static constexpr Iter apply(Iter begin,
                                Iter end,
                                IterArr& ze_ends,
                                Reals const&... args)
    {
        using eval = boost::mp11::mp_front<RemainingEvals>;

        auto new_end = eval_expansion_impl
            <
                Evals,
                eval,
                Sizes,
                AccumulatedSizes,
                ZEEvals,
                Iter,
                Real,
                ZEPolicy,
                FEPolicy,
                StageB,
                eval::operator_type,
                eval::left::is_leaf,
                eval::right::is_leaf,
                std::is_same
                    <
                        typename eval::left,
                        typename eval::right
                    >::value,
                true
            >::apply(begin, end, ze_ends, args...);
        return new_end;
    }
};

template
<
    template <int> class ZEPolicy,
    bool StageB
>
struct is_zero_elim_q
{
    template <typename Exp>
    using fn = ZEPolicy
        <
            expansion_size_impl<Exp, StageB>::value
        >;
};

template
<
    typename Evals,
    typename Sizes,
    typename AccumulatedSizes,
    typename Iter,
    typename Real,
    bool StageB,
    template <int> class ZEPolicy,
    template <int, int> class FEPolicy,
    typename ...Reals
>
static constexpr Iter eval_expansions(Iter begin,
                                      Iter end,
                                      Reals const&... args)
{
    using ze_evals = boost::mp11::mp_copy_if_q
        <
            Evals,
            is_zero_elim_q<ZEPolicy, StageB>
        >;
    std::array<Iter, boost::mp11::mp_size<ze_evals>::value> ze_ends;
    return eval_expansions_impl
        <
            Evals,
            Evals,
            Sizes,
            AccumulatedSizes,
            ze_evals,
            Iter,
            Real,
            ZEPolicy,
            FEPolicy,
            StageB
        >::apply(begin, end, ze_ends, args...);
}

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPANSION_EVAL_HPP
