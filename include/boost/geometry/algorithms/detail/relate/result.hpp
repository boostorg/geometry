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

// With DE9IM only Dimension < 10 is supported
class result
{
public:

    static const bool interrupt = false;

// TODO: replace with std::string?
    inline result()
    {
        ::memset(m_array, 'F', 9);
    }

    template <field F1, field F2>
    inline char get() const
    {
        return m_array[F1 * 3 + F2];
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        m_array[F1 * 3 + F2] = V;
    }

    inline std::pair<const char*, const char*> get_code() const
    {
        return std::make_pair(m_array, m_array+9);
    }

private:
    char m_array[9];
};

template <bool Interrupt>
class mask
    : public result
{
public:

// TODO: put interrupt in a template xxx<Interrupt>
// make it static const if Interrupt == false

    bool interrupt;

    inline mask(std::string const& de9im_mask)
        : interrupt(false)
    {
        BOOST_ASSERT(de9im_mask.size() == 9);
        ::memcpy(m_mask, de9im_mask.c_str(), 9);
    }

    template <field F1, field F2, char V>
    inline void set()
    {
        handle_interrupt_dispatch<F1, F2, V>(boost::integral_constant<bool, Interrupt>());

        result::set<F1, F2, V>();
    }

    inline bool check() const
    {
        if ( interrupt )
            return false;

        std::pair<const char*, const char*> range = result::get_code();
        const char* m_it = m_mask;
        const char* a_it = range.first;
        for ( ; a_it != range.second ; ++a_it, ++m_it )
        {
            if ( *m_it == 'F' )
            {
                if ( *a_it != 'F' )
                    return false;
            }
            else if ( *m_it == 'T' )
            {
                if ( *a_it != 'T' && ( *a_it < '0' || *a_it > '9' ) )
                    return false;
            }
            else if ( *m_it >= '0' && *m_it <= '9' )
            {
                if ( *a_it != *m_it )
                    return false;
            }
        }

        return true;
    }

private:
    template <field F1, field F2, char V>
    void handle_interrupt_dispatch(boost::integral_constant<bool, false>)
    {}

    template <field F1, field F2, char V>
    void handle_interrupt_dispatch(boost::integral_constant<bool, true>)
    {
        char m = get_mask<F1, F2>();

        if ( V >= '0' && V <= '9' )
        {
            if ( m == 'F' || ( m < V && m >= '0' && m <= '9' ) )
                interrupt = true;
        }
        else if ( V == 'T' )
        {
            if ( m == 'F' )
                interrupt = true;
        }
    }

    template <field F1, field F2>
    inline char get_mask() const
    {
        return m_mask[F1 * 3 + F2];
    }

    char m_mask[9];
};

template <field F1, field F2, char V, typename Result>
inline void set(Result & res)
{
    res.template set<F1, F2, V>();
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
    BOOST_STATIC_ASSERT('0' <= D && D <= '9');
    char c = res.template get<F1, F2>();
    if ( D > c || c > '9')
        res.template set<F1, F2, D>();
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
