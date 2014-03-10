// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013, 2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP

#include <boost/tuple/tuple.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/at.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

enum field { interior = 0, boundary = 1, exterior = 2 };

// TODO add height?

template <std::size_t Width>
class matrix
{
    BOOST_STATIC_ASSERT(Width == 2 || Width == 3);

public:

    static const std::size_t size = Width * Width;
    
    inline matrix()
    {
        ::memset(m_array, 'F', size);
    }

    template <field F1, field F2>
    inline char get() const
    {
        static const bool in_bounds = F1 * Width + F2 < size;
        return get_dispatch<F1, F2>(integral_constant<bool, in_bounds>());
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        static const bool in_bounds = F1 * Width + F2 < size;
        set_dispatch<F1, F2, V>(integral_constant<bool, in_bounds>());
    }

    template <field F1, field F2, char D>
    inline void update()
    {
        static const bool in_bounds = F1 * Width + F2 < size;
        update_dispatch<F1, F2, D>(integral_constant<bool, in_bounds>());
    }
    
    inline const char * data() const
    {
        return m_array;
    }

private:
    template <field F1, field F2>
    inline char get_dispatch(integral_constant<bool, true>) const
    {
        return m_array[F1 * Width + F2];
    }
    template <field F1, field F2>
    inline char get_dispatch(integral_constant<bool, false>) const
    {
        return 'F';
    }

    template <field F1, field F2, char V>
    inline void set_dispatch(integral_constant<bool, true>)
    {
        BOOST_STATIC_ASSERT('0' <= V && V <= '9' || V == 'T' || V == 'F');
        m_array[F1 * Width + F2] = V;
    }
    template <field F1, field F2, char V>
    inline void set_dispatch(integral_constant<bool, false>)
    {}

    template <field F1, field F2, char D>
    inline void update_dispatch(integral_constant<bool, true>)
    {
        BOOST_STATIC_ASSERT('0' <= D && D <= '9');
        char c = m_array[F1 * Width + F2];
        if ( D > c || c > '9')
            m_array[F1 * Width + F2] = D;
    }
    template <field F1, field F2, char D>
    inline void update_dispatch(integral_constant<bool, false>)
    {}

    char m_array[size];
};

// TODO add EnableDimensions parameter?

struct matrix9
{
    static const std::size_t width = 3; // TEMP
};

//struct matrix4
//{
//    static const std::size_t width = 2;
//};

template <typename Matrix>
class matrix_handler
    : private matrix<Matrix::width>
{
    typedef matrix<Matrix::width> base_t;

public:
    typedef std::string result_type;

    static const bool interrupt = false;

    matrix_handler(Matrix const&)
    {}

    result_type result() const
    {
        return std::string(this->data(),
                           this->data() + base_t::size);
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        static_cast<base_t&>(*this).template set<F1, F2, V>();
    }

    template <field F1, field F2, char D>
    inline void update()
    {
        static_cast<base_t&>(*this).template update<F1, F2, D>();
    }
};

// RUN-TIME MASKS

class mask9
{
public:
    static const std::size_t width = 3; // TEMP

    inline mask9(std::string const& de9im_mask)
    {
        // TODO: throw an exception here?
        BOOST_ASSERT(de9im_mask.size() == 9);
        ::memcpy(m_mask, de9im_mask.c_str(), 9);
    }

    template <field F1, field F2>
    inline char get() const
    {
        return m_mask[F1 * 3 + F2];
    }

private:
    char m_mask[9];
};

template <typename Mask, bool InterruptEnabled>
struct interrupt_dispatch
{
    template <field F1, field F2, char V>
    static inline bool apply(Mask const&)
    {
        return false;
    }
};

template <typename Mask>
struct interrupt_dispatch<Mask, true>
{
    template <field F1, field F2, char V>
    static inline bool apply(Mask const& mask)
    {
        char m = mask.template get<F1, F2>();
        return check<V>(m);            
    }

