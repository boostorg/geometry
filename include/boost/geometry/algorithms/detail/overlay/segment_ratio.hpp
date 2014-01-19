// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SEGMENT_RATIO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SEGMENT_RATIO_HPP


#include <boost/config.hpp>
#include <boost/rational.hpp>

#include <boost/geometry/util/math.hpp>

namespace boost { namespace geometry
{

//! Small class to keep a ratio (e.g. 1/4)
//! Main purpose is intersections and checking on 0, 1, and smaller/larger
//! The prototype used Boost.Rational. However, we also want to store FP ratios,
//! (so numerator/denominator both in float)
//! and Boost.Rational starts with GCD which we prefer to avoid if not necessary
//! On a segment means: this ratio is between 0 and 1 (both inclusive)
//!
template <typename Type>
class segment_ratio
{
public :
    inline segment_ratio()
        : m_numerator(0)
        , m_denominator(1)
    {}

    inline segment_ratio(const Type& nominator, const Type& denominator)
        : m_numerator(nominator)
        , m_denominator(denominator)
    {
        initialize();
    }

    inline Type numerator() const { return m_numerator; }
    inline Type denominator() const { return m_denominator; }

    inline void assign(const Type& nominator, const Type& denominator)
    {
        m_numerator = nominator;
        m_denominator = denominator;
        initialize();
    }

    inline void initialize()
    {
        // Minimal normalization
        // 1/-4 => -1/4, -1/-4 => 1/4
        if (m_denominator < 0)
        {
            m_numerator = -m_numerator;
            m_denominator = -m_denominator;
        }

        static const double scale = 1000000.0;
        m_approximation =
            m_denominator == 0 ? 0
            : int(double(m_numerator) * scale / double(m_denominator));
    }

    inline bool is_zero() const { return math::equals(m_numerator, 0); }
    inline bool is_one() const { return math::equals(m_numerator, m_denominator); }
    inline bool on_segment() const
    {
        // e.g. 0/4 or 4/4 or 2/4
        return m_numerator >= 0 && m_numerator <= m_denominator;
    }
    inline bool in_segment() const
    {
        // e.g. 1/4
        return m_numerator > 0 && m_numerator < m_denominator;
    }
    inline bool on_end() const
    {
        // e.g. 0/4 or 4/4
        return is_zero() || is_one();
    }
    inline bool left() const
    {
        // e.g. -1/4
        return m_numerator < 0;
    }
    inline bool right() const
    {
        // e.g. 5/4
        return m_numerator > m_denominator;
    }

    inline bool close_to(segment_ratio<Type> const& other) const
    {
        return geometry::math::abs(m_approximation - other.m_approximation) < 2;
    }

    inline bool operator< (segment_ratio<Type> const& other) const
    {
        return close_to(other)
            ? boost::rational<Type>(m_numerator, m_denominator)
                < boost::rational<Type>(other.m_numerator, other.m_denominator)
            : m_approximation < other.m_approximation;
    }

    inline bool operator== (segment_ratio<Type> const& other) const
    {
        return close_to(other)
            && (boost::rational<Type>(m_numerator, m_denominator)
                == boost::rational<Type>(other.m_numerator, other.m_denominator));
    }

    static inline segment_ratio<Type> zero()
    {
        static segment_ratio<Type> result(0, 1);
        return result;
    }

    static inline segment_ratio<Type> one()
    {
        static segment_ratio<Type> result(1, 1);
        return result;
    }

#if defined(BOOST_GEOMETRY_DEFINE_STREAM_OPERATOR_SEGMENT_RATIO)
    friend std::ostream& operator<<(std::ostream &os, segment_ratio const& ratio)
    {
        os << ratio.m_numerator << "/" << ratio.m_denominator;
        return os;
    }
#endif



private :
    Type m_numerator;
    Type m_denominator;

    // Contains ratio on scale 0..1000000 (for 0..1)
    // This is an approximation for fast and rough comparisons
    // Boost.Rational is used if the approximations are close.
    // Reason: performance, Boost.Rational does a GCD by default and also the
    // comparisons contain while-loops.
    int m_approximation;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SEGMENT_RATIO_HPP
