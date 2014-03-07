// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

enum field { interior = 0, boundary = 1, exterior = 2 };

// TODO add EnableDimensions parameter

template <std::size_t Width>
class matrix
{
    BOOST_STATIC_ASSERT(Width == 2 || Width == 3);

public:

    static const bool interrupt = false;

    static const std::size_t size = Width * Width;
    
    inline matrix()
    {
        ::memset(m_array, 'F', size);
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

class matrix9
    : public matrix<3>
{};

class matrix4
    : public matrix<2>
{};

inline bool check_element(char mask_el, char el)
{
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

template <char MaskEl>
inline bool check_element(char el)
{
    if ( MaskEl == 'F' )
    {
        return el == 'F';
    }
    else if ( MaskEl == 'T' )
    {
        return el == 'T' || ( el >= '0' && el <= '9' );
    }
    else if ( MaskEl >= '0' && MaskEl <= '9' )
    {
        return el == mask_el;
    }

    return true;
}

template <char V>
inline bool interrupt(char mask_el)
{
    if ( V >= '0' && V <= '9' )
    {
        return mask_el == 'F' || ( mask_el < V && mask_el >= '0' && mask_el <= '9' );
    }
    else if ( V == 'T' )
    {
        return mask_el == 'F';
    }
    return false;
}

template <char MaskEl, char V>
struct static_interrupt
{
    static const bool value
        = ( V >= '0' && V <= '9' ) ? 
          ( MaskEl == 'F' || ( MaskEl < V && MaskEl >= '0' && MaskEl <= '9' ) ) :
          ( ( V == 'T' ) ? MaskEl == 'F' : false );

};

// TODO: possible optimizations
// 1. interrupt in a template xxx<Interrupt> make it static const if Interrupt == false
// 2. static_mask<II, IB, IE, ...> setting interrupt in compile-time

template <bool Interrupt>
class mask9
    : public matrix9
{
public:

    bool interrupt;

    inline mask9(std::string const& de9im_mask)
        : interrupt(false)
    {
        BOOST_ASSERT(de9im_mask.size() == 9);
        ::memcpy(m_mask, de9im_mask.c_str(), 9);
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        handle_interrupt_dispatch<F1, F2, V>(boost::integral_constant<bool, Interrupt>());
        matrix9::set<F1, F2, V>();
    }

    inline bool check() const
    {
        if ( interrupt )
            return false;
        
        for ( std::size_t i = 0 ; i < 9 ; ++i )
        {
            if ( !check_element(m_mask[i], matrix9::data()[i]) )
                return false;
        }

        return true;
    }

private:
    template <field F1, field F2, char V>
    void handle_interrupt_dispatch(boost::integral_constant<bool, true>)
    {
        char m = m_mask[F1 * 3 + F2];
        if ( relate::interrupt<V>(m) )
            interrupt = true;
    }

    template <field F1, field F2, char V>
    void handle_interrupt_dispatch(boost::integral_constant<bool, false>)
    {}

    char m_mask[9];
};

//template <template StaticMask, field F1, field F2>
//class static_get{};
//
//template <char II, char IB, char IE,
//          char BI, char BB, char BE,
//          char EI, char EB, char EE,
//          bool Interrupt>
//class static_mask
//    : public result
//{
//public:
//
//    bool interrupt;
//
//    inline static_mask()
//        : interrupt(false)
//    {}
//
//    template <field F1, field F2, char V>
//    inline void set()
//    {
//        static const bool is_required = static_get<static_mask>::value == 'F'
//                                     || static_get<static_mask>::value == 'T'
//                                     || ( static_get<static_mask>::value >= 0
//                                       && static_get<static_mask>::value <= 9 );
//
//        handle_interrupt_dispatch<F1, F2, V>(boost::integral_constant<bool, Interrupt && is_required>());
//        set_dispatch<F1, F2, V>(boost::integral_constant<bool, is_required>());
//    }
//
//    inline bool check() const
//    {
//        if ( interrupt )
//            return false;
//
//        return check_one<interior, interior>()
//            && check_one<interior, boundary>()
//            && check_one<interior, exterior>()
//            && check_one<boundary, interior>()
//            && check_one<boundary, boundary>()
//            && check_one<boundary, exterior>()
//            && check_one<exterior, interior>()
//            && check_one<exterior, boundary>()
//            && check_one<exterior, exterior>();
//    }
//
//private:
//    template <field F1, field F2, char V>
//    void handle_interrupt_dispatch(boost::integral_constant<bool, false>)
//    {}
//
//    template <field F1, field F2, char V>
//    void handle_interrupt_dispatch(boost::integral_constant<bool, true>)
//    {
//        char m = get_mask<F1, F2>();
//
//        if ( V >= '0' && V <= '9' )
//        {
//            if ( m == 'F' || ( m < V && m >= '0' && m <= '9' ) )
//                interrupt = true;
//        }
//        else if ( V == 'T' )
//        {
//            if ( m == 'F' )
//                interrupt = true;
//        }
//    }
//
//    template <field F1, field F2, char V>
//    inline void set_dispatch(boost::integral_constant<bool, true>)
//    {
//        result::set<F1, F2, V>();
//    }
//
//    template <field F1, field F2, char V>
//    inline void set_dispatch(boost::integral_constant<bool, false>)
//    {}
//
//    template <field F1, field F2>
//    inline bool check_one() const
//    {
//        static const char m = static_get<static_mask, F1, F2>::value;
//        return check_dispatch<F1, F2>(boost::integral_constant<char, m>);
//    }
//
//    template <field F1, field F2>
//    inline bool check_dispatch(boost::integral_constant<char, 'F'>) const
//    {
//        return get<F1, F2>() == 'F';
//    }
//
//    template <field F1, field F2>
//    inline bool check_dispatch(boost::integral_constant<char, 'T'>) const
//    {
//        const char v = get<F1, F2>();
//        return v == 'T' || ( v >= '0' && v <= '9' );
//    }
//
//    template <field F1, field F2, char C>
//    inline bool check_dispatch(boost::integral_constant<char, C>) const
//    {
//        static const bool is_digit = C >= '0' && C <= '9';
//        return bool check_dispatch_digit<F1, F2>(boost::integral_constant<bool, is_digit>());
//    }
//
//    template <field F1, field F2>
//    inline bool check_dispatch_digit(boost::integral_constant<bool, true>) const
//    {
//        return get<F1, F2>() == static_get<static_mask, F1, F2>::value;
//    }
//
//    template <field F1, field F2>
//    inline bool check_dispatch_digit(boost::integral_constant<bool, false>) const
//    {
//        return true;
//    }
//};
//
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, interior, interior>
//{ static const char value = II };
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, interior, boundary>
//{ static const char value = IB };
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, interior, exterior>
//{ static const char value = IE };
//
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, boundary, interior>
//{ static const char value = BI };
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, boundary, boundary>
//{ static const char value = BB };
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, boundary, exterior>
//{ static const char value = BE };
//
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, exterior, interior>
//{ static const char value = EI };
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, exterior, boundary>
//{ static const char value = EB };
//template <char II, char IB, char IE, char BI, char BB, char BE, char EI, char EB, char EE, bool Interrupt>
//class static_get<static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE, Interrupt>, exterior, exterior>
//{ static const char value = EE };

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