    template <char V>
    static inline bool check(char m)
    {
        if ( V >= '0' && V <= '9' )
        {
            return m == 'F' || ( m < V && m >= '0' && m <= '9' );
        }
        else if ( V == 'T' )
        {
            return m == 'F';
        }
        return false;
    }
};

template <typename Masks, int I = 0, int N = boost::tuples::length<Masks>::value>
struct interrupt_dispatch_tuple
{
    template <field F1, field F2, char V>
    static inline bool apply(Masks const& masks)
    {
        typedef typename boost::tuples::element<I, Masks>::type mask_type;
        mask_type const& mask = boost::get<I>(masks);
        return interrupt_dispatch<mask_type, true>::apply<F1, F2, V>(mask)
            && interrupt_dispatch_tuple<Masks, I+1>::apply<F1, F2, V>(masks);
    }
};

template <typename Masks, int N>
struct interrupt_dispatch_tuple<Masks, N, N>
{
    template <field F1, field F2, char V>
    static inline bool apply(Masks const& )
    {
        return true;
    }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
struct interrupt_dispatch<boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>, true>
{
    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> mask_type;

    template <field F1, field F2, char V>
    static inline bool apply(mask_type const& mask)
    {
        return interrupt_dispatch_tuple<mask_type>::template apply<F1, F2, V>(mask);
    }
};

template <field F1, field F2, char V, bool InterruptEnabled, typename Mask>
inline bool interrupt(Mask const& mask)
{
    return interrupt_dispatch<Mask, InterruptEnabled>
                ::template apply<F1, F2, V>(mask);
}

template <typename Mask>
struct check_dispatch
{
    template <typename Matrix>
    static inline bool apply(Mask const& mask, Matrix const& matrix)
    {
        return per_one<interior, interior>(mask, matrix)
            && per_one<interior, boundary>(mask, matrix)
            && per_one<interior, exterior>(mask, matrix)
            && per_one<boundary, interior>(mask, matrix)
            && per_one<boundary, boundary>(mask, matrix)
            && per_one<boundary, exterior>(mask, matrix)
            && per_one<exterior, interior>(mask, matrix)
            && per_one<exterior, boundary>(mask, matrix)
            && per_one<exterior, exterior>(mask, matrix);
    }

    template <field F1, field F2, typename Matrix>
    static inline bool per_one(Mask const& mask, Matrix const& matrix)
    {
        const char mask_el = mask.template get<F1, F2>();
        const char el = matrix.template get<F1, F2>();

        if ( mask_el == 'F' )
        {
            return el == 'F';
        }
        else if ( mask_el == 'T' )
        {
            return el == 'T' || ( el >= '0' && el <= '9' );
        }
        else if ( mask_el >= '0' && mask_el <= '9' )
        {
            return el == mask_el;
        }

        return true;
    }
};

template <typename Masks, int I = 0, int N = boost::tuples::length<Masks>::value>
struct check_dispatch_tuple
{
    template <typename Matrix>
    static inline bool apply(Masks const& masks, Matrix const& matrix)
    {
        typedef typename boost::tuples::element<I, Masks>::type mask_type;
        mask_type const& mask = boost::get<I>(masks);
        return check_dispatch<mask_type>::apply(mask, matrix)
            || check_dispatch_tuple<Masks, I+1>::apply(masks, matrix);
    }
};

template <typename Masks, int N>
struct check_dispatch_tuple<Masks, N, N>
{
    template <typename Matrix>
    static inline bool apply(Masks const&, Matrix const&)
    {
        return false;
    }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
struct check_dispatch< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> mask_type;

