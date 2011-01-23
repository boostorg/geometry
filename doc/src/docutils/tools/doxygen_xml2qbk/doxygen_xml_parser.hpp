// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//
#ifndef DOXYGEN_XML_PARSER_HPP
#define DOXYGEN_XML_PARSER_HPP


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <rapidxml_util.hpp>
#include <doxygen_elements.hpp>
#include <parameter_predicates.hpp>
#include <configuration.hpp>


inline std::string keep_after(std::string const& input, std::string const& sig)
{
    std::size_t pos = input.rfind(sig);
    if (pos != std::string::npos)
    {
        std::string copy = input.substr(pos + sig.length());
        return copy;
    }
    return input;
}



static inline void add_or_set(std::vector<parameter>& parameters, parameter const& p)
{
    std::vector<parameter>::iterator it = std::find_if(parameters.begin(), parameters.end(), par_by_name(p.name));
    if (it != parameters.end())
    {
        if (it->description.empty()) it->description = p.description;
        if (it->type.empty()) it->type = p.type;
        if (it->fulltype.empty()) it->fulltype = p.fulltype;
    }
    else
    {
        parameters.push_back(p);
    }
}

/// Parses a "para" element 
/*
This is used for different purposes within Doxygen. 
- Either a detailed description, possibly containing several sections (para's)
  -> so parse next siblings
- Or a detailed description also containing qbk records

So we have to list explicitly either where to recurse, or where not to...

*/
static void parse_para(rapidxml::xml_node<>* node, std::string& contents, bool first = true)
{
    if (node != NULL)
    {
        if (node->type() == rapidxml::node_element)
        {
            //std::cout << "ELEMENT: " << node->name() << "=" << node->value() << std::endl;
            std::string name = node->name();
            if (! (
                (boost::equals(name, "para") && first)
                || boost::equals(name, "ref")
                || boost::equals(name, "defval")
                || boost::equals(name, "verbatim")
                || boost::equals(name, "bold")
                || boost::equals(name, "emphasis")
                || boost::equals(name, "linebreak")
                ))
            {
                return;
            }
        }
        else if (node->type() == rapidxml::node_data)
        {
            contents += node->value();
            //std::cout << "DATA: " << node->name() << "=" << node->value() << std::endl;
        }
        else
        {
            //std::cout << "OTHER: " << node->name() << "=" << node->value() << std::endl;
        }
        parse_para(node->first_node(), contents, false);
        parse_para(node->next_sibling(), contents, false);
    }
}


static void parse_parameter(rapidxml::xml_node<>* node, parameter& p)
{
    // #define: <param><defname>Point</defname></param>
    // template: <param><type>typename</type><declname>CoordinateType</declname><defname>CoordinateType</defname></param>
    // template with default: <param><type>typename</type><declname>CoordinateSystem</declname><defname>CoordinateSystem</defname><defval><ref ....>cs::cartesian</ref></defval></param>
    if (node != NULL)
    {
        std::string name = node->name();
        if (name == "type")
        {
            p.fulltype = node->value();
            p.type = p.fulltype;
            boost::replace_all(p.type, " const", "");
            boost::trim(p.type);
            boost::replace_all(p.type, "&", "");
            boost::replace_all(p.type, "*", "");
            boost::trim(p.type);
        }
        else if (name == "declname") p.name = node->value();
        else if (name == "parametername") p.name = node->value();
        else if (name == "defname") p.name = node->value(); 
        else if (name == "defval") 
        {
             parse_para(node, p.default_value);
        }
        else if (name == "para")
        {
             parse_para(node, p.description);
        }

        parse_parameter(node->first_node(), p);
        parse_parameter(node->next_sibling(), p);
    }
}

// Definition is a function or a class/struct
template <typename Parameters>
static void parse_parameter_list(rapidxml::xml_node<>* node, Parameters& parameters)
{
    if (node != NULL)
    {
        std::string name = node->name();

        if (name == "parameteritem")
        {
            parameter p;
            parse_parameter(node->first_node(), p);
            if (! p.name.empty())
            {
                // Copy its description
                std::vector<parameter>::iterator it = std::find_if(parameters.begin(),
                    parameters.end(), par_by_name(p.name));
                if (it != parameters.end())
                {
                    it->description = p.description;
                }
                else
                {
                    parameters.push_back(p);
                }
            }
        }
        else if (name == "param")
        {
            // Element of 'templateparamlist.param (.type,.declname,.defname)'
            parameter p;
            parse_parameter(node->first_node(), p);

            // Doxygen handles templateparamlist param's differently:
            //
            // Case 1:
            // <param><type>typename T</type></param>
            // -> no name, assign type to name, replace typename
            //
            // Case 2:
            // <type>typename</type><declname>T</declname><defname>T</defname>
            // -> set full type
            if (p.name.empty())
            {
                // Case 1
                p.name = p.type;
                boost::replace_all(p.name, "typename", "");
                boost::trim(p.name);
            }
            else
            {
                // Case 2
                p.fulltype = p.type + " " + p.name;
            }

            add_or_set(parameters, p);
        }

        parse_parameter_list(node->first_node(), parameters);
        parse_parameter_list(node->next_sibling(), parameters);
    }
}


