// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//
#ifndef FILE_TO_STRING_HPP
#define FILE_TO_STRING_HPP


#include <string>
#include <fstream>

#include <boost/algorithm/string.hpp>


inline std::string file_to_string(std::string const& filename, bool add_nl = false)
{
    std::string result;

    std::ifstream cpp_file(filename.c_str());
    if (cpp_file.is_open())
    {
        while (! cpp_file.eof() )
        {
            std::string line;
            std::getline(cpp_file, line);
            boost::trim(line);

            result += line;

            if (add_nl)
            {
                result += "\n";
            }
        }
    }
    return result;
}


#endif // FILE_TO_STRING_HPP