    template <typename Matrix>
    static inline bool apply(mask_type const& mask, Matrix const& matrix)
    {
        return check_dispatch_tuple<mask_type>::template apply(mask, matrix);
    }
};

template <typename Mask, typename Matrix>
inline bool check(Mask const& mask, Matrix const& matrix)
{
    return check_dispatch<Mask>::apply(mask, matrix);
}

template <typename Mask, bool Interrupt>
class mask_handler
    : private matrix<Mask::width>
{
    typedef matrix<Mask::width> base_t;

public:
    typedef bool result_type;

    bool interrupt;

    inline mask_handler(Mask const& m)
        : m_mask(m)
        , interrupt(false)
    {}

    result_type result() const
    {
        return !interrupt
            && check(m_mask, static_cast<base_t const&>(*this));
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        if ( relate::interrupt<F1, F2, V, Interrupt>(m_mask) )
        {
            interrupt = true;
        }
        else
        {
            base_t::template set<F1, F2, V>();
        }
    }

    template <field F1, field F2, char V>
    inline void update()
    {
        base_t::template update<F1, F2, V>();
    }

private:
    Mask const& m_mask;
};

// STATIC MASK

template <char II, char IB, char IE,
          char BI, char BB, char BE,
          char EI, char EB, char EE>
class static_mask
{
    typedef boost::mpl::vector_c
                <
                    char, II, IB, IE, BI, BB, BE, EI, EB, EE
                > vector_type;

public:
    template <field F1, field F2>
    struct get
    {
        BOOST_STATIC_ASSERT(F1 * 3 + F2 < boost::mpl::size<vector_type>::value);

        static const char value
            = boost::mpl::at_c<vector_type, F1 * 3 + F2>::type::value;
    };
};

template <typename StaticMask, field F1, field F2>
struct static_should_handle_element
{
    static const char mask_el = StaticMask::template get<F1, F2>::value;
    static const bool value = mask_el == 'F'
                           || mask_el == 'T'
                           || ( mask_el >= '0' && mask_el <= '9' );
};

template <typename StaticMask, char V, field F1, field F2, bool InterruptEnabled, bool IsNotSequence>
struct static_interrupt_dispatch
{
    static const bool value = false;
};

template <typename StaticMask, char V, field F1, field F2, bool IsNotSequence>
struct static_interrupt_dispatch<StaticMask, V, F1, F2, true, IsNotSequence>
{
    static const char mask_el = StaticMask::template get<F1, F2>::value;

    static const bool value
        = ( V >= '0' && V <= '9' ) ? 
          ( mask_el == 'F' || ( mask_el < V && mask_el >= '0' && mask_el <= '9' ) ) :
          ( ( V == 'T' ) ? mask_el == 'F' : false );
};

template <typename First, typename Last, char V, field F1, field F2>
struct static_interrupt_sequence
{
    typedef typename boost::mpl::deref<First>::type StaticMask;

    static const bool value
        = static_interrupt_dispatch
            <
                StaticMask,
                V, F1, F2,
                true,
                !boost::mpl::is_sequence<StaticMask>::value
            >::value
       && static_interrupt_sequence
            <
                typename boost::mpl::next<First>::type,
                Last,
                V, F1, F2
            >::value;
};

template <typename Last, char V, field F1, field F2>
struct static_interrupt_sequence<Last, Last, V, F1, F2>
{
    static const bool value = true;
};

template <typename StaticMask, char V, field F1, field F2>
struct static_interrupt_dispatch<StaticMask, V, F1, F2, true, false>
{
    static const bool value
        = static_interrupt_sequence
            <
                typename boost::mpl::begin<StaticMask>::type,
                typename boost::mpl::end<StaticMask>::type,
                V, F1, F2
            >::value;
};

template <typename StaticMask, char V, field F1, field F2, bool EnableInterrupt>
struct static_interrupt
{
    static const bool value
        = static_interrupt_dispatch
            <
                StaticMask,
                V, F1, F2,
                EnableInterrupt,
                !boost::mpl::is_sequence<StaticMask>::value
            >::value;
};

template <typename StaticMask, bool IsNotSequence>
struct static_check_dispatch
{
    template <typename Matrix>
    static inline bool apply(Matrix const& matrix)
    {
        return per_one<interior, interior>::apply(matrix)
            && per_one<interior, boundary>::apply(matrix)
            && per_one<interior, exterior>::apply(matrix)
            && per_one<boundary, interior>::apply(matrix)
            && per_one<boundary, boundary>::apply(matrix)
            && per_one<boundary, exterior>::apply(matrix)
            && per_one<exterior, interior>::apply(matrix)
            && per_one<exterior, boundary>::apply(matrix)
            && per_one<exterior, exterior>::apply(matrix);
    }
    
