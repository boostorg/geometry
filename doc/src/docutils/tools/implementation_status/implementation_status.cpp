// implementation_status (developed in the context of Boost.Geometry documentation)
//
// Copyright Barend Gehrels 2010, 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <stdlib.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

static const int point = 0;
static const int segment = 1;
static const int box = 2;
static const int linestring = 3;
static const int ring = 4;
static const int polygon = 5;
static const int multi_point = 6;
static const int multi_linestring = 7;
static const int multi_polygon = 8;
static const int geometry_count = 9;


struct algorithm
{
    std::string name;
    int arity;

    explicit algorithm(std::string const& n, int a = 1)
        : name(n)
        , arity(a)
    {}
};


inline std::string bool_string(bool v)
{
    return v ? "true" : "false";
}

inline std::string typedef_string(int type, bool clockwise, bool open)
{
    std::ostringstream out;
    switch(type)
    {
        case point : return "P";
        case linestring : return "bg::model::linestring<P>";
        case box : return "bg::model::box<P>";
        case segment : return "bg::model::segment<P>";
        case ring :
            out << "bg::model::ring<P, "
                << bool_string(clockwise) << ", " << bool_string(open) << ">";
            break;
        case polygon : 
            out << "bg::model::polygon<P, "
                << bool_string(clockwise) << ", " << bool_string(open) << ">";
            break;
        case multi_point : return "bg::model::multi_point<P>";
        case multi_linestring : 
            out << "bg::model::multi_linestring<bg::model::linestring<P> >";
            break;
        case multi_polygon : 
            out << "bg::model::multi_polygon<bg::model::polygon<P, "
                << bool_string(clockwise) << ", " << bool_string(open) << "> >";
            break;
    }
    return out.str();
}

inline std::string wkt_string(int type)
{
    switch(type)
    {
        case point : return "POINT(1 1)";
        case linestring : return "LINESTRING(1 1,2 2)";
        case segment : return "LINESTRING(1 1,2 2)";
        case box : return "POLYGON((1 1,2 2)";
        case polygon : 
        case ring :
            return "POLYGON((0 0,0 1,1 1,0 0))";
        case multi_point : return "MULTIPOINT((1 1),(2 2))";
        case multi_linestring : return "MULTILINESTRING((1 1,2 2))";
        case multi_polygon : return "MULTIPOLYGON(((0 0,0 1,1 1,0 0)))";
    }
    return "";
}

inline std::string geometry_string(int type)
{
    switch(type)
    {
        case point : return "Point";
        case linestring : return "Linestring";
        case box : return "Box";
        case polygon : return "Polygon"; 
        case ring : return "Ring";
        case segment : return "Segment";
        case multi_point : return "MultiPoint";
        case multi_linestring : return "MultiLinestring";
        case multi_polygon : return "MultiPolygon";
    }
    return "";
}



int report_library(int type, algorithm const& algo, bool clockwise,
                   bool open, int dimensions, std::string const& cs,
                   int type2 = -1)
{
    std::string lit;
    {
        std::ostringstream out;
        out << geometry_string(type);
        if (type2 != -1)
        {
            out << "_" << geometry_string(type2);
        }
        out 
            << "_" << algo.name
            << "_" << bool_string(clockwise) 
            << "_" << bool_string(open)
            << "_" << boost::replace_all_copy
                        (
                            boost::replace_all_copy
                                (
                                    boost::replace_all_copy(cs, "bg::", "")
                                , "<", "_"
                                )
                            , ">", "_"
                        );
        lit = out.str();
    }

    std::cout << lit;

    {
        std::ofstream out("tmp/t.cpp");

        out
            << "#include <implementation_status.hpp>" << std::endl
            << "template <typename P>" << std::endl
            << "inline void test()" << std::endl
            << "{" << std::endl
            << "  namespace bg = boost::geometry;" << std::endl
            << "  " << typedef_string(type, clockwise, open) << " geometry;" << std::endl
            << "  bg::read_wkt(\"" << wkt_string(type) << "\", geometry);" << std::endl;

        
        if (algo.arity > 1)
        {
            out 
                << "  " << typedef_string(type2, clockwise, open) << " geometry2;" << std::endl
                << "  bg::read_wkt(\"" << wkt_string(type2) << "\", geometry2);" << std::endl;
        }

        switch(algo.arity)
        {
            case 1 : 
                out << "  bg::" << algo.name << "(geometry);" << std::endl; 
                break;
            case 2 : 
                out << "  bg::" << algo.name << "(geometry, geometry2);" << std::endl; 
                break;
        }

        out
            << "}" << std::endl
            << std::endl
            ;

        out
            << "int main()" << std::endl
            << "{" << std::endl
            << "  namespace bg = boost::geometry;" << std::endl
            << "  test<bg::model::point< double, " << dimensions << ", bg::cs::" << cs << " > >();" << std::endl
            << "  return 0;" << std::endl
            << "}" << std::endl
            << std::endl
            ;
    }

    int failed = system("bjam -a tmp > tmp/t.out");
    if (failed)
    {
        std::cout << " ERROR";
    }

    std::cout << std::endl;

    return !failed;
}


