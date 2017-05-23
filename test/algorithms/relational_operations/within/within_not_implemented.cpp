// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2013-2015 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2014, 2015.
// Modifications copyright (c) 2014-2015 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "test_within.hpp"

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


bool true_pred(bg::not_implemented_exception const&) { return true; }


int test_main( int , char* [] )
{
    typedef bg::model::d2::point_xy<int> pt_t;
    typedef bg::model::multi_point<pt_t> mpt_t;
    typedef bg::model::linestring<pt_t> ls_t;
    typedef bg::model::ring<pt_t> ring_t;
    typedef bg::model::polygon<pt_t> poly_t;
    typedef bg::model::multi_polygon<poly_t> mpoly_t;

    typedef boost::variant<pt_t> var_pt_t;
    typedef boost::variant<poly_t> var_po_t;
    typedef boost::variant<pt_t, poly_t> var_pt_po_t;
    typedef boost::variant<poly_t, pt_t> var_po_pt_t;

    pt_t pt(1, 1);
    poly_t poly;
    boost::geometry::read_wkt("POLYGON((0 0,0 5,5 5,5 0,0 0))", poly);

    var_pt_po_t v1 = pt;
    var_pt_po_t v2 = pt;
    var_pt_po_t v3 = poly;

    BOOST_CHECK(bg::within(v1, v2));
    BOOST_CHECK(bg::within(v1, v3));
    BOOST_CHECK_EXCEPTION(bg::within(v3, v1),
                          bg::not_implemented_exception,
                          true_pred);

    using bg::resolve_variant::within_metapolicy;

    // sanity check
    BOOST_CHECK( (within_metapolicy::apply<poly_t, pt_t>::value));
    BOOST_CHECK(!(within_metapolicy::apply<poly_t, poly_t>::value));
    BOOST_CHECK(!(within_metapolicy::apply<pt_t, poly_t>::value));
    BOOST_CHECK(!(within_metapolicy::apply<pt_t, pt_t>::value));

    BOOST_CHECK( (bg::is_not_implemented<poly_t, pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<poly_t, poly_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<pt_t, poly_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<pt_t, pt_t, within_metapolicy>::value));


    BOOST_CHECK( (bg::is_not_implemented<poly_t, var_pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<poly_t, var_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<poly_t, var_pt_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<poly_t, var_po_pt_t, within_metapolicy>::value));

    BOOST_CHECK(!(bg::is_not_implemented<pt_t, var_pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<pt_t, var_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<pt_t, var_pt_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<pt_t, var_po_pt_t, within_metapolicy>::value));


    BOOST_CHECK(!(bg::is_not_implemented<var_pt_t, pt_t, within_metapolicy>::value));
    BOOST_CHECK( (bg::is_not_implemented<var_po_t, pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_po_t, pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_po_t, pt_t, within_metapolicy>::value));

    BOOST_CHECK(!(bg::is_not_implemented<var_pt_t, poly_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_po_t, poly_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_po_t, poly_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_po_t, poly_t, within_metapolicy>::value));


    BOOST_CHECK(!(bg::is_not_implemented<var_pt_t, var_pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_t, var_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_t, var_pt_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_t, var_po_pt_t, within_metapolicy>::value));

    BOOST_CHECK( (bg::is_not_implemented<var_po_t, var_pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_po_t, var_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_po_t, var_pt_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_po_t, var_po_pt_t, within_metapolicy>::value));

    BOOST_CHECK(!(bg::is_not_implemented<var_pt_po_t, var_pt_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_pt_po_t, var_po_pt_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_po_pt_t, var_pt_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_po_pt_t, var_po_pt_t, within_metapolicy>::value));


    typedef boost::variant<pt_t, mpt_t, ls_t> var_p_ls_t;
    typedef boost::variant<ring_t, poly_t, mpoly_t> var_a_t;
    typedef boost::variant<pt_t, mpt_t, ls_t, poly_t> var_p_ls_po_t;
    typedef boost::variant<ls_t, ring_t, poly_t, mpoly_t> var_ls_a_t;

    // sanity check
    BOOST_CHECK( (within_metapolicy::apply<ring_t, pt_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<ring_t, mpt_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<ring_t, ls_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<poly_t, pt_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<poly_t, mpt_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<poly_t, ls_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<mpoly_t, pt_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<mpoly_t, mpt_t>::value));
    BOOST_CHECK( (within_metapolicy::apply<mpoly_t, ls_t>::value));

    BOOST_CHECK(!(bg::is_not_implemented<var_p_ls_t, var_a_t, within_metapolicy>::value));
    BOOST_CHECK( (bg::is_not_implemented<var_a_t, var_p_ls_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_a_t, var_p_ls_po_t, within_metapolicy>::value));
    BOOST_CHECK(!(bg::is_not_implemented<var_ls_a_t, var_p_ls_po_t, within_metapolicy>::value));

    return 0;
}