    template <field F1, field F2>
    struct per_one
    {
        static const char mask_el = StaticMask::template get<F1, F2>::value;
        static const int version
                            = mask_el == 'F' ? 0
                            : mask_el == 'T' ? 1
                            : mask_el >= '0' && mask_el <= '9' ? 2
                            : 3;

        template <typename Matrix>
        static inline bool apply(Matrix const& matrix)
        {
            const char el = matrix.template get<F1, F2>();
            return apply_dispatch(el, integral_constant<int, version>());
        }

        // mask_el == 'F'
        static inline bool apply_dispatch(char el, integral_constant<int, 0>)
        {
            return el == 'F';
        }
        // mask_el == 'T'
        static inline bool apply_dispatch(char el, integral_constant<int, 1>)
        {
            return el == 'T' || ( el >= '0' && el <= '9' );
        }
        // mask_el >= '0' && mask_el <= '9'
        static inline bool apply_dispatch(char el, integral_constant<int, 2>)
        {
            return el == mask_el;
        }
        // else
        static inline bool apply_dispatch(char el, integral_constant<int, 3>)
        {
            return true;
        }
    };
};

template <typename First, typename Last>
struct static_check_sequence
{
    typedef typename boost::mpl::deref<First>::type StaticMask;

    template <typename Matrix>
    static inline bool apply(Matrix const& matrix)
    {
        return static_check_dispatch
                <
                    StaticMask,
                    !boost::mpl::is_sequence<StaticMask>::value
                >::apply(matrix)
            || static_check_sequence
                <
                    typename boost::mpl::next<First>::type,
                    Last
                >::apply(matrix);
    }
};

template <typename Last>
struct static_check_sequence<Last, Last>
{
    template <typename Matrix>
    static inline bool apply(Matrix const& matrix)
    {
        return false;
    }
};

template <typename StaticMask>
struct static_check_dispatch<StaticMask, false>
{
    template <typename Matrix>
    static inline bool apply(Matrix const& matrix)
    {
        return static_check_sequence
                <
                    typename boost::mpl::begin<StaticMask>::type,
                    typename boost::mpl::end<StaticMask>::type
                >::apply(matrix);
    }
};

template <typename StaticMask>
struct static_check
{
    template <typename Matrix>
    static inline bool apply(Matrix const& matrix)
    {
        return static_check_dispatch
                <
                    StaticMask,
                    !boost::mpl::is_sequence<StaticMask>::value
                >::apply(matrix);
    }
};

template <typename StaticMask, bool Interrupt>
class static_mask_handler
    : private matrix<3>
{
    typedef matrix<3> base_t;

public:
    typedef bool result_type;

    bool interrupt;

    inline static_mask_handler()
        : interrupt(false)
    {}

    result_type result() const
    {
        return (!Interrupt || !interrupt)
            && static_check<StaticMask>::apply(static_cast<base_t const&>(*this));
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        static const bool interrupt_c = static_interrupt<StaticMask, V, F1, F2, Interrupt>::value;
        static const bool should_handle = static_should_handle_element<StaticMask, F1, F2>::value;
        static const int version = interrupt_c ? 0
                                 : should_handle ? 1
                                 : 2;

        set_dispatch<F1, F2, V>(integral_constant<bool, interrupt_c>());
    }

    template <field F1, field F2, char V>
    inline void update()
    {
        static const bool should_handle = static_should_handle_element<StaticMask, F1, F2>::value;
        update_dispatch<F1, F2, V>(integral_constant<bool, should_handle>());
    }

private:
    // Interrupt && interrupt
    template <field F1, field F2, char V>
    inline void set_dispatch(integral_constant<int, 0>)
    {
        interrupt = true;
    }
    // else should_handle
    template <field F1, field F2, char V>
    inline void set_dispatch(integral_constant<int, 1>)
    {
        base_t::template set<F1, F2, V>();
    }
    // else
    template <field F1, field F2, char V>
    inline void set_dispatch(integral_constant<int, 2>)
    {}

    // should_handle
    template <field F1, field F2, char V>
    inline void update_dispatch(integral_constant<bool, true>)
    {
        base_t::template update<F1, F2, V>();
    }
    // else
    template <field F1, field F2, char V>
    inline void update_dispatch(integral_constant<bool, false>)
    {}
};

// PREDEFINED MASKS

// EQUALS
//typedef static_mask<'T', '*', 'F', '*', '*', 'F', 'F', 'F', '*'> static_mask_equals; // wikipedia
typedef static_mask<'T', 'F', 'F', 'F', 'T', 'F', 'F', 'F', 'T'> static_mask_equals; // OGC

// DISJOINT
typedef static_mask<'F', 'F', '*', 'F', 'F', '*', '*', '*', '*'> static_mask_disjoint;

// TOUCHES - NOT P/P
template <typename Geometry1,
          typename Geometry2,
          std::size_t Dimension1 = geometry::dimension<Geometry1>::value,
          std::size_t Dimension2 = geometry::dimension<Geometry2>::value>
struct static_mask_touches_type
{
    typedef boost::mpl::vector<
                static_mask<'F', 'T', '*', '*', '*', '*', '*', '*', '*'>,
                static_mask<'F', '*', '*', 'T', '*', '*', '*', '*', '*'>,
                static_mask<'F', '*', '*', '*', 'T', '*', '*', '*', '*'>
        > type;
};
// According to OGC, doesn't apply to P/P
// Using the above mask the result would be always false
template <typename Geometry1, typename Geometry2>
struct static_mask_touches_type<Geometry1, Geometry2, 0, 0>
    : not_implemented<typename geometry::tag<Geometry1>::type,
                      typename geometry::tag<Geometry2>::type>
{};

// WITHIN
typedef static_mask<'T', '*', 'F', '*', '*', 'F', '*', '*', '*'> static_mask_within;

// COVERED_BY (non OGC)
typedef boost::mpl::vector<
            static_mask<'T', '*', 'F', '*', '*', 'F', '*', '*', '*'>,
            static_mask<'*', 'T', 'F', '*', '*', 'F', '*', '*', '*'>,
            static_mask<'*', '*', 'F', 'T', '*', 'F', '*', '*', '*'>,
            static_mask<'*', '*', 'F', '*', 'T', 'F', '*', '*', '*'>
        > static_mask_covered_by;

// CROSSES
// dim(G1) < dim(G2) - P/L P/A L/A
template <typename Geometry1,
          typename Geometry2,
          std::size_t Dimension1 = geometry::dimension<Geometry1>::value,
          std::size_t Dimension2 = geometry::dimension<Geometry2>::value,
          bool D1LessD2 = (Dimension1 < Dimension2)
>
struct static_mask_crosses_type
{
    typedef static_mask<'T', '*', 'T', '*', '*', '*', '*', '*', '*'> type;
};
// TODO: I'm not sure if this one below should be available!
// dim(G1) > dim(G2) - L/P A/P A/L
template <typename Geometry1, typename Geometry2,
          std::size_t Dimension1, std::size_t Dimension2
>
struct static_mask_crosses_type<Geometry1, Geometry2, Dimension1, Dimension2, false>
{
    typedef static_mask<'T', '*', '*', '*', '*', '*', 'T', '*', '*'> type;
};
// dim(G1) == dim(G2) - P/P A/A
template <typename Geometry1, typename Geometry2,
          std::size_t Dimension, bool D1LessD2
>
struct static_mask_crosses_type<Geometry1, Geometry2, Dimension, Dimension, D1LessD2/*false*/>
    : not_implemented<typename geometry::tag<Geometry1>::type,
                      typename geometry::tag<Geometry2>::type>
{};
// dim(G1) == 1 && dim(G2) == 1 - L/L
template <typename Geometry1, typename Geometry2, bool D1LessD2>
struct static_mask_crosses_type<Geometry1, Geometry2, 1, 1, D1LessD2>
{
    typedef static_mask<'0', '*', '*', '*', '*', '*', '*', '*', '*'> type;
};

// OVERLAPS

// dim(G1) != dim(G2) - NOT P/P, L/L, A/A
template <typename Geometry1,
          typename Geometry2,
          std::size_t Dimension1 = geometry::dimension<Geometry1>::value,
          std::size_t Dimension2 = geometry::dimension<Geometry2>::value
>
struct static_mask_overlaps_type
    : not_implemented<typename geometry::tag<Geometry1>::type,
                      typename geometry::tag<Geometry2>::type>
{};
// dim(G1) == D && dim(G2) == D - P/P A/A
template <typename Geometry1, typename Geometry2, std::size_t Dimension>
struct static_mask_overlaps_type<Geometry1, Geometry2, Dimension, Dimension>
{
    typedef static_mask<'T', '*', 'T', '*', '*', '*', 'T', '*', '*'> type;
};
// dim(G1) == 1 && dim(G2) == 1 - L/L
template <typename Geometry1, typename Geometry2>
struct static_mask_overlaps_type<Geometry1, Geometry2, 1, 1>
{
    typedef static_mask<'1', '*', 'T', '*', '*', '*', 'T', '*', '*'> type;
};

// RESULTS/HANDLERS UTILS

template <field F1, field F2, char V, typename Result>
inline void set(Result & res)
{
    res.template set<F1, F2, V>();
}

template <field F1, field F2, char V, bool Transpose>
struct set_dispatch
{
    template <typename Result>
    static inline void apply(Result & res)
    {
        res.template set<F1, F2, V>();
    }
};

template <field F1, field F2, char V>
struct set_dispatch<F1, F2, V, true>
{
    template <typename Result>
    static inline void apply(Result & res)
    {
        res.template set<F2, F1, V>();
    }
};

template <field F1, field F2, char V, bool Transpose, typename Result>
inline void set(Result & res)
{
    set_dispatch<F1, F2, V, Transpose>::apply(res);
}

template <char V, typename Result>
inline void set(Result & res)
{
    res.template set<interior, interior, V>();
    res.template set<interior, boundary, V>();
    res.template set<interior, exterior, V>();
    res.template set<boundary, interior, V>();
    res.template set<boundary, boundary, V>();
    res.template set<boundary, exterior, V>();
    res.template set<exterior, interior, V>();
    res.template set<exterior, boundary, V>();
    res.template set<exterior, exterior, V>();
}

template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, typename Result>
inline void set(Result & res)
{
    res.template set<interior, interior, II>();
    res.template set<interior, boundary, IB>();
    res.template set<interior, exterior, IE>();
    res.template set<boundary, interior, BI>();
    res.template set<boundary, boundary, BB>();
    res.template set<boundary, exterior, BE>();
    res.template set<exterior, interior, EI>();
    res.template set<exterior, boundary, EB>();
    res.template set<exterior, exterior, EE>();
}

template <field F1, field F2, char D, typename Result>
inline void update(Result & res)
{
    res.template update<F1, F2, D>();
}

template <field F1, field F2, char D, bool Transpose>
struct update_result_dispatch
{
    template <typename Result>
    static inline void apply(Result & res)
    {
        update<F1, F2, D>(res);
    }
};

template <field F1, field F2, char D>
struct update_result_dispatch<F1, F2, D, true>
{
    template <typename Result>
    static inline void apply(Result & res)
    {
        update<F2, F1, D>(res);
    }
};

template <field F1, field F2, char D, bool Transpose, typename Result>
inline void update(Result & res)
{
    update_result_dispatch<F1, F2, D, Transpose>::apply(res);
}

template <typename Result, char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE>
inline Result return_result()
{
    Result res;
    set<II, IB, IE, BI, BB, BE, EI, EB, EE>(res);
    return res;
}

template <typename Geometry>
struct result_dimension
{
    BOOST_STATIC_ASSERT(geometry::dimension<Geometry>::value >= 0);
    static const char value
        = ( geometry::dimension<Geometry>::value <= 9 ) ?
            ( '0' + geometry::dimension<Geometry>::value ) :
              'T';
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
