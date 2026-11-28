// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.
#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_LAZY_DECIMAL_SCALAR_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_LAZY_DECIMAL_SCALAR_HPP

#include <cmath>
#include <cstdint>
#include <deque>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace lazy_decimal_detail
{
using rational = boost::multiprecision::cpp_rational;
enum class op : unsigned char { add, sub, mul, div };
struct argument
{
    // Zero: exact binary literal; low bit set: borrowed rational; otherwise: operation node.
    std::uintptr_t ref = 0;
    double head = 0;
};
struct node
{
    argument a, b;
    rational const* cached;
    op operation;
};
static_assert(alignof(node) >= 2 && alignof(rational) >= 2, "Tagged source handles need one spare bit.");

class arena
{
    arena* previous;
    std::deque<node> nodes;
    std::deque<rational> exact;
    std::vector<node*> stack;

    static rational const& value(argument x, rational& literal)
    {
        if (x.ref == 0) { literal = x.head; return literal; }
        if (x.ref & 1) return *reinterpret_cast<rational const*>(x.ref - 1);
        return *reinterpret_cast<node*>(x.ref)->cached;
    }
public:
    static arena*& active() { static thread_local arena* a = nullptr; return a; }
    static arena& current()
    {
        if (!active()) throw std::logic_error("Decimal expression outside geometry operation");
        return *active();
    }
    arena() : previous(active()) { active() = this; }
    ~arena() { active() = previous; }
    arena(arena const&) = delete;
    arena& operator=(arena const&) = delete;
    std::uintptr_t store(argument a, argument b, op o)
    {
        nodes.push_back({a,b,nullptr,o});
        return reinterpret_cast<std::uintptr_t>(&nodes.back());
    }
    rational const& own(rational v) { exact.push_back(std::move(v)); return exact.back(); }

    rational const& resolve(argument x, rational& tmp)
    {
        if (x.ref && !(x.ref & 1))
        {
            auto* root = reinterpret_cast<node*>(x.ref);
            if (!root->cached) stack.push_back(root);
            while (!stack.empty())
            {
                auto* n = stack.back();
                auto pending = [](argument y) -> node*
                {
                    if (!y.ref || (y.ref & 1)) return nullptr;
                    auto* p = reinterpret_cast<node*>(y.ref);
                    return p->cached ? nullptr : p;
                };
                if (auto* p = pending(n->a)) { stack.push_back(p); continue; }
                if (auto* p = pending(n->b)) { stack.push_back(p); continue; }
                rational la, lb;
                auto const& a = value(n->a,la);
                auto const& b = value(n->b,lb);
                auto& out = exact.emplace_back();
                switch (n->operation)
                {
                case op::add: out = a+b; break;
                case op::sub: out = a-b; break;
                case op::mul: out = a*b; break;
                case op::div: out = a/b; break;
                }
                n->cached = &out;
                stack.pop_back();
            }
        }
        return value(x,tmp);
    }
};
}

class lazy_decimal_scalar
{
    using argument = lazy_decimal_detail::argument;
    using op = lazy_decimal_detail::op;
    using rational = lazy_decimal_detail::rational;
    argument arg;
    double err = 0;
    static constexpr double eps = std::numeric_limits<double>::epsilon();
    static constexpr double tiny = (std::numeric_limits<double>::min)();
    static constexpr double inf = std::numeric_limits<double>::infinity();

