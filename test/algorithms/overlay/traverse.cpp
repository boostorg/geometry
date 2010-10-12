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


#include <geometry_test_common.hpp>

//#define HAVE_TTMATH
#ifdef HAVE_TTMATH
#  include <ttmath_stub.hpp>
#endif



#define BOOST_GEOMETRY_REPORT_OVERLAY_ERROR
#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER

//#define BOOST_GEOMETRY_DEBUG_OVERLAY_ONLY_ONE

#if defined(_MSC_VER)
#  define BOOST_GEOMETRY_TEST_OVERLAY_NOT_REVERSED
//#  define BOOST_GEOMETRY_DEBUG_OVERLAY
#else
//#  define BOOST_GEOMETRY_DEBUG_OVERLAY
#endif


#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>

#include <boost/geometry/algorithms/area.hpp>


#include <boost/geometry/strategies/strategies.hpp>

#include <algorithms/overlay/overlay_common.hpp>
#include <algorithms/overlay/overlay_cases.hpp>

#if defined(TEST_WITH_SVG)
#  include <boost/geometry/extensions/io/svg/svg_mapper.hpp>
#endif

template <boost::geometry::detail::overlay::operation_type Direction>
struct test_traverse
{
#if defined(TEST_WITH_SVG)
    static inline std::string operation(int d)
    {
        return d == 1 ? "union" : "intersection";
    }
#endif

    template <typename G1, typename G2>
    static void apply(std::string const& id,
            boost::tuple<int, double> const& expected_count_area,
            G1 const& g1, G2 const& g2,
            double precision)
    {
        namespace bg = boost::geometry;

        //std::cout << "TRAVERSE " << id << "("  << int(Direction) << ")" << std::endl;
#ifdef BOOST_GEOMETRY_DEBUG_OVERLAY
        //std::cerr << "TRAVERSE " << id << "("  << Direction << ")" << std::endl;

        //std::cout << bg::area(g1) << " " << bg::area(g2) << std::endl;
#endif

        typedef typename bg::strategy_side
        <
            typename bg::cs_tag<G1>::type
        >::type side_strategy_type;


        typedef bg::detail::overlay::traversal_turn_info
            <
                typename boost::geometry::point_type<G2>::type
            > turn_info;
        std::vector<turn_info> turns;

        bg::detail::get_turns::no_interrupt_policy policy;
        bg::get_turns<bg::detail::overlay::calculate_distance_policy>(g1, g2, turns, policy);
        bg::enrich_intersection_points(turns, g1, g2, side_strategy_type());

        typedef bg::linear_ring<typename bg::point_type<G2>::type> ring_type;
        typedef std::vector<ring_type> out_vector;
        out_vector v;


        bg::traverse<bg::clockwise>(g1, g2, Direction, turns, v);

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
        }

#ifndef HAVE_TTMATH
        BOOST_CHECK_CLOSE(expected_count_area.get<1>(), double(total_area), precision);
#endif


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
            int const lineheight = 10;
            int const margin = 5;

            BOOST_FOREACH(turn_info const& turn, turns)
            {
                mapper.map(turn.point, "fill:rgb(255,128,0);"
                        "stroke:rgb(0,0,0);stroke-width:1", 3);

                {
                    // Map characteristics
                    // Create a rounded off point
                    std::pair<int, int> p
                        = std::make_pair(
                            boost::numeric_cast<int>(0.5 + 10 * bg::get<0>(turn.point)),
                            boost::numeric_cast<int>(0.5 + 10 * bg::get<1>(turn.point))
                            );
                    std::string style =  "fill:rgb(0,0,0);font-family:Arial;font-size:8px";

                    std::ostringstream out;
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

                        << "vis: " << bg::visited_char(turn.operations[0].visited)
                        << " / "  << bg::visited_char(turn.operations[1].visited);

                    offsets[p] += lineheight;
                    int offset = offsets[p];
                    offsets[p] += lineheight * 5;
                    mapper.text(turn.point, out.str(), style, margin, offset, lineheight);
                }
                index++;
            }
        }
        #endif
    }
};


