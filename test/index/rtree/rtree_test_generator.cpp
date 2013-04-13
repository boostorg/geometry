// Boost.Geometry Index
// Rtree tests generator

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fstream>
#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

int main()
{
    std::vector<std::string> generated_files;

    typedef boost::tuple<std::string, std::string, std::string> CT;
    std::vector<CT> coordinate_types;
    coordinate_types.push_back(boost::make_tuple("double", "d", ""));
    coordinate_types.push_back(boost::make_tuple("int", "i", ""));
    //coordinate_types.push_back(boost::make_tuple("float", "f", ""));
    //coordinate_types.push_back(boost::make_tuple("ttmath_big", "tt", "HAVE_TTMATH"));

    std::vector<std::string> dimensions;
    dimensions.push_back("2");
    dimensions.push_back("3");

    typedef boost::tuple<std::string, std::string> P;
    std::vector<P> parameters;
    parameters.push_back(boost::make_tuple("bgi::linear<8, 3>()", "lin"));
    parameters.push_back(boost::make_tuple("bgi::quadratic<8, 3>()", "qua"));
    parameters.push_back(boost::make_tuple("bgi::rstar<8, 3>()", "rst"));
    parameters.push_back(boost::make_tuple("bgi::dynamic_linear(8, 3)", "dlin"));
    parameters.push_back(boost::make_tuple("bgi::dynamic_quadratic(8, 3)", "dqua"));
    parameters.push_back(boost::make_tuple("bgi::dynamic_rstar(8, 3)","drst"));
    
    std::vector<std::string> indexables;
    indexables.push_back("p");
    indexables.push_back("b");

    typedef std::pair<std::string, std::string> TS;
    std::vector<TS> testsets;
    testsets.push_back(std::make_pair("testset::modifiers<Indexable>(parameters, std::allocator<int>())", "mod"));
    testsets.push_back(std::make_pair("testset::queries<Indexable>(parameters, std::allocator<int>())", "que"));
    testsets.push_back(std::make_pair("testset::additional<Indexable>(parameters, std::allocator<int>())", "add"));

    BOOST_FOREACH(std::string const& d, dimensions)
    {
        BOOST_FOREACH(CT const& c, coordinate_types)   
        {
            BOOST_FOREACH(P const& p, parameters)
            {
                BOOST_FOREACH(std::string const& i, indexables)
                {
                    BOOST_FOREACH(TS const& ts, testsets)
                    {
                        std::string point_type = std::string() +
                            "bg::model::point<" + boost::get<0>(c) + ", " + d + ", bg::cs::cartesian>";
                        std::string box_type = std::string("bg::model::box<") + point_type + ">";

                        std::string filename = std::string() +
                            "rtree_" + i + d + boost::get<1>(c) + '_' + boost::get<1>(p) + "_" + ts.first + ".cpp";

                        std::ofstream f(filename.c_str(), std::ios::trunc);

                        f <<
                            "// Boost.Geometry Index\n" <<
                            "// Unit Test\n" <<
                            "\n" <<
                            "// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.\n" <<
                            "\n" <<
                            "// Use, modification and distribution is subject to the Boost Software License,\n" <<
                            "// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at\n" <<
                            "// http://www.boost.org/LICENSE_1_0.txt)\n" <<
                            "\n";

                        f <<
                            "#include <rtree/test_rtree.hpp>\n" <<
                            "\n" <<
                            "#include <boost/geometry/geometries/point.hpp>\n" <<
                            (i == "p" ? "" : "#include <boost/geometry/geometries/box.hpp>\n") <<
                            "\n";

                        f <<
                            "int test_main(int, char* [])\n" <<
                            "{\n" <<
                            (boost::get<2>(c).empty() ? "" : "#ifdef HAVE_TTMATH\n") <<
                            "    typedef " << point_type << " Point;\n" <<
                            "    " <<
                                (i == "p" ? "test_rtree_for_point" : "test_rtree_for_box" ) <<
                                    "<Point>(" << boost::get<0>(p) << ");\n" <<
                            (boost::get<2>(c).empty() ? "" : "#endif\n") <<
                            "    return 0;\n" <<
                            "}\n";

                        generated_files.push_back(filename);
                    }
                }
            }           

        }
    }

    std::ofstream f("Jamfile.v2", std::ios::trunc);

    f <<
        "# Boost.Geometry Index\n" <<
        "#\n" <<
        "# Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.\n" <<
        "#\n" <<
        "# Use, modification and distribution is subject to the Boost Software License,\n" <<
        "# Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at\n" <<
        "# http://www.boost.org/LICENSE_1_0.txt)\n" <<
        "\n" <<
        "test-suite boost-geometry-index-rtree-generated\n" <<
        "    :\n";

    BOOST_FOREACH(std::string const& s, generated_files)
    {
        f <<
            "    [ run " << s << " ]\n";
    }

    f <<
        "    ;\n" <<
        "\n";

    return 0;
}
