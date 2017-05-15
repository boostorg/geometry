// Boost.Geometry
// Unit Test

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <iostream>

#include <boost/geometry/srs/projection.hpp>

#include "projection_selftest_cases.hpp"


void test_projection(std::string const& id, std::string const& parameters,
                     const LL * fwd_in, const XY * fwd_expected,
                     const XY * inv_in, const LL * inv_expected)
{
    bg::srs::projection<> prj = bg::srs::proj4(parameters);

    for (std::size_t i = 0 ; i < 4 ; ++i)
    {
        if (bg::get<0>(fwd_expected[i]) == HUGE_VAL)
            break;

        {
            XY fwd_out;
            prj.forward(fwd_in[i], fwd_out);

            bool fwd_eq = bg::math::abs(bg::get<0>(fwd_out) - bg::get<0>(fwd_expected[i])) < 1e-7
                       && bg::math::abs(bg::get<1>(fwd_out) - bg::get<1>(fwd_expected[i])) < 1e-7;

            BOOST_CHECK_MESSAGE((fwd_eq),
                                std::setprecision(16) << "Result of " << id << " forward projection {"
                                << bg::wkt(fwd_out) << "} different than expected {"
                                << bg::wkt(fwd_expected[i]) << "}");
        }

        if (bg::get<0>(inv_expected[i]) == HUGE_VAL)
            break;

        {
            LL inv_out;        
            prj.inverse(inv_in[i], inv_out);

            bool inv_eq = bg::math::abs(bg::get<0>(inv_out) - bg::get<0>(inv_expected[i])) < 1e-10
                       && bg::math::abs(bg::get<1>(inv_out) - bg::get<1>(inv_expected[i])) < 1e-10;

            BOOST_CHECK_MESSAGE((inv_eq),
                                std::setprecision(16) << "Result of " << id << " forward projection {"
                                << bg::wkt(inv_out) << "} different than expected {"
                                << bg::wkt(inv_expected[i]) << "}");
        }
    }
}

void test_projections(const projection_case * cases, std::size_t n)
{
    for (std::size_t i = 0 ; i < n ; ++i)
    {
        projection_case const& pcas = cases[i];

        if (! pcas.e_args.empty())
        {
            test_projection(pcas.e_id, pcas.e_args,
                            pcas.fwd_in, pcas.e_fwd_expect,
                            pcas.inv_in, pcas.e_inv_expect);
        }

        if (! pcas.s_args.empty())
        {
            test_projection(pcas.s_id, pcas.s_args,
                            pcas.fwd_in, pcas.s_fwd_expect,
                            pcas.inv_in, pcas.s_inv_expect);
        }
    }
}

int test_main(int, char*[])
{  
    test_projections(projection_cases, sizeof(projection_cases)/sizeof(projection_case));

    return 0;
}