#if ! defined(BOOST_GEOMETRY_TEST_MULTI)
template <typename T>
void test_all()
{
    namespace bg = boost::geometry;
    using namespace boost::geometry::detail::overlay;

    typedef bg::point<T, 2, bg::cs::cartesian> P;
    typedef bg::polygon<P> polygon;
    typedef bg::box<P> box;
    typedef boost::tuple<int, double> Tuple;


    std::cout << std::setprecision(10);

#ifdef BOOST_GEOMETRY_DEBUG_OVERLAY_ONLY_ONE
    {
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_2",
                boost::make_tuple(2, 68.678921274288541), dz_2[0], dz_2[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_2",
                boost::make_tuple(2, 1505.4202304878663), dz_2[0], dz_2[1]);
    }
    return;
#endif

    /*
        // SNL problem
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("snl-3",
        boost::make_tuple(1, 99),
        "POLYGON((184913.4512400339881423860788345336914 606985.779408219968900084495544433594,184912.8999999999941792339086532592773 606987.145999999949708580970764160156,184904.4135310589917935431003570556641 606987.651360383024439215660095214844,184901.847619076987029984593391418457 607014.593436188995838165283203125,184916.3977574919990729540586471557617 607021.060164373018778860569000244141,184927.7147701499925460666418075561523 607008.126435620011761784553527832031,184926.0980706939881201833486557006836 606998.426238880958408117294311523438,184913.4512400339881423860788345336914 606985.779408219968900084495544433594),(184907.5560000000114087015390396118164 607013.300999999977648258209228515625,184905.7820000000065192580223083496094 607009.971999999950639903545379638672,184906.0039999999862629920244216918945 607005.978000000002793967723846435547,184908.4439999999885912984609603881836 606998.876999999978579580783843994141,184912.2149999999965075403451919555664 606994.217999999993480741977691650391,184919.3140000000130385160446166992188 606993.996000000042840838432312011719,184922.4200000000128056854009628295898 606995.770999999949708580970764160156,184925.7470000000030267983675003051758 606998.876999999978579580783843994141,184926.4130000000004656612873077392578 607002.871999999973922967910766601563,184925.7470000000030267983675003051758 607007.753000000026077032089233398438,184922.4200000000128056854009628295898 607012.190999999991618096828460693359,184917.0959999999904539436101913452148 607015.297999999951571226119995117188,184911.7710000000079162418842315673828 607015.297999999951571226119995117188,184907.5560000000114087015390396118164 607013.300999999977648258209228515625))",
        "POLYGON((184861.1180000010062940418720245361328 606901.158000000054016709327697753906,184893.7870000000111758708953857421875 606898.482999998959712684154510498047,184925.0430000009946525096893310546875 606913.399999998975545167922973632813,184927.1739999990095384418964385986328 606951.758999999961815774440765380859,184912.8999999990046489983797073364258 606987.146000002045184373855590820313,184877.8700000010139774531126022338867 606989.232000001007691025733947753906,184885.1030000000027939677238464355469 607023.773999999975785613059997558594,184899.0579999980109278112649917602539 607022.743000000948086380958557128906,184906.0080000009911600500345230102539 607044.947999999043531715869903564453,184966.4649999999965075403451919555664 607025.020000000018626451492309570313,184968.4420000019890721887350082397461 606961.300000000977888703346252441406,185024.7679999989923089742660522460938 606947.401999998954124748706817626953,185024.5439999999944120645523071289063 606941.354999999981373548507690429688,185027.0069999989937059581279754638672 606937.322999999043531715869903564453,185030.3660000000090803951025009155273 606934.186999998986721038818359375,185035.5159999990137293934822082519531 606933.962999999988824129104614257813,185040.4420000019890721887350082397461 606935.530999999027699232101440429688,185042.905000000988366082310676574707 606939.114999998011626303195953369141,185088.3640000000013969838619232177734 606931.385000001988373696804046630859,185089.1389999990060459822416305541992 607015.508999999961815774440765380859,185095.1999999989930074661970138549805 607011.300000000977888703346252441406,185118.8269999999902211129665374755859 606995.545000002020969986915588378906,185126.813000001013278961181640625 606991.9950000010430812835693359375,185177.7270000019925646483898162841797 606973.798999998951330780982971191406,185181.4820000010076910257339477539063 606966.67599999904632568359375,185193.5709999990067444741725921630859 606977.795000002020969986915588378906,185193.710999998991610482335090637207 606960.300000000977888703346252441406,185189.3520000019925646483898162841797 606779.020000000018626451492309570313,185167.5150000010035000741481781005859 606783.844000000972300767898559570313,185086.9600000010104849934577941894531 606801.241000000038184225559234619141,185011.7069999990053474903106689453125 606817.809000000008381903171539306641,185000 606819.304000001051463186740875244141,184994.0340000019932631403207778930664 606819.793999999994412064552307128906,184976.3979999980074353516101837158203 606819.572000000975094735622406005859,184956.6539999989909119904041290283203 606817.1310000009834766387939453125,184934.9129999990109354257583618164063 606813.136999998008832335472106933594,184893.0969999989902134984731674194336 606804.927000000956468284130096435547,184884.4450000000069849193096160888672 606831.555000000051222741603851318359,184866.9189999999944120645523071289063 606883.480999998981133103370666503906,184861.1180000010062940418720245361328 606901.158000000054016709327697753906),(184907.5560000019904691725969314575195 607013.30099999904632568359375,184905.7820000019855797290802001953125 607009.971999999019317328929901123047,184906.0040000010048970580101013183594 607005.978000000002793967723846435547,184908.4439999980095308274030685424805 606998.876999999978579580783843994141,184912.2149999999965075403451919555664 606994.217999998014420270919799804688,184919.3139999989944044500589370727539 606993.995999998995102941989898681641,184922.420000001991866156458854675293 606995.771000002045184373855590820313,184925.7470000009925570338964462280273 606998.876999999978579580783843994141,184926.4129999990109354257583618164063 607002.872000001021660864353179931641,184925.7470000009925570338964462280273 607007.752999998978339135646820068359,184922.420000001991866156458854675293 607012.190999999991618096828460693359,184917.0960000010090880095958709716797 607015.297999999951571226119995117188,184911.7710000019869767129421234130859 607015.297999999951571226119995117188,184907.5560000019904691725969314575195 607013.30099999904632568359375))");

    return;
    */
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("49b", boost::make_tuple(1, 15), case_49[0], case_49[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("55c", boost::make_tuple(3, 18), case_55[0], case_55[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("55c", boost::make_tuple(1, 2), case_55[0], case_55[1]);
    //return;


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

    // 43-48
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("43", boost::make_tuple(2, 0.75), case_43[0], case_43[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("44", boost::make_tuple(1, 44), case_44[0], case_44[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("45", boost::make_tuple(1, 45), case_45[0], case_45[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("46", boost::make_tuple(1, 46), case_46[0], case_46[1]);
    //test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("47", boost::make_tuple(1, 47), case_47[0], case_47[1]);

    // 49-54

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("50", boost::make_tuple(0, 0), case_50[0], case_50[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("51", boost::make_tuple(0, 0), case_51[0], case_51[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("52", boost::make_tuple(1, 10.5), case_52[0], case_52[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("53a", boost::make_tuple(0, 0), case_53[0], case_53[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("53b", boost::make_tuple(0, 0), case_53[0], case_53[2]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54aa", boost::make_tuple(1, 2), case_54[0], case_54[2]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54ab", boost::make_tuple(1, 2), case_54[0], case_54[3]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54ba", boost::make_tuple(1, 2), case_54[1], case_54[2]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("54bb", boost::make_tuple(1, 2), case_54[1], case_54[3]);

    // 55-60
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("55", boost::make_tuple(1, 2), case_55[0], case_55[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("56", boost::make_tuple(2, 4.5), case_56[0], case_56[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("57", boost::make_tuple(2, 5.9705882), case_57[0], case_57[1]);


    // other

    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("collinear_overlaps", boost::make_tuple(1, 24), collinear_overlaps[0], collinear_overlaps[1]);
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("many_situations", boost::make_tuple(1, 184), case_many_situations[0], case_many_situations[1]);


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
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("9", boost::make_tuple(0, 0), case_9[0], case_9[1]);
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
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("22", boost::make_tuple(0, 0), case_22[0], case_22[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("23", boost::make_tuple(1, 6.1), case_23[0], case_23[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("24", boost::make_tuple(1, 5.5), case_24[0], case_24[1]);

    // 25-30
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("25", boost::make_tuple(0, 0), case_25[0], case_25[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("26", boost::make_tuple(0, 0), case_26[0], case_26[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("27", boost::make_tuple(1, 8.04545), case_27[0], case_27[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("28", boost::make_tuple(1, 10.04545), case_28[0], case_28[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("29", boost::make_tuple(1, 8.1), case_29[0], case_29[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("30", boost::make_tuple(1, 6.5), case_30[0], case_30[1]);

    // 31-36
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("31", boost::make_tuple(0, 0), case_31[0], case_31[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("32", boost::make_tuple(0, 0), case_32[0], case_32[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("33", boost::make_tuple(0, 0), case_33[0], case_33[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("34", boost::make_tuple(1, 6.0), case_34[0], case_34[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("35", boost::make_tuple(1, 10.5), case_35[0], case_35[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("36", boost::make_tuple(1, 14.375), case_36[0], case_36[1]);

    // 37-42
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("37", boost::make_tuple(1, 7.33333), case_37[0], case_37[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("38", boost::make_tuple(1, 9.52857), case_38[0], case_38[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("39", boost::make_tuple(1, 40.0), case_39[0], case_39[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("40", boost::make_tuple(0, 0), case_40[0], case_40[1]);
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
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("53a", boost::make_tuple(2, 16), case_53[0], case_53[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("53b", boost::make_tuple(2, 16), case_53[0], case_53[2]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54aa", boost::make_tuple(2, 20), case_54[0], case_54[2]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54ab", boost::make_tuple(2, 20), case_54[0], case_54[3]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54ba", boost::make_tuple(2, 20), case_54[1], case_54[2]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("54bb", boost::make_tuple(2, 20), case_54[1], case_54[3]);


    // 55-60
    // 55 is going wrong!
    // TODO: implement "node" behaviour which merges nodes
    //test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("55", boost::make_tuple(3, 18), case_55[0], case_55[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("56", boost::make_tuple(2, 14), case_56[0], case_56[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("57", boost::make_tuple(1, 14.029412), case_57[0], case_57[1]);

    // other
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("collinear_overlaps",
        boost::make_tuple(1, 50), collinear_overlaps[0], collinear_overlaps[1]);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("many_situations",
        boost::make_tuple(1, 207), case_many_situations[0], case_many_situations[1]);


    // pies (went wrong when not all cases where implemented, especially some collinear (opposite) cases
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


    // From "Random Ellipse Stars", this all went wrong
    // when using Determinant/ra/rb and comparing with 0/1
    // Solved now.
    // ("hv" means "high volume")
    {
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv1", boost::make_tuple(1, 1624.508688461573), hv_1[0], hv_1[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv1", boost::make_tuple(1, 1622.7200125123809), hv_1[0], hv_1[1], 0.1);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv2", boost::make_tuple(1, 1622.9193392726836), hv_2[0], hv_2[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv2", boost::make_tuple(1, 1622.1733591429329), hv_2[0], hv_2[1], 0.1);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv3", boost::make_tuple(1, 1624.22079205664), hv_3[0], hv_3[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv3", boost::make_tuple(1, 1623.8265057282042), hv_3[0], hv_3[1], 0.1);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv4", boost::make_tuple(1, 1626.5146964146334), hv_4[0], hv_4[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv4", boost::make_tuple(1, 1626.2580370864305), hv_4[0], hv_4[1], 0.1);

        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv5", boost::make_tuple(1, 1624.2158307261871), hv_5[0], hv_5[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv5", boost::make_tuple(1, 1623.4506071521519), hv_5[0], hv_5[1], 0.1);

        // Case 2009-12-07
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv6", boost::make_tuple(1, 1604.6318757402121), hv_6[0], hv_6[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv6", boost::make_tuple(1, 1790.091872401327), hv_6[0], hv_6[1], 0.1);

        // Case 2009-12-08, needing sorting on side in enrich_intersection_points
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv7", boost::make_tuple(1, 1624.5779453641017), hv_7[0], hv_7[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv7", boost::make_tuple(1, 1623.6936420295772), hv_7[0], hv_7[1], 0.1);
    }

    // From "Random Ellipse Stars", this all went wrong
    // when distances was zero (dz)
    // "Distance zero", dz, means: the distance between two intersection points
    // on a same segment is 0, therefore it can't be sorted normally, therefore
    // the chance is 50% that the segments are not sorted correctly and the wrong
    // decision is taken.
    // Solved (by sorting on sides instead of on distance)
    {
        // distance-zero-1
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_1",
                boost::make_tuple(3, 16.887537949472005), dz_1[0], dz_1[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_1",
                boost::make_tuple(3, 1444.2621305732864), dz_1[0], dz_1[1]);

        // distance-zero-2
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_2",
                boost::make_tuple(2, 68.678921274288541), dz_2[0], dz_2[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_2",
                boost::make_tuple(2, 1505.4202304878663), dz_2[0], dz_2[1]);

        // distance-zero-3
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_3",
                boost::make_tuple(6, 192.49316937645651), dz_3[0], dz_3[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_3",
                boost::make_tuple(6, 1446.496005965641), dz_3[0], dz_3[1]);
    }

    return;

    // They still have errors

    // ticket#17
    test_overlay<polygon, box, test_traverse<operation_intersection>,  Tuple>("ticket_17", boost::make_tuple(2, 2.687433027e-006), ticket_17[0], ticket_17[1], 0.1);
    test_overlay<polygon, box, test_traverse<operation_union>,  Tuple>("ticket_17", boost::make_tuple(3, 0.00922511561516), ticket_17[0], ticket_17[1], 0.1);

    // Boost.List during Formal Review, isovists Brandon
    // For FP, they may deviate more.
    /***
    static const bool is_float = string_from_type<T>::name() == std::string("f");
    static const bool is_double = string_from_type<T>::name() == std::string("d");
    test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("isov",
            boost::make_tuple(1, 88.19203119), isovist[0], isovist[1],
            is_float || is_double ? 1.0 : 0.01);
    test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("isov",
            boost::make_tuple(1, 313.3603646234), isovist[0], isovist[1],
            is_float || is_double ? 1.0 : 0.01);
    ***/
}


int test_main(int, char* [])
{
    //test_all<float>();
    test_all<double>();
#ifdef HAVE_TTMATH
    test_all<tt>();
#endif

#if ! defined(_MSC_VER)
    test_all<long double>();
#endif
#if defined(HAVE_CLN)
    test_all<boost::numeric_adaptor::cln_value_type>();
#endif
#if defined(HAVE_GMP)
    test_all<boost::numeric_adaptor::gmp_value_type>();
#endif
    return 0;
}

#endif