    lazy_decimal_scalar(argument a, double e) : arg(a), err(e) {}
    bool nearest() const { return !arg.ref || (arg.ref & 1); }
    bool literal(double d) const { return !arg.ref && arg.head == d; }
    static bool same(lazy_decimal_scalar a, lazy_decimal_scalar b)
    {
        return a.arg.ref == b.arg.ref && (a.arg.ref || a.arg.head == b.arg.head);
    }
    static lazy_decimal_scalar derive(lazy_decimal_scalar a, lazy_decimal_scalar b,
                                      op o, double z, double error)
    {
        // Allow for rounding in the non-negative bound expression (at most
        // eight operations on any path), including absolute underflow losses.
        error = error * (1 + 16 * eps) + tiny;
        if (!std::isfinite(z) || !std::isfinite(error)) error = inf;
        return {{scope::current().store(a.arg,b.arg,o),z},error};
    }
    static int cmp(lazy_decimal_scalar a, lazy_decimal_scalar b)
    {
        if (same(a,b)) return 0;
        // Monotone rounding proves strict order of two unequal nearest heads,
        // even for two finite decimal inputs which overflow to opposite signs.
        if (a.nearest() && b.nearest())
        {
            if (a.arg.head < b.arg.head) return -1;
            if (a.arg.head > b.arg.head) return 1;
        }
        // Inverse-envelope sentinels are literal infinities, never a decimal.
        if (!a.arg.ref && std::isinf(a.arg.head)) return a.arg.head < 0 ? -1 : 1;
        if (!b.arg.ref && std::isinf(b.arg.head)) return b.arg.head < 0 ? 1 : -1;
        if (a.arg.head + a.err < b.arg.head - b.err) return -1;
        if (a.arg.head - a.err > b.arg.head + b.err) return 1;
        rational ta,tb;
        auto const& x = scope::current().resolve(a.arg,ta);
        auto const& y = scope::current().resolve(b.arg,tb);
        return x.compare(y);
    }
public:
    using scope = lazy_decimal_detail::arena;
    lazy_decimal_scalar() = default;
    lazy_decimal_scalar(double v) : arg{0,v} {}
    template <typename I, std::enable_if_t<std::is_integral<I>::value, int> = 0>
    lazy_decimal_scalar(I v) : lazy_decimal_scalar(double(v))
    {
        if constexpr (std::numeric_limits<I>::digits > std::numeric_limits<double>::digits)
        {
            // At this binade an inexact integer can round to the first value.
            if (std::abs(arg.head) >= 2 / std::numeric_limits<double>::epsilon())
                *this = from_source(scope::current().own(rational(v)), arg.head, false);
        }
    }
    // The owner of q must remain alive until all uses of the prepared geometry finish.
    static lazy_decimal_scalar from_source(rational const& q, double nearest_head, bool exact = false)
    {
        if (exact) return nearest_head;
        double const e = std::isfinite(nearest_head) ? eps*std::abs(nearest_head) + tiny : inf;
        return {{reinterpret_cast<std::uintptr_t>(&q) | 1, nearest_head}, e};
    }
    rational exact_value() const
    {
        rational tmp;
        return scope::current().resolve(arg,tmp);
    }
    double approximation() const { return arg.head; }
    static int compare(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a, b); }
    friend bool operator<(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a,b)<0; }
    friend bool operator>(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a,b)>0; }
    friend bool operator<=(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a,b)<=0; }
    friend bool operator>=(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a,b)>=0; }
    friend bool operator==(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a,b)==0; }
    friend bool operator!=(lazy_decimal_scalar a, lazy_decimal_scalar b) { return cmp(a,b)!=0; }
    friend lazy_decimal_scalar operator+(lazy_decimal_scalar a, lazy_decimal_scalar b)
    {
        if (a.literal(0)) return b;
        if (b.literal(0)) return a;
        double const z = a.arg.head + b.arg.head;
        return derive(a,b,op::add,z,a.err+b.err+eps*std::abs(z));
    }
    friend lazy_decimal_scalar operator-(lazy_decimal_scalar a, lazy_decimal_scalar b)
    {
        if (same(a,b)) return 0;
        if (b.literal(0)) return a;
        double const z = a.arg.head - b.arg.head;
        return derive(a,b,op::sub,z,a.err+b.err+eps*std::abs(z));
    }
    friend lazy_decimal_scalar operator*(lazy_decimal_scalar a, lazy_decimal_scalar b)
    {
        if (a.literal(0) || b.literal(0)) return 0;
        if (a.literal(1)) return b;
        if (b.literal(1)) return a;
        double const z = a.arg.head * b.arg.head;
        double const e = std::abs(a.arg.head)*b.err + std::abs(b.arg.head)*a.err
                       + a.err*b.err + eps*std::abs(z);
        return derive(a,b,op::mul,z,e);
    }
    friend lazy_decimal_scalar operator/(lazy_decimal_scalar a, lazy_decimal_scalar b)
    {
        if (b.literal(1)) return a;
        double const z = a.arg.head/b.arg.head;
        double const den = std::abs(b.arg.head);
        // |B| >= den/2. Bound the numerator perturbation before division;
        // the tiny term there accounts for underflow *before* rescaling.
        double const e = den >= tiny && b.err <= den*0.5
            ? 2*((a.err + std::abs(z)*b.err + tiny)/den + eps*std::abs(z)) : inf;
        return derive(a,b,op::div,z,e);
    }
    lazy_decimal_scalar operator-() const
    {
        return !arg.ref ? lazy_decimal_scalar(-arg.head) : lazy_decimal_scalar(0) - *this;
    }
    lazy_decimal_scalar& operator+=(lazy_decimal_scalar b) { return *this = *this+b; }
    lazy_decimal_scalar& operator-=(lazy_decimal_scalar b) { return *this = *this-b; }
    lazy_decimal_scalar& operator*=(lazy_decimal_scalar b) { return *this = *this*b; }
    lazy_decimal_scalar& operator/=(lazy_decimal_scalar b) { return *this = *this/b; }
    friend lazy_decimal_scalar abs(lazy_decimal_scalar x) { return x < 0 ? -x : x; }
};

}}}}}

namespace std {
template <> class numeric_limits<boost::geometry::strategies::detail::exact_cartesian::lazy_decimal_scalar>
    : public numeric_limits<double>
{
    using scalar = boost::geometry::strategies::detail::exact_cartesian::lazy_decimal_scalar;
public:
    static constexpr bool is_exact = true;
    static constexpr bool is_iec559 = false;
    static scalar (max)() noexcept { return numeric_limits<double>::infinity(); }
    static scalar (lowest)() noexcept { return -numeric_limits<double>::infinity(); }
    static scalar (min)() noexcept { return 0; }
    static scalar epsilon() noexcept { return 0; }
};
}
BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