std::vector<int> report(int type, algorithm const& algo, bool clockwise,
                        bool open, int dimensions, std::string const& cs)
{
    std::vector<int> result;

    switch(algo.arity)
    {
        case 1 :
            result.push_back(report_library(type, algo, clockwise, open, dimensions, cs));
            break;
        case 2 :
            for (int type2 = point; type2 < geometry_count; ++type2)
            {
                result.push_back(report_library(type, algo, clockwise, open, dimensions, cs, type2));
            }
            break;
    }

    return result;
}


struct cs
{
    std::string name;

    cs(std::string const& n)
        : name(n)
    {}
};


int main(int argc, char** argv)
{
    typedef std::vector<algorithm> v_a_type;
    v_a_type algorithms;
    algorithms.push_back(algorithm("area"));
    algorithms.push_back(algorithm("length"));
    //algorithms.push_back(algorithm("perimeter"));
    //algorithms.push_back(algorithm("correct"));
    algorithms.push_back(algorithm("distance", 2));
    //algorithms.push_back(algorithm("centroid", 2));

    typedef std::vector<cs> cs_type;
    cs_type css;
    css.push_back(cs("cartesian"));
    // css.push_back(cs("spherical<bg::degree>"));
    // css.push_back(cs("spherical<bg::radian>"));

    for (v_a_type::const_iterator it = algorithms.begin(); it != algorithms.end(); ++it)
    {
/*([heading Behavior]
[table
[[Case] [Behavior] ]
[[__2dim__][All combinations of: box, ring, polygon, multi_polygon]]
[[__other__][__nyiversion__]]
[[__sph__][__nyiversion__]]
[[Three dimensional][__nyiversion__]]
]*/

        std::ostringstream name;
        name << "../../../../generated/" << it->name << "_status.qbk";

        std::ofstream out(name.str().c_str());
        out << "[heading Supported geometries]" << std::endl;

        cs_type::const_iterator cit = css.begin();

        {
            // Construct the table

            std::vector<std::vector<int> > table;

            for (int type = point; type < geometry_count; type++)
            {
                table.push_back(report(type, *it, true, true, 2, cit->name));
            }


            // Detect red rows/columns

            std::vector<int> lines_status(table.size(), false);
            std::vector<int> columns_status(table[0].size(), false);

            for (unsigned int i = 0; i != table.size(); ++i)
            {
                for (unsigned int j = 0; j != table[i].size(); ++j)
                {
                    lines_status[i] |= table[i][j];
                    columns_status[j] |= table[i][j];
                }
            }


            // Display the table

            out << "[table" << std::endl << "[";

            if (it->arity > 1)
            {
                out << "[ ]";
                for (int type = point; type < geometry_count; type++)
                {
                    if (!columns_status[type]) continue;
                    out << "[[" << geometry_string(type) << "]]";
                }
            }
            else
            {
                out << "[Geometry][Status]";
            }

            out << "]" << std::endl;

            for (unsigned int i = 0; i != table.size(); ++i)
            {
                if (!lines_status[i]) continue;
                out << "[[" << geometry_string(i) << "]]";
                for (unsigned int j = 0; j != table[i].size(); ++j)
                {
                    if (!columns_status[j]) continue;
                    out << "[[$img/" << (table[i][j] ? "ok" : "nyi") << ".png]]";
                }
                out << std::endl;
            }

            out << "]" << std::endl;
        }
    }

    return 0;
}
