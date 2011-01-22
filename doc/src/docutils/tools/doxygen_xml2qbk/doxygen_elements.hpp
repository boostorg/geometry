// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//
#ifndef DOXYGEN_ELEMENTS_HPP
#define DOXYGEN_ELEMENTS_HPP


#include <string>
#include <vector>


// contains (template)parameter
struct param
{
    std::string name;
    std::string description;
    std::string type;
    std::string fulltype; // post-processed
};

struct markup
{
    int code;
    std::string value;

    markup(int c = 0, std::string const& v = "")
        : code(c)
        , value(v)
    {}

};

// Basic element, base of a class/struct, function, define
struct element
{
    std::string name;
    std::string brief_description, detailed_description;
    std::string location;
    int line; // To sort - Doxygen changes order - we change it back

    // QBK-includes
    // Filled with e.g.: \qbk([include reference/myqbk.qbk]}
    std::vector<markup> qbk_markup;

    // To distinguish overloads: unary, binary etc,
    // Filled with: \qbk{distinguish,<A discerning description>}
    std::string additional_description;

    std::vector<param> template_parameters;
    std::vector<param> parameters;

    element()
        : line(0)
    {}
};

enum function_type { function_unknown, function_define, function_constructor, function_member, function_free };

struct function : public element
{
    function_type type;
    std::string definition, argsstring;
    std::string return_type, return_description;

    bool unique;

    function()
        : type(function_unknown)
        , unique(true)
    {}

};

struct class_or_struct : public element
{
    std::string name, fullname;
    std::string model_of;
    std::vector<function> functions;
};


struct documentation
{
    class_or_struct cos;
    std::vector<function> functions;
    std::vector<function> defines;
};


#endif // DOXYGEN_ELEMENTS_HPP
