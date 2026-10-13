#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_ERROR_BOUND_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_ERROR_BOUND_HPP

#include <array>
#include <limits>

#include <boost/mp11/utility.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

enum class stage_a_error_propagation_cases {
    exact, op_on_exacts, sum_or_diff, product
};

template
<
    typename Expression,
    operator_arities Arity = Expression::operator_arity
>
constexpr stage_a_error_propagation_cases stage_a_error_propagation_case =
    stage_a_error_propagation_cases::exact;

template <typename Expression>
constexpr stage_a_error_propagation_cases stage_a_error_propagation_case
    <
        Expression,
        operator_arities::binary
    >
    = Expression::left::is_leaf && Expression::right::is_leaf ?
        stage_a_error_propagation_cases::op_on_exacts :
        (Expression::operator_type == operator_types::product ?
         stage_a_error_propagation_cases::product :
         stage_a_error_propagation_cases::sum_or_diff);


template
<
    typename Expression,
    stage_a_error_propagation_cases =
        stage_a_error_propagation_case<Expression>
>
struct stage_a_error_bound {};

template <typename Expression>
struct stage_a_error_bound<Expression, stage_a_error_propagation_cases::exact>
{
    using magnitude = abs<Expression>;
    static constexpr std::array<int, 3> a {0, 0, 0};
};

template <typename Expression>
struct stage_a_error_bound
    <
        Expression,
        stage_a_error_propagation_cases::op_on_exacts
    >
{
    using magnitude = abs<Expression>;
    static constexpr std::array<int, 3> a {1, 0, 0};
};

constexpr std::array<int, 3> coeff_max(const std::array<int, 3> a,
                                       const std::array<int, 3> b)
{
    bool a_bigger =
           a[0] > b[0]
        || (a[0] == b[0] && a[1] > b[1])
        || (a[0] == b[0] && a[1] == b[1] && a[2] > b[2]);
    return a_bigger ? a : b;
}

constexpr std::array<int, 3> coeff_product(const std::array<int, 3> a,
                                           const std::array<int, 3> b)
{
    return std::array<int, 3> {
            a[0] + b[0],
            a[1] + b[1] + a[0] * b[0],
            a[2] + b[2] + a[0] * b[1] + a[1] * b[0]
        };
}

constexpr std::array<int, 3>
coeff_mult_by_1_plus_eps(const std::array<int, 3> a)
{
    return std::array<int, 3> {
            a[0],
            a[1] + a[0],
            a[2] + a[1]
        };
}

constexpr std::array<int, 3> coeff_inc_first(const std::array<int, 3> a)
{
    return std::array<int, 3> { a[0] + 1, a[1], a[2] };
}

constexpr std::array<int, 3>
coeff_div_by_1_minus_eps(const std::array<int, 3> a)
{
    return std::array<int, 3> {
        a[0],
        a[0] + a[1],
        a[0] + a[1] + a[2] + 1
    };
}

template <int N>
constexpr int round_to_next_2_pow() {
    static_assert(N >= 1, "Expects positive integer.");
    int out = 1;
    while(out < N)
    {
        out *= 2;
    }
    return out;
}

template <typename Expression>
struct stage_a_error_bound
    <
        Expression,
        stage_a_error_propagation_cases::sum_or_diff
    >
{
private:
    using leb  = stage_a_error_bound<typename Expression::left>;
    using reb = stage_a_error_bound<typename Expression::right>;
    static constexpr auto max_a = coeff_max(leb::a, reb::a);
public:
    using magnitude = sum<typename leb::magnitude, typename reb::magnitude>;
    static constexpr std::array<int, 3> a =
        coeff_inc_first(coeff_mult_by_1_plus_eps(max_a));
};

template <typename Expression>
struct stage_a_error_bound
    <
        Expression,
        stage_a_error_propagation_cases::product
    >
{
private:
    using leb  = stage_a_error_bound<typename Expression::left>;
    using reb = stage_a_error_bound<typename Expression::right>;
    static constexpr std::array<int, 3> a_prod = coeff_product(leb::a, reb::a);
public:
    using magnitude =
        product<typename leb::magnitude, typename reb::magnitude>;
    static constexpr std::array<int, 3> a =
        coeff_inc_first(coeff_mult_by_1_plus_eps(a_prod));
};

template
<
    typename Expression,
    stage_a_error_propagation_cases =
        stage_a_error_propagation_case<Expression>
>
struct stage_a_condition {};

template <typename Expression>
struct stage_a_condition
    <
        Expression,
        stage_a_error_propagation_cases::sum_or_diff
    >
{
private:
    using leb = stage_a_error_bound<typename Expression::left>;
    using reb = stage_a_error_bound<typename Expression::right>;
    static constexpr auto max_a = coeff_max(leb::a, reb::a);
    static constexpr std::array<int, 3> a =
        coeff_mult_by_1_plus_eps(coeff_mult_by_1_plus_eps(coeff_div_by_1_minus_eps(max_a)));
    static constexpr int c = round_to_next_2_pow<a[0]>();
    static constexpr int eps_square_coeff =
        a[2] > 0 ? c * ((a[1] + 1) / c + 1) : c * (a[1] / c + 1);
public:
    using magnitude = sum<typename leb::magnitude, typename reb::magnitude>;
    static constexpr std::array<int, 2> coefficients {a[0], eps_square_coeff};
};

template <typename Expression>
struct stage_a_condition
    <
        Expression,
        stage_a_error_propagation_cases::product
    >
{
private:
    using leb = stage_a_error_bound<typename Expression::left>;
    using reb = stage_a_error_bound<typename Expression::right>;
    static constexpr auto a_prod = coeff_product(leb::a, reb::a);
    static constexpr std::array<int, 3> a =
        coeff_mult_by_1_plus_eps(coeff_mult_by_1_plus_eps(coeff_div_by_1_minus_eps(a_prod)));
    static constexpr int c = round_to_next_2_pow<a[0]>();
    static constexpr int eps_square_coeff =
        a[2] > 0 ? c * ((a[1] + 1) / c + 1) : c * (a[1] / c + 1);
public:
    using magnitude = product<typename leb::magnitude, typename reb::magnitude>;
    static constexpr std::array<int, 2> coefficients {a[0], eps_square_coeff};
};

template
<
    typename Expression,
    typename CalculationType
>
struct stage_a_error_bound_expression_impl
{
private:
    using ct = CalculationType;
    using stage_a_cond = stage_a_condition<Expression>;
    static constexpr ct eps = std::numeric_limits<ct>::epsilon() / 2.0;
    struct constant : public static_constant_interface<ct>
    {
        static constexpr ct value =
              stage_a_cond::coefficients[0] * eps
            + stage_a_cond::coefficients[1] * eps * eps;
        static constexpr bool non_negative = true;
    };
public:
    using type = product<constant, typename stage_a_cond::magnitude>;
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_ERROR_BOUND_HPP
