// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_TEST_INTERSECTION1_HPP
#define BOOST_GEOMETRY_TEST_INTERSECTION1_HPP

#include "from_wkt.hpp"
#include <boost/geometry/algorithms/reverse.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/multi/algorithms/reverse.hpp>

#include <string>
#include <sstream>
#include <algorithm>
#include <boost/geometry/geometry.hpp>


namespace bg = ::boost::geometry;

#include "to_svg.hpp"


template <typename LS1, typename LS2>
struct ls_less
{
    typedef typename boost::range_iterator<LS1 const>::type It1;
    typedef typename boost::range_iterator<LS2 const>::type It2;

    typedef bg::less<typename bg::point_type<LS1>::type> PointLess;

    bool operator()(LS1 const& ls1, LS2 const& ls2) const
    {
        if ( boost::size(ls1) != boost::size(ls2) )
            return boost::size(ls1) < boost::size(ls2);

        It1 it1 = boost::begin(ls1);
        It2 it2 = boost::begin(ls2);
        PointLess less;
        for (; it1 != boost::end(ls1); ++it1, ++it2)
        {
            if ( less(*it1, *it2) )
                return true;
            if ( less(*it2, *it1) )
                return false;
        }
        return false;
    }
};


template <typename MultiLinestring1, typename MultiLinestring2>
struct multilinestring_equals
{
    static inline
    bool apply(MultiLinestring1 const& mls1_, MultiLinestring2 const& mls2_)
    {
        MultiLinestring1 mls1 = mls1_;
        MultiLinestring2 mls2 = mls2_;
        BOOST_CHECK(boost::size(mls1) == boost::size(mls2));

        typedef typename boost::range_iterator
            <
                MultiLinestring1 const
            >::type ls1_iterator;

        typedef typename boost::range_iterator
            <
                MultiLinestring2 const
            >::type ls2_iterator;

        typedef typename boost::range_value<MultiLinestring1>::type Linestring1;

        typedef typename boost::range_value<MultiLinestring2>::type Linestring2;

        typedef typename boost::range_iterator
            <
                Linestring1 const
            >::type pt1_iterator;

        typedef typename boost::range_iterator
            <
                Linestring2 const
            >::type pt2_iterator;

        typedef ls_less<Linestring1, Linestring2> LS_Less;

        std::sort(boost::begin(mls1), boost::end(mls1), LS_Less());
        std::sort(boost::begin(mls2), boost::end(mls2), LS_Less());

        if ( boost::size(mls1) == boost::size(mls2) )
        {
            ls1_iterator it1 = boost::begin(mls1);
            ls2_iterator it2 = boost::begin(mls2);
            for (; it1 != boost::end(mls1); ++it1, ++it2)
            {
                BOOST_CHECK( boost::size(*it1) == boost::size(*it2) );
                if ( boost::size(*it1) == boost::size(*it2) )
                {
                    pt1_iterator pit1 = boost::begin(*it1);
                    pt2_iterator pit2 = boost::begin(*it2);
                    for (; pit1 != boost::end(*it1); ++pit1, ++pit2)
                    {
                        BOOST_CHECK( bg::equals(*pit1, *pit2) );
                    }
                }
            }
        }
        return true;
    }
};

struct equals
{
    template <typename MLS1, typename MLS2>
    bool operator()(MLS1 const& mls1, MLS2 const& mls2) const
    {
#if 0
        if ( multilinestring_equals<MLS1, MLS2>::apply(mls1, mls2) )
        {
            return true;
        }

        MLS1 rmls1 = mls1;
        bg::reverse(rmls1);
        if ( multilinestring_equals<MLS1, MLS2>::apply(rmls1, mls2) )
        {
            return true;
        }

        MLS2 rmls2 = mls2;
        bg::reverse(rmls2);
        if ( multilinestring_equals<MLS1, MLS2>::apply(mls1, rmls2) )
        {
            return true;
        }
#endif
        return multilinestring_equals<MLS1, MLS2>::apply(mls1, mls2);
    }
};