template <typename Element>
static void parse_element(rapidxml::xml_node<>* node, configuration const& config, std::string const& parent, Element& el)
{
    if (node != NULL)
    {
        std::string name = node->name();
        std::string full = parent + "." + name;

        if (full == ".briefdescription.para")
        {
            parse_para(node, el.brief_description);
        }
        else if (full == ".detaileddescription.para")
        {
            std::string para;
            parse_para(node, para);
            if (!para.empty() && !el.detailed_description.empty())
            {
                el.detailed_description += "\n\n";
            }
            el.detailed_description += para;
        }
        else if (full == ".location")
        {
            std::string loc = get_attribute(node, "file");
            // Location of (header)file. It is a FULL path, so find the start
            // and strip the rest
            std::size_t pos = loc.rfind(config.start_include);
            if (pos != std::string::npos)
            {
                loc = loc.substr(pos);
            }
            el.location = loc;
            el.line = atol(get_attribute(node, "line").c_str());
        }
        else if (full == ".detaileddescription.para.qbk")
        {
            el.qbk_markup.push_back(markup(node->value()));
        }
        else if (full == ".detaileddescription.para.qbk.after.synopsis")
        {
            el.qbk_markup.push_back(markup(markup_after, markup_synopsis, node->value()));
        }
        else if (full == ".detaileddescription.para.qbk.before.synopsis")
        {
            el.qbk_markup.push_back(markup(markup_before, markup_synopsis, node->value()));
        }
        else if (full == ".detaileddescription.para.qbk.distinguish")
        {
            el.additional_description = node->value();
            boost::trim(el.additional_description);
        }
        else if (full == ".templateparamlist")
        {
            parse_parameter_list(node->first_node(), el.template_parameters);
        }
        else if (full == ".detaileddescription.para.parameterlist")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "param")
            {
                parse_parameter_list(node->first_node(), el.parameters);
            }
            else if (kind == "templateparam")
            {
                parse_parameter_list(node->first_node(), el.template_parameters);
            }
        }

        parse_element(node->first_node(), config, full, el);
        parse_element(node->next_sibling(), config, parent, el);
    }
}

static void parse_function(rapidxml::xml_node<>* node, configuration const& config, std::string const& parent, function& f)
{
    if (node != NULL)
    {
        std::string name = node->name();
        std::string full = parent + "." + name;

        if (full == ".name") f.name = node->value();
        else if (full == ".argsstring") f.argsstring = node->value();
        else if (full == ".definition")
        {
            f.definition = node->value();
            if (! config.skip_namespace.empty())
            {
                boost::replace_all(f.definition, config.skip_namespace, "");
            }
        }
        else if (full == ".param")
        {
            parameter p;
            parse_parameter(node->first_node(), p);
            add_or_set(f.parameters, p);
        }
        else if (full == ".type")
        {
            get_contents(node->first_node(), f.return_type);
        }
        else if (full == ".detaileddescription.para.simplesect")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "return")
            {
                get_contents(node->first_node(), f.return_description);
            }
            /*else if (kind == "param")
            {
                get_contents(node->first_node(), f.paragraphs);
            }*/
        }
        else if (full == ".detaileddescription.para.image")
        {
        }

        parse_function(node->first_node(), config, full, f);
        parse_function(node->next_sibling(), config, parent, f);
    }
}

static void parse(rapidxml::xml_node<>* node, configuration const& config, documentation& doc, bool member = false)
{
    if (node != NULL)
    {
        bool recurse = false;
        bool is_member = member;

        std::string nodename = node->name();

        if (nodename == "doxygen")
        {
            recurse = true;
        }
        else if (nodename == "sectiondef")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "func" )
            {
                // Get free function definition
                recurse = true;
            }
            if (kind == "define" )
            {
                // Get define or registration macro
                recurse = true;
            }
            else if (kind == "public-static-func" || kind == "public-func")
            {
                recurse = true;
                is_member = true;
            }
        }
        else if (nodename == "compounddef")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "group")
            {
                recurse = true;
            }
            if (kind == "struct" || kind == "class")
            {
                recurse = true;
                parse_element(node->first_node(), config, "", doc.cos);
            }
        }
        else if (nodename == "memberdef")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "function" || kind == "define")
            {
                function f;
                parse_element(node->first_node(), config, "", f);
                parse_function(node->first_node(), config, "", f);
                if (kind == "define")
                {
                    f.type = function_define;
                    doc.defines.push_back(f);
                }
                else if (member)
                {
                    f.type = f.name == doc.cos.name ? function_constructor : function_member;
                    doc.cos.functions.push_back(f);
                }
                else
                {
                    f.type = function_free;
                    doc.functions.push_back(f);
                }
            }
        }
        else if (nodename == "compoundname")
        {
            std::string name = node->value();
            if (name.find("::") != std::string::npos)
            {
                doc.cos.fullname = name;

                // For a class, it should have "boost::something::" before
                // set its name without namespace
                doc.cos.name = keep_after(name, "::");
            }
        }
        else
        {
            //std::cout << nodename << " ignored." << std::endl;
        }


        if (recurse)
        {
            // First recurse into childnodes, then handle next siblings
            parse(node->first_node(), config, doc, is_member);
        }
        parse(node->next_sibling(), config, doc, is_member);
    }
}

#endif // DOXYGEN_XML_PARSER_HPP
