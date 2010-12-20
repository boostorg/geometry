// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/type_traits/is_same.hpp>

#ifdef HAVE_TTMATH
#  include <boost/geometry/extensions/contrib/ttmath_stub.hpp>
#endif

#include <geometry_test_common.hpp>


//#define BOOST_GEOMETRY_DEBUG_ENRICH
//#define BOOST_GEOMETRY_DEBUG_RELATIVE_ORDER

#define BOOST_GEOMETRY_REPORT_OVERLAY_ERROR
#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER


#define BOOST_GEOMETRY_TEST_OVERLAY_NOT_EXCHANGED



#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>

#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>


#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/correct.hpp>


#include <boost/geometry/strategies/strategies.hpp>

#include <algorithms/overlay/overlay_common.hpp>
#include <algorithms/overlay/overlay_cases.hpp>

#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif

template 
<
    bg::detail::overlay::operation_type Direction, 
    bool Reverse1 = false, 
    bool Reverse2 = false
>
struct test_traverse
{
    static inline std::string operation(int d)
    {
        return d == 1 ? "union" : "intersection";
    }

    template <typename G1, typename G2>
    static void apply(std::string const& id,
            boost::tuple<int, double> const& expected_count_area,
            G1 const& g1, G2 const& g2,
            double precision)
    {
        // DEBUG ONE or FEW CASE(S) ONLY
        //if (! boost::contains(id, "36") || Direction != 1) return;
        //if (! boost::contains(id, "iet_") || boost::contains(id, "st")) return;
        //if (! boost::contains(id, "66") || Direction != 1) return;
        //if (! boost::contains(id, "92") && ! boost::contains(id, "96") ) return;
        //if (! (boost::contains(id, "58_st") || boost::contains(id, "59_st") || boost::contains(id, "60_st")  || boost::contains(id, "83"))  ) return;
        //if (! (boost::contains(id, "81") || boost::contains(id, "82") || boost::contains(id, "84") || boost::contains(id, "85")  || boost::contains(id, "68"))  ) return;
        //if (! (boost::contains(id, "81") || boost::contains(id, "86") || boost::contains(id, "88"))  ) return;
        //if (! boost::contains(id, "58_") || Direction != 1) return;
        //if (! boost::contains(id, "55") || Direction != 1) return;
        //if (! boost::contains(id, "55_iet_iet") || Direction != 1) return;
        //if (! boost::contains(id, "55_st_iet") || Direction != 1) return;
        //if (! boost::contains(id, "55_iet_st") || Direction != 1) return;
        //if (! boost::contains(id, "54_st_st") || Direction != 1) return;
        //if (! boost::contains(id, "54_iet_st") || Direction != 1) return;
        //if (! (boost::contains(id, "54_") || boost::contains(id, "55_")) || Direction != 1) return;
        //if (Direction != 1) return;
        // END DEBUG ONE ...


        /*** FOR REVERSING ONLY
        {
            // If one or both are invalid (e.g. ccw),
            // they can be corrected by uncommenting this section
            G1 cg1 = g1;
            G2 cg2 = g2;
            bg::correct(cg1);
            bg::correct(cg2);
            std::cout << std::setprecision(12)
                << bg::wkt(cg1) << std::endl
                << bg::wkt(cg2) << std::endl;
        }
        ***/

#if defined(BOOST_GEOMETRY_DEBUG_OVERLAY) || defined(BOOST_GEOMETRY_DEBUG_ENRICH)
        bool const ccw =
            bg::point_order<G1>::value == bg::counterclockwise
            || bg::point_order<G2>::value == bg::counterclockwise;

        std::cout << std::endl
            << "TRAVERSE"
            << " " << id
            << (ccw ? "_ccw" : "")
            << " " << string_from_type<typename bg::coordinate_type<G1>::type>::name()
            << "("  << operation(Direction) << ")" << std::endl;

        //std::cout << bg::area(g1) << " " << bg::area(g2) << std::endl;
#endif

        typedef typename bg::strategy_side
        <
            typename bg::cs_tag<G1>::type
        >::type side_strategy_type;


        typedef bg::detail::overlay::traversal_turn_info
            <
                typename bg::point_type<G2>::type
            > turn_info;
        std::vector<turn_info> turns;

        bg::detail::get_turns::no_interrupt_policy policy;
        bg::get_turns<Reverse1, Reverse2, bg::detail::overlay::calculate_distance_policy>(g1, g2, turns, policy);
        bg::enrich_intersection_points<Reverse1, Reverse2>(turns,
                    Direction == 1 ? bg::detail::overlay::operation_union
                    : bg::detail::overlay::operation_intersection,
            g1, g2, side_strategy_type());

        typedef bg::model::linear_ring<typename bg::point_type<G2>::type> ring_type;
        typedef std::vector<ring_type> out_vector;
        out_vector v;


        bg::traverse<Reverse1, Reverse2>(g1, g2, Direction, turns, v);

        // Check number of resulting rings
        BOOST_CHECK_MESSAGE(expected_count_area.get<0>() == boost::size(v),
                "traverse: " << id
                << " #shapes expected: " << expected_count_area.get<0>()
                << " detected: " << boost::size(v)
                << " type: " << string_from_type
                    <typename bg::coordinate_type<G1>::type>::name()
                );

        // Check total area of resulting rings
        typename bg::area_result<G1>::type total_area = 0;
        BOOST_FOREACH(ring_type const& ring, v)
        {
            total_area += bg::area(ring);
            //std::cout << bg::wkt(ring) << std::endl;
        }

        BOOST_CHECK_CLOSE(expected_count_area.get<1>(),
            boost::numeric_cast<double>(total_area),
            precision);

#if defined(TEST_WITH_SVG)
        {
            std::ostringstream filename;
            filename << "traverse_" << operation(Direction)
                << "_" << id
                << "_" << string_from_type<typename bg::coordinate_type<G1>::type>::name()
                << ".svg";

            std::ofstream svg(filename.str().c_str());

            bg::svg_mapper<typename bg::point_type<G2>::type> mapper(svg, 500, 500);
            mapper.add(g1);
            mapper.add(g2);

            // Input shapes in green/blue
            mapper.map(g1, "fill-opacity:0.5;fill:rgb(153,204,0);"
                    "stroke:rgb(153,204,0);stroke-width:3");
            mapper.map(g2, "fill-opacity:0.3;fill:rgb(51,51,153);"
                    "stroke:rgb(51,51,153);stroke-width:3");

            // Traversal rings in magenta outline/red fill -> over blue/green this gives brown
            BOOST_FOREACH(ring_type const& ring, v)
            {
                mapper.map(ring, "fill-opacity:0.2;stroke-opacity:0.4;fill:rgb(255,0,0);"
                        "stroke:rgb(255,0,255);stroke-width:8");
            }

            // turn points in orange, + enrichment/traversal info
            typedef typename bg::coordinate_type<G1>::type coordinate_type;

            // Simple map to avoid two texts at same place (note that can still overlap!)
            std::map<std::pair<int, int>, int> offsets;
            int index = 0;
            int const margin = 5;

            BOOST_FOREACH(turn_info const& turn, turns)
            {
                int lineheight = 10;
                mapper.map(turn.point, "fill:rgb(255,128,0);"
                        "stroke:rgb(0,0,0);stroke-width:1", 3);

                {
                    coordinate_type half = 0.5;
                    coordinate_type ten = 10;
                    // Map characteristics
                    // Create a rounded off point
                    std::pair<int, int> p
                        = std::make_pair(
                            boost::numeric_cast<int>(half
                                + ten * bg::get<0>(turn.point)),
                            boost::numeric_cast<int>(half
                                + ten * bg::get<1>(turn.point))
                            );
                std::string style =  "fill:rgb(0,0,0);font-family:Arial;font-size:10px";

                if (turn.discarded)
                {
                    style =  "fill:rgb(92,92,92);font-family:Arial;font-size:6px";
                    lineheight = 6;
                }

                //if (! turn.is_discarded() && ! turn.blocked() && ! turn.both(bg::detail::overlay::operation_union))
                //if (! turn.discarded)
                {
                    std::ostringstream out;
                    out << index
                        << ": " << bg::method_char(turn.method)
                        << std::endl
                        << "op: " << bg::operation_char(turn.operations[0].operation)
                        << " / " << bg::operation_char(turn.operations[1].operation)
                        << (turn.is_discarded() ? " (discarded) " : turn.blocked() ? " (blocked)" : "")
                        << std::endl;

                    if (turn.operations[0].enriched.next_ip_index != -1)
                    {
                        out << "ip: " << turn.operations[0].enriched.next_ip_index;
                    }
                    else
                    {
                        out << "vx: " << turn.operations[0].enriched.travels_to_vertex_index
                         << " -> ip: "  << turn.operations[0].enriched.travels_to_ip_index;
                    }
                    out << " ";
                    if (turn.operations[1].enriched.next_ip_index != -1)
                    {
                        out << "ip: " << turn.operations[1].enriched.next_ip_index;
                    }
                    else
                    {
                        out << "vx: " << turn.operations[1].enriched.travels_to_vertex_index
                         << " -> ip: " << turn.operations[1].enriched.travels_to_ip_index;
                    }

                    out << std::endl;

                    /*out

                        << std::setprecision(3)
                        << "dist: " << boost::numeric_cast<double>(turn.operations[0].enriched.distance)
                        << " / "  << boost::numeric_cast<double>(turn.operations[1].enriched.distance)
                        << std::endl
                        << "vis: " << bg::visited_char(turn.operations[0].visited)
                        << " / "  << bg::visited_char(turn.operations[1].visited);
                    */

                    /*
                        out << index
                            << ": " << bg::operation_char(turn.operations[0].operation)
                            << " " << bg::operation_char(turn.operations[1].operation)
                            << " (" << bg::method_char(turn.method) << ")"
                            << (turn.ignore() ? " (ignore) " : " ")
                            << std::endl

                            << "ip: " << turn.operations[0].enriched.travels_to_ip_index
                            << "/"  << turn.operations[1].enriched.travels_to_ip_index;

                        if (turn.operations[0].enriched.next_ip_index != -1
                            || turn.operations[1].enriched.next_ip_index != -1)
                        {
                            out << " [" << turn.operations[0].enriched.next_ip_index
                                << "/"  << turn.operations[1].enriched.next_ip_index
                                << "]"
                                ;
                        }
                        out << std::endl;


                        out
                            << "vx:" << turn.operations[0].enriched.travels_to_vertex_index
                            << "/"  << turn.operations[1].enriched.travels_to_vertex_index
                            << std::endl

                            << std::setprecision(3)
                            << "dist: " << turn.operations[0].enriched.distance
                            << " / "  << turn.operations[1].enriched.distance
                            << std::endl
                            */



                        offsets[p] += lineheight;
                        int offset = offsets[p];
                        offsets[p] += lineheight * 3;
                        mapper.text(turn.point, out.str(), style, margin, offset, lineheight);
                    }
                    index++;
                }
            }
        }
        #endif
    }
};