//==================================================================
//==================================================================
// intersection of (linear) geometries
//==================================================================
//==================================================================

template
<
    typename Geometry1, typename Geometry2,
    typename MultiLineString
>
struct test_intersection_of_geometries
{
    void base_test(Geometry1 const& geometry1,
                   Geometry2 const& geometry2,
                   MultiLineString const& mls_int,
                   bool test_vector_and_deque = true,
                   bool reverse_output_for_checking = false) const
    {
        typedef typename boost::range_value<MultiLineString>::type LineString;
        typedef std::vector<LineString> LineStringVector;
        typedef std::deque<LineString> LineStringDeque;

        MultiLineString mls_output;

        LineStringVector ls_vector_output;
        LineStringDeque ls_deque_output;

        bg::intersection(geometry1, geometry2, mls_output);

        if ( reverse_output_for_checking ) 
        {
            bg::reverse(mls_output);
        }

        BOOST_CHECK( equals()(mls_int, mls_output) );

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "Geometry #1: " << bg::wkt(geometry1) << std::endl;
        std::cout << "Geometry #2: " << bg::wkt(geometry2) << std::endl;
        std::cout << "intersection : " << bg::wkt(mls_output) << std::endl;
        std::cout << "expected intersection : " << bg::wkt(mls_int)
                  << std::endl;
        std::cout << std::endl;
        std::cout << "************************************" << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
#endif

        if ( test_vector_and_deque )
        {
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            std::cout << "Testing with vector and deque as output container..."
                      << std::endl;
#endif
            bg::intersection(geometry1, geometry2, ls_vector_output);
            bg::intersection(geometry1, geometry2, ls_deque_output);

            BOOST_CHECK((multilinestring_equals
                         <
                             MultiLineString, LineStringVector
                         >::apply(mls_int, ls_vector_output)
                         ));

            BOOST_CHECK((multilinestring_equals
                         <
                             MultiLineString, LineStringDeque
                         >::apply(mls_int, ls_deque_output)
                         ));
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "Done!" << std::endl << std::endl;
#endif
        }

        // check the intersection where the order of the two
        // geometries is reversed
        bg::clear(mls_output);
        bg::intersection(geometry2, geometry1, mls_output);

        if ( reverse_output_for_checking ) 
        {
            bg::reverse(mls_output);
        }

        BOOST_CHECK( equals()(mls_int, mls_output) );

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "Geometry #1: " << bg::wkt(geometry2) << std::endl;
        std::cout << "Geometry #2: " << bg::wkt(geometry1) << std::endl;
        std::cout << "intersection : " << bg::wkt(mls_output) << std::endl;
        std::cout << "expected intersection : " << bg::wkt(mls_int)
                  << std::endl;
        std::cout << std::endl;
        std::cout << "************************************" << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
#endif
    }



    void operator()(Geometry1 const& geometry1,
                    Geometry2 const& geometry2,
                    MultiLineString const& mls_int) const
    {
        Geometry1 rg1(geometry1);
        bg::reverse<Geometry1>(rg1);

        Geometry2 rg2(geometry2);
        bg::reverse<Geometry2>(rg2);

        base_test(geometry1, geometry2, mls_int);
        //        base_test(geometry1, rg2, mls_diff, false);
        //        base_test(rg1, geometry2, mls_diff, false, true);
        //        base_test(rg1, rg2, mls_diff, false, true);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << std::endl;
#endif
    }


    void operator()(Geometry1 const& geometry1,
                    Geometry2 const& geometry2,
                    MultiLineString const& mls_int,
                    std::string const& test_case_str) const
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "test case: " << test_case_str << std::endl;
        std::stringstream sstr;
        sstr << "svgs/" << test_case_str << ".svg";
        to_svg(geometry1, geometry2, sstr.str());
#endif
        this->operator()(geometry1, geometry2, mls_int);
    }
};


#endif // BOOST_GEOMETRY_TEST_INTERSECTION1_HPP
