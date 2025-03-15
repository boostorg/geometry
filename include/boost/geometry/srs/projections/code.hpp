// Boost.Geometry

// Copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_PROJECTIONS_CODE_HPP
#define BOOST_GEOMETRY_PROJECTIONS_CODE_HPP

#include <cstdint>

#include <boost/geometry/srs/projections/dpar.hpp>

namespace boost { namespace geometry { namespace projections
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

    template <std::size_t MaxParams, bool HasTowgs84 = true>
    struct code_element
    {
        union parameter_variant
        {
            bool b;
            int i;
            double v;
            //spheroid
            //nadgrids,
            //towgs84
        };

        int code;
        std::int8_t parameter_count;
        std::int8_t towgs84_size {0};
        // Not bundling the following together into a struct to avoid 8-byte-alignment.
        std::int8_t parameter_variant_tags[MaxParams];
        std::int16_t parameter_ids[MaxParams];
        parameter_variant parameters[MaxParams];
        double towgs84_data[7];
    };

    template <std::size_t MaxParams>
    struct code_element<MaxParams, false>
    {
        union parameter_variant
        {
            bool b;
            int i;
            double v;
        };

        int code;
        std::int8_t parameter_count;
        std::int8_t parameter_variant_tags[MaxParams];
        std::int16_t parameter_ids[MaxParams];
        parameter_variant parameters[MaxParams];
    };

    template <bool HasTowgs84 = true>
    struct code_element_to_towgs84
    {
        template <std::size_t MaxParams>
        static void apply(code_element<MaxParams, true> const& e,
                          srs::dpar::parameters<double>& out)
        {
            using p = srs::dpar::parameter<double>;
            out(p(srs::dpar::name_towgs84::towgs84,
                  srs::detail::towgs84<double>(e.towgs84_data, e.towgs84_data + e.towgs84_size)));
        }
    };

    template <>
    struct code_element_to_towgs84<false>
    {
        template <std::size_t MaxParams>
        static void apply(code_element<MaxParams, false> const&, srs::dpar::parameters<double>&) {}
    };


    template <std::size_t MaxParams, bool HasTowgs84>
    inline srs::dpar::parameters<>
    code_element_to_dpar_parameters(code_element<MaxParams, HasTowgs84> const& e)
    {
        using namespace srs::dpar;
        using p = srs::dpar::parameter<double>;
        srs::dpar::parameters<> out;
        out.reserve(e.parameter_count);
        for (int i = 0; i < e.parameter_count; ++i)
        {
            auto const& id = e.parameter_ids[i];
            auto const& pv = e.parameters[i];
            auto const& tag = e.parameter_variant_tags[i];
            if (id == -1)
            {
                out(p());
            }
            else if (id < static_cast<int>(name_r::alpha)) // f
            {
                out(p(static_cast<name_f>(id), pv.v));
            }
            else if (id < static_cast<int>(name_i::aperture)) // r
            {
                out(p(static_cast<name_r>(id), pv.v));
            }
            else if (id < static_cast<int>(name_be::czech)) // i
            {
                out(p(static_cast<name_i>(id), pv.i));
            }
            else if (id < static_cast<int>(name_datum::datum)) // be
            {
                out(p(static_cast<name_be>(id), pv.b));
            }
            else if (id == static_cast<int>(name_datum::datum)) //datum
            {
                out(p(name_datum::datum, static_cast<value_datum>(pv.i)));
            }
            else if ( id == static_cast<int>(name_ellps::ellps)) //ellps
            {
                out(p(name_ellps::ellps, static_cast<value_ellps>(pv.i)));
            }
            // no occurence of ellps with spheroid in arr
            // no occurence of mode in arr
            // no occurence of nadgrids in arr
            // no occurence of orient in arr
            else if (id == static_cast<name_pm>(name_pm::pm)) //pm
            {
                if (tag == 1)
                {
                    out(p(name_pm::pm, static_cast<value_pm>(pv.i)));
                }
                else //if tag == 2
                {
                    out(p(name_pm::pm, pv.v));
                }
            }
            else if (id < static_cast<int>(name_sweep::sweep)) // proj
            {
                out(p(static_cast<name_proj>(id), static_cast<value_proj>(pv.i)));
            }
            else if (id == static_cast<int>(name_sweep::sweep)) // sweep
            {
                out(p(name_sweep::sweep, static_cast<value_sweep>(pv.i)));
            }
            else if (id == static_cast<int>(name_towgs84::towgs84)) //towgs84
            {
                code_element_to_towgs84<HasTowgs84>::apply(e, out);
            }
            // no occurence of axis in arr
            else if (id == static_cast<int>(name_units::units)) //units
            {
                out(p(name_units::units, static_cast<value_units>(pv.i)));
            }
            // no occurence of vunits in arr
        }
        return out;
    }

}
#endif // DOXYGEN_NO_DETAIL


}}} // namespace boost::geometry::projections

#endif
