#ifndef BOOST_GEOMETRY_TEST_DIFFERENCE1_HPP
#define BOOST_GEOMETRY_TEST_DIFFERENCE1_HPP

#include "from_wkt.hpp"
#include <boost/geometry/algorithms/reverse.hpp>
#include <boost/geometry/multi/algorithms/reverse.hpp>

//#include "to_svg.hpp"
#include <string>
#include <boost/geometry/geometry.hpp>


namespace bg = ::boost::geometry;

template<typename MultiLinestring1, typename MultiLinestring2>
struct multilinestring_equals
{
    static inline
    bool apply(MultiLinestring1 const& mls1, MultiLinestring2 const& mls2)
    {
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


//==================================================================
//==================================================================
// difference of linestrings
//==================================================================
//==================================================================

struct test_difference_of_linestrings
{
    template
        <
            typename Linestring1, typename Linestring2,
            typename MultiLinestring
        >
    void base_test(Linestring1 const& linestring1,
                   Linestring2 const& linestring2,
                   MultiLinestring const& mls_diff) const
    {
        typedef typename boost::range_value<MultiLinestring>::type Linestring;
        typedef std::vector<Linestring> LinestringVector;
        typedef std::deque<Linestring> LinestringDeque;

        MultiLinestring mls_output;

        LinestringVector ls_vector_output;
        LinestringDeque ls_deque_output;

        bg::difference(linestring1, linestring2, mls_output);

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLinestring, MultiLinestring
                     >::apply(mls_diff, mls_output)
                     ));

#if 0
        bg::difference(linestring1, linestring2, ls_vector_output);
        bg::difference(linestring1, linestring2, ls_deque_output);

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLinestring, LinestringVector
                     >::apply(mls_diff, ls_vector_output)
                     ));

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLinestring, LinestringDeque
                     >::apply(mls_diff, ls_deque_output)
                     ));
#endif
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "Linestring #1: " << bg::wkt(linestring1) << std::endl;
        std::cout << "Linestring #2: " << bg::wkt(linestring2) << std::endl;
        std::cout << "difference : " << bg::wkt(mls_output)
                  << std::endl;
        std::cout << std::endl;
#endif
    }

    template
        <
            typename Linestring1, typename Linestring2,
            typename MultiLinestringOut
        >
    void operator()(Linestring1 const& linestring1,
                    Linestring2 const& linestring2,
                    MultiLinestringOut const& mls_diff) const
    {
        Linestring1 rls1(linestring1);
        bg::reverse<Linestring1>(rls1);

        Linestring2 rls2(linestring2);
        bg::reverse<Linestring2>(rls2);

        base_test(linestring1, linestring2, mls_diff);
        //        base_test(linestring1, rls2, mls_diff);
        //        base_test(rls1, linestring2, mls_diff);
        //        base_test(rls1, rls2, mls_diff);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << std::endl;
#endif
    }

    template
        <
            typename Linestring1, typename Linestring2,
            typename MultiLinestringOut
        >
    void operator()(Linestring1 const& linestring1,
                    Linestring2 const& linestring2,
                    MultiLinestringOut const& mls_diff,
                    std::string const& svg_fname) const
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "SVG file: " << svg_fname << std::endl;
        //        to_svg(linestring1, linestring2, svg_fname);
#endif
        this->operator()(linestring1, linestring2, mls_diff);
    }
};


//==================================================================
//==================================================================
// difference of multilinestrings
//==================================================================
//==================================================================

struct test_difference_of_multilinestrings
{
    template
        <
            typename MultiLinestring1, typename MultiLinestring2,
            typename MultiLinestring
        >
    void base_test(MultiLinestring1 const& multilinestring1,
                   MultiLinestring2 const& multilinestring2,
                   MultiLinestring const& mls_diff) const
    {
        typedef typename boost::range_value<MultiLinestring>::type Linestring;
        typedef std::vector<Linestring> LinestringVector;
        typedef std::deque<Linestring> LinestringDeque;

        MultiLinestring mls_output;

        LinestringVector ls_vector_output;
        LinestringDeque ls_deque_output;

        bg::difference(multilinestring1, multilinestring2, mls_output);

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLinestring, MultiLinestring
                     >::apply(mls_diff, mls_output)
                     ));

#if 0
        bg::difference(linestring1, linestring2, ls_vector_output);
        bg::difference(linestring1, linestring2, ls_deque_output);

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLinestring, LinestringVector
                     >::apply(mls_diff, ls_vector_output)
                     ));

        BOOST_CHECK((multilinestring_equals
                     <
                         MultiLinestring, LinestringDeque
                     >::apply(mls_diff, ls_deque_output)
                     ));
#endif
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "MultiLinestring #1: "
                  << bg::wkt(multilinestring1) << std::endl;
        std::cout << "MultiLinestring #2: "
                  << bg::wkt(multilinestring2) << std::endl;
        std::cout << "difference : " << bg::wkt(mls_output)
                  << std::endl;
        std::cout << std::endl;
#endif
    }

    template
        <
            typename MultiLinestring1, typename MultiLinestring2,
            typename MultiLinestringOut
        >
    void operator()(MultiLinestring1 const& multilinestring1,
                    MultiLinestring2 const& multilinestring2,
                    MultiLinestringOut const& mls_diff) const
    {
        MultiLinestring1 rls1(multilinestring1);
        bg::reverse<MultiLinestring1>(rls1);

        MultiLinestring2 rls2(multilinestring2);
        bg::reverse<MultiLinestring2>(rls2);

        base_test(multilinestring1, multilinestring2, mls_diff);
        //        base_test(linestring1, rls2, mls_diff);
        //        base_test(rls1, linestring2, mls_diff);
        //        base_test(rls1, rls2, mls_diff);

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << std::endl;
#endif
    }

    template
        <
            typename MultiLinestring1, typename MultiLinestring2,
            typename MultiLinestringOut
        >
    void operator()(MultiLinestring1 const& multilinestring1,
                    MultiLinestring2 const& multilinestring2,
                    MultiLinestringOut const& mls_diff,
                    std::string const& svg_fname) const
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "SVG file: " << svg_fname << std::endl;
        //        to_svg(multilinestring1, multilinestring2, svg_fname);
#endif
        this->operator()(multilinestring1, multilinestring2, mls_diff);
    }
};



#endif // BOOST_GEOMETRY_TEST_DIFFERENCE1_HPP
