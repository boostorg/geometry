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

#include <boost/geometry/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/overlay/traverse.hpp>

#include <boost/geometry/algorithms/area.hpp>


#include <boost/geometry/strategies/strategies.hpp>

#include <overlay_common.hpp>
#include <overlay_cases.hpp>

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


        bg::traverse(g1, g2, Direction, turns, v);

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

            svg_mapper<typename bg::point_type<G2>::type> mapper(svg, 500, 500);
            mapper.add(g1);
            mapper.add(g2);

            // Input shapes in green/blue
            mapper.map(g1, "fill-opacity:0.5;fill:rgb(153,204,0);"
                    "stroke:rgb(153,204,0);stroke-width:3");
            mapper.map(g2, "fill-opacity:0.3;fill:rgb(51,51,153);"
                    "stroke:rgb(51,51,153);stroke-width:3");

            // Traversal rings in magenta/light yellow fill
            BOOST_FOREACH(ring_type const& ring, v)
            {
                mapper.map(ring, "fill-opacity:0.05;stroke-opacity:0.4;fill:rbg(255,255,128);"
                        "stroke:rgb(255,0,255);stroke-width:8");
            }

            // turn points in orange, + enrichment/traversal info
            typedef typename bg::coordinate_type<G1>::type coordinate_type;

            // Simple map to avoid two texts at same place (note that can still overlap!)
            std::map<std::pair<coordinate_type, coordinate_type>, int> offsets;
            int index = 0;
            int const lineheight = 10;
            int const margin = 5;

            BOOST_FOREACH(turn_info const& turn, turns)
            {
                mapper.map(turn.point, "fill:rgb(255,128,0);"
                        "stroke:rgb(0,0,0);stroke-width:1", 3);

                {
                    // Map characteristics
                    std::pair<coordinate_type, coordinate_type> p
                        = std::make_pair(bg::get<0>(turn.point), bg::get<1>(turn.point));
                    std::string style =  "fill:rgb(0,0,0);font-family:Arial;font-size:8px";

                    {
                        std::ostringstream out;
                        out << index
                            << ": " << bg::operation_char(turn.operations[0].operation)
                            << " " << bg::operation_char(turn.operations[1].operation)
                            << " (" << bg::method_char(turn.method) << ")"
                            << (turn.ignore ? " (ignore) " : " ")
                            ;

                        offsets[p] += lineheight;
                        int offset = offsets[p];
                        mapper.text(turn.point, out.str(), style, margin, offset);
                    }

                    {
                        std::ostringstream out;
                        out << "ip: " << turn.operations[0].enriched.travels_to_ip_index
                            << "/"  << turn.operations[1].enriched.travels_to_ip_index;
                        if (turn.operations[0].enriched.next_ip_index != -1
                            || turn.operations[1].enriched.next_ip_index != -1)
                        {
                            out << " [" << turn.operations[0].enriched.next_ip_index
                                << "/"  << turn.operations[1].enriched.next_ip_index
                                << "]"
                                ;
                        }

                        offsets[p] += lineheight;
                        int offset = offsets[p];
                        mapper.text(turn.point, out.str(), style, margin, offset);
                    }


                    {
                        std::ostringstream out;
                        out << "vx:" << turn.operations[0].enriched.travels_to_vertex_index
                            << "/"  << turn.operations[1].enriched.travels_to_vertex_index;

                        offsets[p] += lineheight;
                        int offset = offsets[p];
                        mapper.text(turn.point, out.str(), style, margin, offset);
                    }

                    {
                        std::ostringstream out;
                        out << std::setprecision(3)
                            << "dist: " << turn.operations[0].enriched.distance
                            << " / "  << turn.operations[1].enriched.distance;

                        offsets[p] += lineheight;
                        int offset = offsets[p];
                        mapper.text(turn.point, out.str(), style, margin, offset);
                    }
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
        // distance-zero-2
        std::string dz[2] = {
            "POLYGON((24.587966918945313 61.027225494384766,32.1783447265625 62.988296508789063,34.655326843261719 55.550270080566406,41.730445861816406 52.173538208007812,38.846851348876953 44.883510589599609,42.096187591552734 37.748981475830078,35.201282501220703 34.018035888671875,33.104434967041016 26.46403694152832,25.42442512512207 28.037921905517578,18.962528228759766 23.599054336547852,14.090974807739258 29.741334915161133,6.2876262664794922 30.494592666625977,6.5039811134338379 38.331226348876953,1.0104535818099976 43.924152374267578,6.2134823799133301 49.788291931152344,5.6002583503723145 57.603889465332031,13.355405807495117 58.751640319824219,17.909420013427734 65.132911682128906,24.587966918945313 61.027225494384766))",
            "POLYGON((43.551433563232422 47.905071258544922,46.384872436523438 39.57366943359375,53.589195251464844 44.627212524414063,50.984420776367188 36.221515655517578,59.783241271972656 36.075325012207031,52.735191345214844 30.806018829345703,59.767654418945313 25.51593017578125,50.968441009521484 25.395713806152344,53.548389434814453 16.982362747192383,46.359016418457031 22.057153701782227,43.500991821289063 13.734155654907227,40.667552947998047 22.065553665161133,33.463230133056641 17.012012481689453,36.068000793457031 25.417709350585938,27.269184112548828 25.563901901245117,34.317234039306641 30.833206176757812,27.284770965576172 36.123294830322266,36.083980560302734 36.243511199951172,33.504035949707031 44.6568603515625,40.693408966064453 39.582073211669922,43.551433563232422 47.905071258544922))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_2",
                boost::make_tuple(2, 68.678921274288541), dz[0], dz[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_2",
                boost::make_tuple(2, 1505.4202304878663), dz[0], dz[1]);
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
        std::string hv[2] = {
            "POLYGON((24.995166778564453 50.011310577392578,46.630809783935547 37.494682312011719,46.661380767822266 12.499360084533691,25.003841400146484 0.020658308640122414,3.3419711589813232 12.491842269897461,3.3638687133789062 37.487174987792969,24.995166778564453 50.011310577392578))",
            "POLYGON((25.025228500366211 49.992599487304688,46.6719970703125 37.482185363769531,46.631874084472656 12.480358123779297,24.974153518676758 -0.011088892817497253,3.3419976234436035 12.524576187133789,3.3529467582702637 37.526435852050781,25.025228500366211 49.992599487304688))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv1", boost::make_tuple(1, 1624.508688461573), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv1", boost::make_tuple(1, 1622.7200125123809), hv[0], hv[1], 0.1);
    }
    {
        std::string hv[2] = {
            "POLYGON((24.988700866699219 49.986705780029297,46.643772125244141 37.5079345703125,46.645118713378906 12.514699935913086,25.010652542114258 0.00024537215358577669,3.3652000427246094 12.495694160461426,3.3445985317230225 37.488922119140625,24.988700866699219 49.986705780029297))",
            "POLYGON((24.993022918701172 49.977996826171875,46.643772125244141 37.503200531005859,46.634654998779297 12.51569938659668,25.005790710449219 0.0029967525042593479,3.3705389499664307 12.504646301269531,3.348651647567749 37.492141723632812,24.993022918701172 49.977996826171875))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv2", boost::make_tuple(1, 1622.9193392726836), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv2", boost::make_tuple(1, 1622.1733591429329), hv[0], hv[1], 0.1);
    }
    {
        std::string hv[2] = {
            "POLYGON((25.007728576660156 49.988899230957031,46.667163848876953 37.501667022705078,46.637229919433594 12.500443458557129,24.993251800537109 -0.01356174610555172,3.3565254211425781 12.512973785400391,3.3410670757293701 37.514209747314453,25.007728576660156 49.988899230957031))",
            "POLYGON((24.998353958129883 49.993511199951172,46.659591674804688 37.507373809814453,46.646518707275391 12.505118370056152,25.002584457397461 -0.0109936548396945,3.3565335273742676 12.501456260681152,3.3392288684844971 37.503707885742188,24.998353958129883 49.993511199951172))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv3", boost::make_tuple(1, 1624.22079205664), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv3", boost::make_tuple(1, 1623.8265057282042), hv[0], hv[1], 0.1);
    }
    {
        std::string hv[2] = {
            "POLYGON((25.009130477905273 50.022209167480469,46.670387268066406 37.500617980957031,46.666873931884766 12.480625152587891,24.992231369018555 -0.017777863889932632,3.3260366916656494 12.495262145996094,3.3394229412078857 37.515254974365234,25.009130477905273 50.022209167480469))",
            "POLYGON((25.00263786315918 50.019630432128906,46.669231414794922 37.507579803466797,46.666202545166016 12.487733840942383,24.997152328491211 -0.020060751587152481,3.3308455944061279 12.492485046386719,3.3333024978637695 37.5123291015625,25.00263786315918 50.019630432128906))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv4", boost::make_tuple(1, 1626.5146964146334), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv4", boost::make_tuple(1, 1626.2580370864305), hv[0], hv[1], 0.1);
    }
    {
        std::string hv[2] = {
            "POLYGON((24.987522125244141 49.997768402099609,46.643741607666016 37.509471893310547,46.654956817626953 12.510490417480469,25.011669158935547 -0.00019846600480377674,3.3563058376312256 12.489578247070313,3.3433761596679687 37.488559722900391,24.987522125244141 49.997768402099609))",
            "POLYGON((25.005760192871094 50.008182525634766,46.648590087890625 37.491542816162109,46.655918121337891 12.489977836608887,24.994773864746094 0.0050580352544784546,3.3391191959381104 12.499494552612305,3.3574333190917969 37.501052856445312,25.005760192871094 50.008182525634766))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv5", boost::make_tuple(1, 1624.2158307261871), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv5", boost::make_tuple(1, 1623.4506071521519), hv[0], hv[1], 0.1);
    }


    // Case 2009-12-07
    {
        std::string hv[2] = {
            "POLYGON((25.011470794677734 50.017532348632813,42.678981781005859 42.661365509033203,50.017532348632813 24.986530303955078,42.661365509033203 7.3190178871154785,24.986530303955078 -0.019533095881342888,7.3190178871154785 7.336634635925293,-0.019533095881342888 25.011470794677734,7.336634635925293 42.678981781005859,25.011470794677734 50.017532348632813))",
            "POLYGON((25.002880096435547 50.013965606689453,46.671913146972656 37.507381439208984,46.660655975341797 12.488155364990234,24.9951171875 -0.024483053013682365,3.3334629535675049 12.494877815246582,3.3299689292907715 37.514102935791016,25.002880096435547 50.013965606689453))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv6", boost::make_tuple(1, 1604.6318757402121), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv6", boost::make_tuple(1, 1790.091872401327), hv[0], hv[1], 0.1);
    }

    // Case 2009-12-08, needing sorting on side in enrich_intersection_points
    {
        std::string hv[2] = {
            "POLYGON((24.983684539794922 49.995647430419922,46.643482208251953 37.513137817382813,46.654392242431641 12.51393985748291,25.014318466186523 -0.0027416276279836893,3.3589246273040771 12.487411499023438,3.3391971588134766 37.486602783203125,24.983684539794922 49.995647430419922))",
            "POLYGON((24.990163803100586 49.9993896484375,46.655281066894531 37.512466430664062,46.654388427734375 12.506458282470703,25.007841110229492 -0.012621366418898106,3.3524465560913086 12.491152763366699,3.3338801860809326 37.497154235839844,24.990163803100586 49.9993896484375))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("hv7", boost::make_tuple(1, 1624.5779453641017), hv[0], hv[1], 0.1);
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("hv7", boost::make_tuple(1, 1623.6936420295772), hv[0], hv[1], 0.1);
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
        std::string dz[2] = {
            "POLYGON((30.526203155517578 56.781166076660156,38.987510681152344 58.710700988769531,41.042613983154297 50.279010772705078,48.390048980712891 45.660350799560547,43.881126403808594 38.245067596435547,45.810657501220703 29.783760070800781,37.378971099853516 27.728654861450195,32.760307312011719 20.381219863891602,25.345026016235352 24.890144348144531,16.883718490600586 22.960611343383789,14.828612327575684 31.392299652099609,7.481177806854248 36.010959625244141,11.990103721618652 43.426242828369141,10.060568809509277 51.887550354003906,18.492258071899414 53.942657470703125,23.110919952392578 61.290092468261719,30.526203155517578 56.781166076660156))",
            "POLYGON((12.580197334289551 33.274467468261719,14.852641105651855 24.577714920043945,21.524574279785156 30.601236343383789,18.734457015991211 22.056488037109375,27.603805541992188 23.51667594909668,20.636968612670898 17.836828231811523,28.887777328491211 14.270085334777832,19.956142425537109 13.258448600769043,24.968837738037109 5.7971897125244141,16.908138275146484 9.7749528884887695,17.091224670410156 0.78807485103607178,12.460672378540039 8.4923257827758789,7.7560214996337891 0.83309894800186157,8.0257854461669922 9.8177928924560547,-0.072908863425254822 5.9179673194885254,5.0115232467651367 13.330527305603027,-3.9099369049072266 14.42827033996582,4.3748917579650879 17.915260314941406,-2.5368332862854004 23.662046432495117,6.3180174827575684 22.116373062133789,3.6104514598846436 30.687637329101563,10.223971366882324 24.600040435791016,12.580197334289551 33.274467468261719))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_1",
                boost::make_tuple(3, 16.887537949472005), dz[0], dz[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_1",
                boost::make_tuple(3, 1444.2621305732864), dz[0], dz[1]);
    }
    {
        // distance-zero-2
        std::string dz[2] = {
            "POLYGON((24.587966918945313 61.027225494384766,32.1783447265625 62.988296508789063,34.655326843261719 55.550270080566406,41.730445861816406 52.173538208007812,38.846851348876953 44.883510589599609,42.096187591552734 37.748981475830078,35.201282501220703 34.018035888671875,33.104434967041016 26.46403694152832,25.42442512512207 28.037921905517578,18.962528228759766 23.599054336547852,14.090974807739258 29.741334915161133,6.2876262664794922 30.494592666625977,6.5039811134338379 38.331226348876953,1.0104535818099976 43.924152374267578,6.2134823799133301 49.788291931152344,5.6002583503723145 57.603889465332031,13.355405807495117 58.751640319824219,17.909420013427734 65.132911682128906,24.587966918945313 61.027225494384766))",
            "POLYGON((43.551433563232422 47.905071258544922,46.384872436523438 39.57366943359375,53.589195251464844 44.627212524414063,50.984420776367188 36.221515655517578,59.783241271972656 36.075325012207031,52.735191345214844 30.806018829345703,59.767654418945313 25.51593017578125,50.968441009521484 25.395713806152344,53.548389434814453 16.982362747192383,46.359016418457031 22.057153701782227,43.500991821289063 13.734155654907227,40.667552947998047 22.065553665161133,33.463230133056641 17.012012481689453,36.068000793457031 25.417709350585938,27.269184112548828 25.563901901245117,34.317234039306641 30.833206176757812,27.284770965576172 36.123294830322266,36.083980560302734 36.243511199951172,33.504035949707031 44.6568603515625,40.693408966064453 39.582073211669922,43.551433563232422 47.905071258544922))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_2",
                boost::make_tuple(2, 68.678921274288541), dz[0], dz[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_2",
                boost::make_tuple(2, 1505.4202304878663), dz[0], dz[1]);
    }

    {
        // distance-zero-3
        std::string dz[2] = {
            "POLYGON((20.813335418701172 73.060707092285156,22.815366744995117 61.968788146972656,31.383756637573242 69.291458129882813,28.001794815063477 58.539661407470703,38.991741180419922 61.041633605957031,31.000555038452148 53.093067169189453,41.894393920898437 50.201171875,31.124666213989258 46.876754760742188,39.426750183105469 39.253490447998047,28.345697402954102 41.314804077148438,32.154121398925781 30.706569671630859,23.300275802612305 37.681396484375,21.742572784423828 26.518407821655273,17.144247055053711 36.808895111083984,10.5772705078125 27.648460388183594,11.287883758544922 38.897186279296875,1.2160475254058838 33.837848663330078,7.0728073120117187 43.467861175537109,-4.1965517997741699 43.668655395507812,5.4646410942077637 49.473834991455078,-4.4205660820007324 54.888763427734375,6.8317971229553223 55.539215087890625,0.59532338380813599 64.927780151367187,10.861076354980469 60.274494171142578,9.7020368576049805 71.485885620117188,16.629419326782227 62.594875335693359,20.813335418701172 73.060707092285156))",
            "POLYGON((1.6459450721740723 46.720386505126953,10.693820953369141 61.892372131347656,7.2385158538818359 44.568569183349609,23.921955108642578 50.3751220703125,10.139513969421387 39.325347900390625,26.652151107788086 33.049518585205078,8.9915294647216797 33.444084167480469,17.606916427612305 18.02239990234375,4.3317174911499023 29.676681518554687,1.0186206102371216 12.32512378692627,-1.6595441102981567 29.785955429077148,-15.35089111328125 18.623508453369141,-6.1788778305053711 33.720771789550781,-23.842140197753906 33.970470428466797,-7.1116366386413574 39.639987945556641,-20.481979370117188 51.184993743896484,-4.0213727951049805 44.773937225341797,-6.8426628112792969 62.212215423583984,1.6459450721740723 46.720386505126953))"
        };
        test_overlay<polygon, polygon, test_traverse<operation_intersection>,  Tuple>("dz_3",
                boost::make_tuple(6, 192.49316937645651), dz[0], dz[1]);
        test_overlay<polygon, polygon, test_traverse<operation_union>,  Tuple>("dz_3",
                boost::make_tuple(6, 1446.496005965641), dz[0], dz[1]);
    }
    return;


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