#if ! defined(BOOST_GEOMETRY_TEST_MULTI)
template <typename T>
void test_all(bool test_self_tangencies = true, bool test_mixed = false)
{
    using namespace bg::detail::overlay;

    typedef bg::model::point<T, 2, bg::cs::cartesian> P;
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::box<P> box;
    typedef boost::tuple<int, double> Tuple;

    // 1-6
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("1", boost::make_tuple(1, 5.4736), case_1[0], case_1[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("2", boost::make_tuple(1, 12.0545), case_2[0], case_2[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("3", boost::make_tuple(1, 5), case_3[0], case_3[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("4", boost::make_tuple(1, 10.2212), case_4[0], case_4[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("5", boost::make_tuple(2, 12.8155), case_5[0], case_5[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("6", boost::make_tuple(1, 4.5), case_6[0], case_6[1]);

    // 7-12
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("7", boost::make_tuple(0, 0), case_7[0], case_7[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("8", boost::make_tuple(0, 0), case_8[0], case_8[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("9", boost::make_tuple(0, 0), case_9[0], case_9[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("10", boost::make_tuple(0, 0), case_10[0], case_10[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("11", boost::make_tuple(1, 1), case_11[0], case_11[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("12", boost::make_tuple(2, 0.63333), case_12[0], case_12[1]);

    // 13-18
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("13", boost::make_tuple(0, 0), case_13[0], case_13[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("14", boost::make_tuple(0, 0), case_14[0], case_14[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("15", boost::make_tuple(0, 0), case_15[0], case_15[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("16", boost::make_tuple(0, 0), case_16[0], case_16[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("17", boost::make_tuple(1, 2), case_17[0], case_17[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("18", boost::make_tuple(1, 2), case_18[0], case_18[1]);

    // 19-24
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("19", boost::make_tuple(0, 0), case_19[0], case_19[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("20", boost::make_tuple(1, 5.5), case_20[0], case_20[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("21", boost::make_tuple(0, 0), case_21[0], case_21[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("22", boost::make_tuple(0, 0), case_22[0], case_22[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("23", boost::make_tuple(1, 1.4), case_23[0], case_23[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("24", boost::make_tuple(1, 1.0), case_24[0], case_24[1]);

    // 25-30
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("25", boost::make_tuple(0, 0), case_25[0], case_25[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("26", boost::make_tuple(0, 0), case_26[0], case_26[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("27", boost::make_tuple(1, 0.9545454), case_27[0], case_27[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("28", boost::make_tuple(1, 0.9545454), case_28[0], case_28[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("29", boost::make_tuple(1, 1.4), case_29[0], case_29[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("30", boost::make_tuple(1, 0.5), case_30[0], case_30[1]);

    // 31-36
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("31", boost::make_tuple(0, 0), case_31[0], case_31[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("32", boost::make_tuple(0, 0), case_32[0], case_32[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("33", boost::make_tuple(0, 0), case_33[0], case_33[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("34", boost::make_tuple(1, 0.5), case_34[0], case_34[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("35", boost::make_tuple(1, 1.0), case_35[0], case_35[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("36", boost::make_tuple(1, 1.625), case_36[0], case_36[1]);

    // 37-42
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("37", boost::make_tuple(2, 0.666666), case_37[0], case_37[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("38", boost::make_tuple(2, 0.971429), case_38[0], case_38[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("39", boost::make_tuple(1, 24), case_39[0], case_39[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("40", boost::make_tuple(0, 0), case_40[0], case_40[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("41", boost::make_tuple(1, 5), case_41[0], case_41[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("42", boost::make_tuple(1, 5), case_42[0], case_42[1]);

    // 43-48 - invalid polygons
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("43", boost::make_tuple(2, 0.75), case_43[0], case_43[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("44", boost::make_tuple(1, 44), case_44[0], case_44[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("45", boost::make_tuple(1, 45), case_45[0], case_45[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("46", boost::make_tuple(1, 46), case_46[0], case_46[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("47", boost::make_tuple(1, 47), case_47[0], case_47[1]);

    // 49-54

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("50", boost::make_tuple(0, 0), case_50[0], case_50[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("51", boost::make_tuple(0, 0), case_51[0], case_51[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("52", boost::make_tuple(1, 10.5), case_52[0], case_52[1]);
    if (test_self_tangencies)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("53_st", boost::make_tuple(0, 0), case_53[0], case_53[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("53_iet", boost::make_tuple(0, 0), case_53[0], case_53[2]);

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54_iet_iet", boost::make_tuple(1, 2), case_54[1], case_54[3]);
    if (test_self_tangencies)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54_st_iet", boost::make_tuple(1, 2), case_54[0], case_54[3]);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54_iet_st", boost::make_tuple(1, 2), case_54[1], case_54[2]);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54_st_st", boost::make_tuple(1, 2), case_54[0], case_54[2]);
    }

    if (test_self_tangencies)
    {
        // 55-60
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("55_st_st", boost::make_tuple(1, 2), case_55[0], case_55[2]);
    }

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("55_st_iet", boost::make_tuple(1, 2), case_55[0], case_55[3]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("55_iet_st", boost::make_tuple(1, 2), case_55[1], case_55[2]);
    if (test_self_tangencies)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("56", boost::make_tuple(2, 4.5), case_56[0], case_56[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("55_iet_iet", boost::make_tuple(1, 2), case_55[1], case_55[3]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("57", boost::make_tuple(2, 5.9705882), case_57[0], case_57[1]);

    if (test_self_tangencies)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("58_st",
            boost::make_tuple(2, 0.333333),
            case_58[0], case_58[1]);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("59_st",
            boost::make_tuple(2, 1.5416667),
            case_59[0], case_59[1]);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("60_st",
            boost::make_tuple(3, 2),
            case_60[0], case_60[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("58_iet",
        boost::make_tuple(2, 0.333333),
        case_58[0], case_58[2]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("59_iet",
        boost::make_tuple(2, 1.5416667),
        case_59[0], case_59[2]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("60_iet",
        boost::make_tuple(3, 2),
        case_60[0], case_60[2]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("61_st",
        boost::make_tuple(0, 0),
        case_61[0], case_61[1]);

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("70",
        boost::make_tuple(2, 4),
        case_70[0], case_70[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("71",
        boost::make_tuple(2, 2),
        case_71[0], case_71[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("72",
        boost::make_tuple(3, 2.85),
        case_72[0], case_72[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("79",
        boost::make_tuple(2, 20),
        case_79[0], case_79[1]);

    // other



    // pies (went wrong when not all cases where implemented, especially some collinear (opposite) cases
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_16_4_12",
        boost::make_tuple(1, 491866.5), pie_16_4_12[0], pie_16_4_12[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_23_21_12_500",
        boost::make_tuple(2, 2363199.3313), pie_23_21_12_500[0], pie_23_21_12_500[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_23_23_3_2000",
        boost::make_tuple(2, 1867779.9349), pie_23_23_3_2000[0], pie_23_23_3_2000[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_23_16_16",
        boost::make_tuple(2, 2128893.9555), pie_23_16_16[0], pie_23_16_16[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_16_2_15_0",
        boost::make_tuple(0, 0), pie_16_2_15_0[0], pie_16_2_15_0[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_4_13_15",
        boost::make_tuple(1, 490887.06678), pie_4_13_15[0], pie_4_13_15[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("pie_20_20_7_100",
        boost::make_tuple(2, 2183372.2718), pie_20_20_7_100[0], pie_20_20_7_100[1]);



    // 1-6
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("1", boost::make_tuple(1, 11.5264), case_1[0], case_1[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("2", boost::make_tuple(1, 17.9455), case_2[0], case_2[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("3", boost::make_tuple(1, 9), case_3[0], case_3[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("4", boost::make_tuple(3, 17.7788), case_4[0], case_4[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("5", boost::make_tuple(2, 18.4345), case_5[0], case_5[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("6", boost::make_tuple(1, 9), case_6[0], case_6[1]);

    // 7-12
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("7", boost::make_tuple(1, 9), case_7[0], case_7[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("8", boost::make_tuple(1, 12), case_8[0], case_8[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("9", boost::make_tuple(0, 0 /*UU 2, 11*/), case_9[0], case_9[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("10", boost::make_tuple(1, 9), case_10[0], case_10[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("11", boost::make_tuple(1, 8), case_11[0], case_11[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("12", boost::make_tuple(2, 8.36667), case_12[0], case_12[1]);

    // 13-18
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("13", boost::make_tuple(1, 4), case_13[0], case_13[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("14", boost::make_tuple(1, 12), case_14[0], case_14[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("15", boost::make_tuple(1, 12), case_15[0], case_15[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("16", boost::make_tuple(1, 9), case_16[0], case_16[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("17", boost::make_tuple(1, 8), case_17[0], case_17[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("18", boost::make_tuple(1, 8), case_18[0], case_18[1]);

    // 19-24
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("19", boost::make_tuple(1, 10), case_19[0], case_19[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("20", boost::make_tuple(1, 5.5), case_20[0], case_20[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("21", boost::make_tuple(0, 0), case_21[0], case_21[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("22", boost::make_tuple(0, 0 /*UU 2, 9.5*/), case_22[0], case_22[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("23", boost::make_tuple(1, 6.1), case_23[0], case_23[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("24", boost::make_tuple(1, 5.5), case_24[0], case_24[1]);

    // 25-30
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("25", boost::make_tuple(0, 0 /*UU 2, 7*/), case_25[0], case_25[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("26", boost::make_tuple(0, 0 /*UU  2, 7.5 */), case_26[0], case_26[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("27", boost::make_tuple(1, 8.04545), case_27[0], case_27[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("28", boost::make_tuple(1, 10.04545), case_28[0], case_28[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("29", boost::make_tuple(1, 8.1), case_29[0], case_29[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("30", boost::make_tuple(1, 6.5), case_30[0], case_30[1]);

    // 31-36
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("31", boost::make_tuple(0, 0 /*UU 2, 4.5 */), case_31[0], case_31[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("32", boost::make_tuple(0, 0 /*UU 2, 4.5 */), case_32[0], case_32[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("33", boost::make_tuple(0, 0 /*UU 2, 4.5 */), case_33[0], case_33[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("34", boost::make_tuple(1, 6.0), case_34[0], case_34[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("35", boost::make_tuple(1, 10.5), case_35[0], case_35[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("36", boost::make_tuple(1 /*UU 2*/, 14.375), case_36[0], case_36[1]);

    // 37-42
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("37", boost::make_tuple(1, 7.33333), case_37[0], case_37[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("38", boost::make_tuple(1, 9.52857), case_38[0], case_38[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("39", boost::make_tuple(1, 40.0), case_39[0], case_39[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("40", boost::make_tuple(0, 0 /*UU 2, 11 */), case_40[0], case_40[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("41", boost::make_tuple(1, 5), case_41[0], case_41[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("42", boost::make_tuple(1, 5), case_42[0], case_42[1]);

    // 43-48
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("43", boost::make_tuple(3, 8.1875), case_43[0], case_43[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("44", boost::make_tuple(1, 44), case_44[0], case_44[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("45", boost::make_tuple(1, 45), case_45[0], case_45[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("46", boost::make_tuple(1, 46), case_46[0], case_46[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("47", boost::make_tuple(1, 47), case_47[0], case_47[1]);

    // 49-54

    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("50", boost::make_tuple(1, 25), case_50[0], case_50[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("51", boost::make_tuple(0, 0), case_51[0], case_51[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("52", boost::make_tuple(1, 15.5), case_52[0], case_52[1]);
    if (test_self_tangencies)
    {
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("53_st", boost::make_tuple(2, 16), case_53[0], case_53[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("53_iet",
            boost::make_tuple(2, 16),
            case_53[0], case_53[2]);
    if (test_self_tangencies)
    {
        // The st_st version might generate one ring with area zero, which is OK
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54_st_st", boost::make_tuple(3, 20), case_54[0], case_54[2]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54_st_iet", boost::make_tuple(2, 20), case_54[0], case_54[3]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54_iet_st", boost::make_tuple(2, 20), case_54[1], case_54[2]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54_iet_iet", boost::make_tuple(2, 20), case_54[1], case_54[3]);

    if (test_mixed)
    {
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("55_st_iet", boost::make_tuple(2, 18), case_55[0], case_55[3]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("55_iet_st", boost::make_tuple(2, 18), case_55[1], case_55[2]);
        // moved to mixed
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("55_iet_iet", boost::make_tuple(3, 18), case_55[1], case_55[3]);
    }

    // 55-60
    if (test_self_tangencies)
    {
        // 55 with both input polygons having self tangencies (st_st) generates 1 correct shape
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("55_st_st", boost::make_tuple(1, 18), case_55[0], case_55[2]);
        // 55 with one of them self-tangency, other int/ext ring tangency generate 2 correct shapes

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("56", boost::make_tuple(2, 14), case_56[0], case_56[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("57", boost::make_tuple(1, 14.029412), case_57[0], case_57[1]);

    if (test_self_tangencies)
    {
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("58_st",
            boost::make_tuple(4, 12.16666),
            case_58[0], case_58[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("59_st",
            boost::make_tuple(2, 17.208333),
            case_59[0], case_59[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("60_st",
            boost::make_tuple(3, 19),
            case_60[0], case_60[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("58_iet",
        boost::make_tuple( 4, 12.16666),
        case_58[0], case_58[2]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("59_iet",
        boost::make_tuple(1, -3.791666), // 2, 17.208333), outer ring (ii/ix) is done by ASSEMBLE
        case_59[0], case_59[2]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("60_iet",
        boost::make_tuple(3, 19),
        case_60[0], case_60[2]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("61_st",
        boost::make_tuple(1, 4),
        case_61[0], case_61[1]);

    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("70",
        boost::make_tuple(1, 9),
        case_70[0], case_70[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("71",
        boost::make_tuple(2, 9),
        case_71[0], case_71[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("72",
        boost::make_tuple(1, 10.65),
        case_72[0], case_72[1]);



    // other
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("collinear_overlaps",
        boost::make_tuple(1, 24),
        collinear_overlaps[0], collinear_overlaps[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("collinear_overlaps",
        boost::make_tuple(1, 50),
        collinear_overlaps[0], collinear_overlaps[1]);

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("many_situations", boost::make_tuple(1, 184), case_many_situations[0], case_many_situations[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("many_situations",
        boost::make_tuple(1, 207), case_many_situations[0], case_many_situations[1]);


    // From "intersection piets", robustness test.
    // This all went wrong in the past
    // (when not all cases (get_turns) where implemented,
    //   especially important are some collinear (opposite) cases)
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_16_4_12",
        boost::make_tuple(1, 3669665.5), pie_16_4_12[0], pie_16_4_12[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_23_21_12_500",
        boost::make_tuple(1, 6295516.7185), pie_23_21_12_500[0], pie_23_21_12_500[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_23_23_3_2000",
        boost::make_tuple(1, 7118735.0530), pie_23_23_3_2000[0], pie_23_23_3_2000[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_23_16_16",
        boost::make_tuple(1, 5710474.5406), pie_23_16_16[0], pie_23_16_16[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_16_2_15_0",
        boost::make_tuple(1, 3833641.5), pie_16_2_15_0[0], pie_16_2_15_0[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_4_13_15",
        boost::make_tuple(1, 2208122.43322), pie_4_13_15[0], pie_4_13_15[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_20_20_7_100",
        boost::make_tuple(1, 5577158.72823), pie_20_20_7_100[0], pie_20_20_7_100[1]);

    /*
    if (test_not_valid)
    {
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("pie_5_12_12_0_7s",
        boost::make_tuple(1, 3271710.48516), pie_5_12_12_0_7s[0], pie_5_12_12_0_7s[1]);
    }
    */

    static const bool is_float
        = boost::is_same<T, float>::value;
    static const bool is_double
        = boost::is_same<T, double>::value
        || boost::is_same<T, long double>::value;

    static const double float_might_deviate_more = is_float ? 0.1 : 0.001; // In some cases up to 1 promille permitted

    // GCC: does not everywhere handle float correctly (in our algorithms)
    bool const is_float_on_non_msvc =
#if defined(_MSC_VER)
        false;
#else
        is_float;
#endif



    // From "Random Ellipse Stars", robustness test.
    // This all went wrong in the past
    // when using Determinant/ra/rb and comparing with 0/1
    // Solved now by avoiding determinant / using sides
    // ("hv" means "high volume")
    {
        double deviation = is_float ? 0.01 : 0.001;
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv1", boost::make_tuple(1, 1624.508688461573), hv_1[0], hv_1[1], deviation);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv1", boost::make_tuple(1, 1622.7200125123809), hv_1[0], hv_1[1], deviation);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv2", boost::make_tuple(1, 1622.9193392726836), hv_2[0], hv_2[1], deviation);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv2", boost::make_tuple(1, 1622.1733591429329), hv_2[0], hv_2[1], deviation);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv3", boost::make_tuple(1, 1624.22079205664), hv_3[0], hv_3[1], deviation);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv3", boost::make_tuple(1, 1623.8265057282042), hv_3[0], hv_3[1], deviation);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv4", boost::make_tuple(1, 1626.5146964146334), hv_4[0], hv_4[1], deviation);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv4", boost::make_tuple(1, 1626.2580370864305), hv_4[0], hv_4[1], deviation);

        if (! is_float)
        {
            test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv5", boost::make_tuple(1, 1624.2158307261871), hv_5[0], hv_5[1], deviation);
            test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv5", boost::make_tuple(1, 1623.4506071521519), hv_5[0], hv_5[1], deviation);

            // Case 2009-12-07
            test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv6", boost::make_tuple(1, 1604.6318757402121), hv_6[0], hv_6[1], deviation);
            test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv6", boost::make_tuple(1, 1790.091872401327), hv_6[0], hv_6[1], deviation);

            // Case 2009-12-08, needing sorting on side in enrich_intersection_points
            test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv7", boost::make_tuple(1, 1624.5779453641017), hv_7[0], hv_7[1], deviation);
            test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv7", boost::make_tuple(1, 1623.6936420295772), hv_7[0], hv_7[1], deviation);
        }
    }

    // From "Random Ellipse Stars", robustness test.
    // This all went wrong in the past when distances (see below) were zero (dz)
    // "Distance zero", dz, means: the distance between two intersection points
    // on a same segment is 0, therefore it can't be sorted normally, therefore
    // the chance is 50% that the segments are not sorted correctly and the wrong
    // decision is taken.
    // Solved now (by sorting on sides in those cases)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_1",
                boost::make_tuple(3, 16.887537949472005), dz_1[0], dz_1[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_1",
                boost::make_tuple(3, 1444.2621305732864), dz_1[0], dz_1[1]);

        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_2",
                boost::make_tuple(2, 68.678921274288541), dz_2[0], dz_2[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_2",
                boost::make_tuple(2, 1505.4202304878663), dz_2[0], dz_2[1]);

        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_3",
                boost::make_tuple(6, 192.49316937645651), dz_3[0], dz_3[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_3",
                boost::make_tuple(6, 1446.496005965641), dz_3[0], dz_3[1]);

        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_4",
                boost::make_tuple(1, 473.59423868207693), dz_4[0], dz_4[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_4",
                boost::make_tuple(1, 1871.6125138873476), dz_4[0], dz_4[1]);
    }

    // Real-life problems

    // SNL (Subsidiestelsel Natuur & Landschap - verAANnen)

    if (! is_float_on_non_msvc)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("snl-1",
            boost::make_tuple(2, 286.996062095888),
            snl_1[0], snl_1[1],
            float_might_deviate_more);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("snl-1",
            boost::make_tuple(2, 51997.5408506132),
            snl_1[0], snl_1[1],
            float_might_deviate_more);
    }

    {
        // Note: values are checked with SQL Server,
        /*
            select geometry::STGeomFromText('POLYGON((...))', 0)
                .STIntersection(geometry::STGeomFromText('...))', 0))
                .STArea()

            and STUnion
        */

        // Boost.List during Formal Review, isovists Brandon
        // For FP, they may deviate more.
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("isov",
                boost::make_tuple(1, 88.1920416352664), isovist[0], isovist[1],
                float_might_deviate_more);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("isov",
                boost::make_tuple(1, 313.360374193241), isovist[0], isovist[1],
                float_might_deviate_more);
    }

    // GEOS tests
    if (! is_float)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("geos_1_test_overlay",
                boost::make_tuple(1, 3461.02330171138), geos_1_test_overlay[0], geos_1_test_overlay[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("geos_1_test_overlay",
                boost::make_tuple(1, 3461.31592235516), geos_1_test_overlay[0], geos_1_test_overlay[1]);

        if (! is_double)
        {
            test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("geos_2",
                    boost::make_tuple(2, 2.155655988e-6), // by bg/ttmath; sql server reports: 2.20530228034477E-06
                    geos_2[0], geos_2[1]);
        }
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("geos_2",
                boost::make_tuple(1, 350.550662845485),
                geos_2[0], geos_2[1]);
    }

    if (! is_float && ! is_double)
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("geos_3",
                boost::make_tuple(1, 2.484885e-7),
                geos_3[0], geos_3[1]);
    }
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("geos_3",
            boost::make_tuple(1, 29391548.5),
            geos_3[0], geos_3[1],
            float_might_deviate_more);

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("geos_4",
            boost::make_tuple(1, 0.0836884926070727),
            geos_4[0], geos_4[1],
            is_float_on_non_msvc ? 3.0 : float_might_deviate_more);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("geos_4",
            boost::make_tuple(1, 2304.41633605957),
            geos_4[0], geos_4[1]);


    return;

    // Cases below still have errors

    // ticket#17
    test_overlay<polygon, box, test_traverse<operation_intersection>,  Tuple>("ticket_17", boost::make_tuple(2, 2.687433027e-006),
                ticket_17[0], ticket_17[1], 0.1);
    test_overlay<polygon, box, test_traverse<operation_union>,  Tuple>("ticket_17", boost::make_tuple(3, 0.00922511561516),
                ticket_17[0], ticket_17[1], 0.1);
}

template <typename T>
void test_open()
{
    using namespace bg::detail::overlay;

    typedef bg::model::point<T, 2, bg::cs::cartesian> P;
    typedef bg::model::polygon<P, true, false> polygon;
    typedef boost::tuple<int, double> Tuple;

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("open_1", boost::make_tuple(1, 5.4736),
        open_case_1[0], open_case_1[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("open_1", boost::make_tuple(1, 11.5264),
        open_case_1[0], open_case_1[1]);
}

template <typename T>
void test_ccw()
{
    using namespace bg::detail::overlay;

    typedef bg::model::point<T, 2, bg::cs::cartesian> P;
    typedef bg::model::polygon<P, false, true> polygon;
    typedef boost::tuple<int, double> Tuple;

    test_overlay<polygon, polygon, test_traverse<operation_intersection, true, true>,  Tuple>("ccw_1", boost::make_tuple(1, 5.4736),
        ccw_case_1[0], ccw_case_1[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union, true, true>,  Tuple>("ccw_1", boost::make_tuple(1, 11.5264),
        ccw_case_1[0], ccw_case_1[1]);
}



int test_main(int, char* [])
{
    //test_all<float>();
    test_all<double>();
    test_open<double>();
    test_ccw<double>();

#if ! defined(_MSC_VER)
    test_all<long double>();
#endif

#ifdef HAVE_TTMATH
    test_all<ttmath_big>();
#endif

    return 0;
 }

#endif
