// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_TEST_DIFFERENCE1_HPP
#define BOOST_GEOMETRY_TEST_DIFFERENCE1_HPP

#include "from_wkt.hpp"

#include <string>
#include <sstream>
#include <algorithm>
#include <boost/geometry/geometry.hpp>

#include "test_set_ops_common.hpp"

#include "to_svg.hpp"


//==================================================================
//==================================================================
// difference of (linear) geometries
//==================================================================
//==================================================================

template
<
    typename Geometry1, typename Geometry2,
    typename MultiLineString
>
class test_difference_of_geometries
{
private:
    static inline void base_test(Geometry1 const& geometry1,
                                 Geometry2 const& geometry2,
                                 MultiLineString const& mls_diff,
                                 bool test_vector_and_deque = true,
                                 bool reverse_output_for_checking = false)
    {
        static bool vector_deque_already_tested = false;

        typedef typename boost::range_value<MultiLineString>::type LineString;
        typedef std::vector<LineString> linestring_vector;
        typedef std::deque<LineString> linestring_deque;

        MultiLineString mls_output;

        linestring_vector ls_vector_output;
        linestring_deque ls_deque_output;

        bg::difference(geometry1, geometry2, mls_output);

        if ( reverse_output_for_checking ) 
        {
            bg::reverse(mls_output);
        }

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLineString, MultiLineString
                     >::apply(mls_diff, mls_output)
                     ));

        if ( !vector_deque_already_tested && test_vector_and_deque )
        {
            vector_deque_already_tested = true;
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            std::cout << "Testing with vector and deque as output container..."
                      << std::endl;
#endif
            bg::difference(geometry1, geometry2, ls_vector_output);
            bg::difference(geometry1, geometry2, ls_deque_output);

            BOOST_CHECK((multilinestring_equals
                         <
                             MultiLineString, linestring_vector
                         >::apply(mls_diff, ls_vector_output)
                         ));

            BOOST_CHECK((multilinestring_equals
                         <
                             MultiLineString, linestring_deque
                         >::apply(mls_diff, ls_deque_output)
                         ));
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "Done!" << std::endl << std::endl;
#endif
        }

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "Geometry #1: " << bg::wkt(geometry1) << std::endl;
        std::cout << "Geometry #2: " << bg::wkt(geometry2) << std::endl;
        std::cout << "difference : " << bg::wkt(mls_output) << std::endl;
        std::cout << std::endl;
        std::cout << "************************************" << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
#endif
    }


public:
    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             MultiLineString const& mls_diff)
    {
        Geometry1 rg1(geometry1);
        bg::reverse<Geometry1>(rg1);

        Geometry2 rg2(geometry2);
        bg::reverse<Geometry2>(rg2);

        base_test(geometry1, geometry2, mls_diff);
        base_test(geometry1, rg2, mls_diff, false);
        base_test(rg1, geometry2, mls_diff, false, true);
        base_test(rg1, rg2, mls_diff, false, true);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << std::endl;
#endif
    }


    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             MultiLineString const& mls_diff,
                             std::string const& test_case_str)
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "test case: " << test_case_str << std::endl;
        std::stringstream sstr;
        sstr << "svgs/" << test_case_str << ".svg";
        to_svg(geometry1, geometry2, sstr.str());
#endif
        apply(geometry1, geometry2, mls_diff);
    }
};


#endif // BOOST_GEOMETRY_TEST_DIFFERENCE1_HPP
