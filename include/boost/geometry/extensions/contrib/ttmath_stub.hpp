#ifndef TTMATH_STUB
#define TTMATH_STUB

#include <boost/math/constants/constants.hpp>


#include <ttmath/ttmath.h>
namespace ttmath
{
    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> sqrt(Big<Mantissa, Exponent> const& v)
    {
        return Sqrt(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> abs(Big<Mantissa, Exponent> const& v)
    {
        return Abs(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> ceil(Big<Mantissa, Exponent> const& v)
    {
        return Ceil(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> floor(Big<Mantissa, Exponent> const& v)
    {
        return Floor(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> asin(Big<Mantissa, Exponent> const& v)
    {
        return ASin(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> sin(Big<Mantissa, Exponent> const& v)
    {
        return Sin(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> cos(Big<Mantissa, Exponent> const& v)
    {
        return Cos(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> tan(Big<Mantissa, Exponent> const& v)
    {
        return Tan(v);
    }

    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> atan(Big<Mantissa, Exponent> const& v)
    {
        return ATan(v);
    }


    template <uint Mantissa, uint Exponent>
    inline Big<Mantissa, Exponent> atan2(Big<Mantissa, Exponent> const& y, Big<Mantissa, Exponent> const& x)
    {
        // return ATan2(y, 2); does not (yet) exist in ttmath...

        // See http://en.wikipedia.org/wiki/Atan2

        Big<Mantissa, Exponent> const zero(0);
        Big<Mantissa, Exponent> const two(2);

        if (y == zero)
        {
            // return x >= 0 ? 0 : pi and pi=2*arccos(0)
            return x >= zero ? zero : two * ACos(zero);
        }

        return two * ATan((sqrt(x * x + y * y) - x) / y);
    }

}

// Specific structure implementing constructor
// (WHICH IS NECESSARY FOR Boost.Geometry because it enables using T() !! )
struct ttmath_big : ttmath::Big<1,4>
{
    ttmath_big(double v = 0)
        : ttmath::Big<1,4>(v)
    {}
    ttmath_big(ttmath::Big<1,4> const& v)
        : ttmath::Big<1,4>(v)
    {}

    /*
    inline operator double() const
    {
        return atof(this->ToString().c_str());
    }

    inline operator int() const
    {
        return atol(ttmath::Round(*this).ToString().c_str());
    }
    */
};

namespace boost{ namespace math { namespace constants
{
    // Workaround for boost::math::constants::pi: 
    // 1) lexical cast -> stack overflow and
    // 2) because it is implemented as a function, generic implementation not possible

    template <ttmath::uint Mantissa, ttmath::uint Exponent>
    inline ttmath::Big<Mantissa, Exponent> ttmath_pi()
    {
        static ttmath::Big<Mantissa, Exponent> const the_pi("3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493038196");
        return the_pi;
    }


    template <> inline ttmath::Big<1,4> pi<ttmath::Big<1,4> >(BOOST_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC( (ttmath::Big<1,4>) ))
    {
        return ttmath_pi<1,4>();
    }
    template <> inline ttmath::Big<1,8> pi<ttmath::Big<1,8> >(BOOST_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC( (ttmath::Big<1,8>) ))
    {
        return ttmath_pi<1,8>();
    }

    // and so on...
    // or fix lexical_cast but probably has the same problem


    template <> inline ttmath_big pi<ttmath_big >(BOOST_MATH_EXPLICIT_TEMPLATE_TYPE_SPEC(ttmath_big))
    {
        return ttmath_pi<1,4>();
    }

}}}


#endif
