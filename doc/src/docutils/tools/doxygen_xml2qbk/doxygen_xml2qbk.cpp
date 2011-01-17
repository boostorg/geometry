// doxygen_xml2qbk (developed in the context of Boost.Geometry documentation)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//
// Barend Gehrels, Aug 1, 2010
// In continuation of the QuickBook documentation of Boost.Geometry
//
// Converts XML files created by Doxygen to Quickbook
// Notes:
// - basically generic, but implemented with Boost.Geometry in mind
// - makes use of some specific XML elements, which can be created by Doxygen
//       using /xmlonly
//     currently this is the element <qbk.example> which will make a reference
//     to an example.
// - currently still in draft

// Aug 14/15: added classes, defines, various enhancements.

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>


#include <rapidxml.hpp>

#include <file_to_string.hpp>

// -------------------------------------------------------------
// rapid xml convenience code
// -------------------------------------------------------------
class xml_doc : public rapidxml::xml_document<>
{
    public :
        xml_doc(const char* xml)
        {
            // Make a copy because rapidxml destructs string
            m_copy = new char[strlen(xml) + 1];
            strcpy(m_copy, xml);
            this->parse<0>(m_copy);
        };
        virtual ~xml_doc()
        {
            delete[] m_copy;
        }
    private :
        char* m_copy;

};

inline std::string get_attribute(rapidxml::xml_node<>* node, const char* name)
{
    rapidxml::xml_attribute<> *attr = node->first_attribute(name);
    std::string value;
    if (attr)
    {
        value = attr->value();
    }
    return value;
}

inline void get_contents(rapidxml::xml_node<>* node, std::string& contents)
{
    if (node != NULL && node->type() == rapidxml::node_element)
    {
        contents += node->value();
        get_contents(node->first_node(), contents);
        get_contents(node->next_sibling(), contents);
    }
}

// -------------------------------------------------------------

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




// -------------------------------------------------------------
// definitions
// -------------------------------------------------------------

// (template)parameter
struct param
{
    std::string name;
    std::string description;
    std::string type;
    std::string fulltype; // post-processed
};

// Basic element, base of a class/struct, function, define
struct element
{
    std::string name;
    std::string brief_description, detailed_description;
    std::string location;
    int line; // To sort - Doxygen changes order - we change it back

    // QBK-includes
    // Filled with e.g.: \qbk(include, reference/myqbk.qbk}
    std::vector<std::string> includes; 

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

struct configuration
{
    // Prefix to find headerfiles (will not be outputted)
    std::string prefix;
    std::vector<std::string> headerfiles;
};



// Predicate for std::find_if
struct par_by_name
{
    par_by_name(std::string const& n)
        : m_name(n)
    {}

    inline bool operator()(param const& p)
    {
        return p.name == m_name;
    }
private :
    std::string m_name;
};


// Predicate for std::find_if
struct par_by_type
{
    par_by_type(std::string const& n)
        : m_type(n)
    {}

    inline bool operator()(param const& p)
    {
        return p.type == m_type;
    }
private :
    std::string m_type;
};


template <typename Element>
struct sort_on_line
{
    inline bool operator()(Element const& left, Element const& right)
    {
        return left.line < right.line;
    }
};


static inline void add_or_set(std::vector<param>& parameters, param const& p)
{
    std::vector<param>::iterator it = std::find_if(parameters.begin(), parameters.end(), par_by_name(p.name));
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

// -------------------------------------------------------------
static void parse_parameter(rapidxml::xml_node<>* node, param& p)
{
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
        // Define: <param><defname>Point</defname></param>
        else if (name == "defname") p.name = node->value(); // for defines
        else if (name == "para") p.description = node->value();

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
            param p;
            parse_parameter(node->first_node(), p);
            if (! p.name.empty())
            {
                // Copy its description
                std::vector<param>::iterator it = std::find_if(parameters.begin(),
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
            param p;
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
static void parse_element(rapidxml::xml_node<>* node, std::string const& parent, Element& el)
{
    if (node != NULL)
    {
        std::string name = node->name();
        std::string full = parent + "." + name;

        if (full == ".briefdescription.para")
        {
            el.brief_description = node->value();
        }
        else if (full == ".detaileddescription.para")
        {
          std::string const para(node->value());
          if (!para.empty() && !el.detailed_description.empty())
          {
              el.detailed_description += "\n\n";
          }
          el.detailed_description += node->value();
        }
        else if (full == ".location")
        {
            std::string loc = get_attribute(node, "file");
            // Boost.Geometry specific, to make generic: make this path-start configurable
            std::size_t pos = loc.rfind("/boost/geometry/");
            if (pos != std::string::npos)
            {
                loc = loc.substr(pos + 1);
            }
            el.location = loc;
            el.line = atol(get_attribute(node, "line").c_str());
        }
        else if (full == ".detaileddescription.para.qbk.include")
        {
            el.includes.push_back(boost::trim_copy(std::string(node->value())));
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

        parse_element(node->first_node(), full, el);
        parse_element(node->next_sibling(), parent, el);
    }
}

static void parse_function(rapidxml::xml_node<>* node, std::string const& parent, function& f)
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
            // Boost.Geometry specific, to make generic: make this namespace-to-be-skipped configurable
            boost::replace_all(f.definition, "boost::geometry::", "");
        }
        else if (full == ".param")
        {
            param p;
            parse_parameter(node->first_node(), p);
            add_or_set(f.parameters, p);
        }
        else if (full == ".type")
        {
            f.return_type = node->value();
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

        parse_function(node->first_node(), full, f);
        parse_function(node->next_sibling(), parent, f);
    }
}

static void parse(rapidxml::xml_node<>* node, documentation& doc, bool member = false)
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
                parse_element(node->first_node(), "", doc.cos);
            }
        }
        else if (nodename == "memberdef")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "function" || kind == "define")
            {
                function f;
                parse_element(node->first_node(), "", f);
                parse_function(node->first_node(), "", f);
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
            parse(node->first_node(), doc, is_member);
        }
        parse(node->next_sibling(), doc, is_member);
    }
}

void quickbook_template_parameter_list(std::vector<param> const& parameters, std::ostream& out, bool name = false)
{
    if (!parameters.empty())
    {
        out << "template<" ;
        bool first = true;
        BOOST_FOREACH(param const& p, parameters)
        {
            out << (first ? "" : ", ") << p.fulltype;
            first = false;
        }
        out << ">" << std::endl;
    }
}


void quickbook_synopsis(function const& f, std::ostream& out)
{
    out << "``";
    quickbook_template_parameter_list(f.template_parameters, out);
    switch(f.type)
    {
        case function_constructor :
            out << f.name;
            break;
        case function_member :
            out << f.return_type << " " << f.name;
            break;
        case function_free :
            out << f.definition;
            break;
        case function_define :
            {
                out << "#define " << f.name;
                bool first = true;
                BOOST_FOREACH(param const& p, f.parameters)
                {
                    out << (first ? "(" : ", ") << p.name;
                    first = false;
                }
                if (! first)
                {
                    out << ")";
                }
            }
            break;
        case function_unknown :
            // do nothing
            break;
    }
    if (! f.argsstring.empty())
    {
        out << " " << f.argsstring;
    }
    out << "``" << std::endl
        << std::endl;
}

inline bool includes(std::string const& filename, std::string const& header)
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
            if (boost::starts_with(line, "#include")
                && boost::contains(line, header))
            {
                return true;
            }
        }
    }
    return false;
}


void quickbook_header(std::string const& location,
    configuration const& config,
    std::ostream& out)
{
    if (! location.empty())
    {
        std::vector<std::string> including_headers;

        // Check all headerfiles to see if this specific headerfile is found
        BOOST_FOREACH(std::string const& headerfile, config.headerfiles)
        {
            if (includes(config.prefix + headerfile, location))
            {
                including_headers.push_back(headerfile);
            }
        }

        out << "[heading Header]" << std::endl;
        if (! including_headers.empty())
        {
            out << "Either"
                << (including_headers.size() > 1 ? " one of" : "")
                << std::endl << std::endl;
            BOOST_FOREACH(std::string const& headerfile, including_headers)
            {
                out << "`#include <" << headerfile << ">`" << std::endl;
            }

            out << std::endl << "Or" << std::endl << std::endl;
        }
        out << "`#include <" << location << ">`" << std::endl;
        out << std::endl;
    }
}


void quickbook_includes(std::vector<std::string> const& includes, std::ostream& out)
{
    BOOST_FOREACH(std::string const& inc, includes)
    {
        out << "[include " << inc << "]" << std::endl;
    }
}



void quickbook_heading_string(std::string const& heading,
            std::string const& contents, std::ostream& out)
{
    if (! contents.empty())
    {
        out << "[heading " << heading << "]" << std::endl
            << contents << std::endl
            << std::endl;
    }
}

inline std::string to_section_name(std::string const& name)
{
    // Make section-name lowercase and remove :: because these are filenames
    return boost::to_lower_copy(boost::replace_all_copy(name, "::", "_"));
}



void quickbook_output(function const& f, configuration const& config, std::ostream& out)
{
    // Write the parsed function
    int arity = (int)f.parameters.size();

    std::string additional_description;

    if (! f.additional_description.empty())
    {
        additional_description = " (";
        additional_description += f.additional_description;
        additional_description += ")";
    }

    out << "[section:" << to_section_name(f.name);
    // Make section name unique if necessary by arity and additional description
    if (! f.unique)
    {
        out << "_" << arity;
        if (! f.additional_description.empty())
        {
            out << "_" << boost::to_lower_copy(boost::replace_all_copy(f.additional_description, " ", "_"));
        }
    }
    out << " " << f.name
        << additional_description
        << "]" << std::endl
        << std::endl;

    out << f.brief_description << std::endl;
    out << std::endl;

    quickbook_heading_string("Description", f.detailed_description, out);

    out << "[heading Synopsis]" << std::endl;
    quickbook_synopsis(f, out);

    out << "[heading Parameters]" << std::endl
        << std::endl;

    out << "[table" << std::endl << "[";
    if (f.type != function_define)
    {
        out << "[Type] [Concept] ";
    }
    out << "[Name] [Description] ]" << std::endl;

    // First: output any template parameter which is NOT used in the normal parameter list
    BOOST_FOREACH(param const& tp, f.template_parameters)
    {
        std::vector<param>::const_iterator it = std::find_if(f.parameters.begin(), f.parameters.end(), par_by_type(tp.name));

        if (it == f.parameters.end())
        {
            out << "[[" << tp.name << "] [" << tp.description << "] [ - ] [Must be specified]]" << std::endl;
        }

    }

    BOOST_FOREACH(param const& p, f.parameters)
    {
        out << "[";
        std::vector<param>::const_iterator it = std::find_if(f.template_parameters.begin(),
            f.template_parameters.end(), par_by_name(p.type));

        if (f.type != function_define)
        {
            out << "[" << p.fulltype
                << "] [" << (it == f.template_parameters.end() ? "" : it->description)
                << "] ";
        }
        out << "[" << p.name
            << "] [" << p.description
            << "]]"
            << std::endl;
    }
    out << "]" << std::endl
        << std::endl
        << std::endl;

    quickbook_heading_string("Returns", f.return_description, out);

    quickbook_header(f.location, config, out);

    quickbook_includes(f.includes, out);

    out << std::endl;
    out << "[endsect]" << std::endl;
    out << std::endl;
}


void quickbook_short_output(function const& f, std::ostream& out)
{
    BOOST_FOREACH(param const& p, f.parameters)
    {
        out << "[* " << p.fulltype << "]: ['" << p.name << "]:  " << p.description << std::endl << std::endl;
    }
    out << std::endl;
    out << std::endl;

    if (! f.return_description.empty())
    {
        out << "][" << std::endl;
        out << f.return_description << std::endl;
        out << std::endl;
    }

    out << std::endl;
}

void quickbook_output(class_or_struct const& cos, configuration const& config, std::ostream& out)
{
    // Boost.Geometry specific, to make generic: make this namespace-to-be-skipped configurable
    std::string short_name =
            boost::replace_all_copy(cos.fullname, "boost::geometry::", "");

    // Write the parsed function
    out << "[section:" << to_section_name(short_name) << " " << short_name << "]" << std::endl
        << std::endl;

    quickbook_heading_string("Description", cos.detailed_description, out);

    out << "[heading Synopsis]" << std::endl
        << "``";
    quickbook_template_parameter_list(cos.template_parameters, out);
    out << "class " << short_name << std::endl
        << "{" << std::endl
        << "  // ..." << std::endl
        << "};" << std::endl
        << "``" << std::endl << std::endl;

    if (! cos.template_parameters.empty())
    {
        out << "[heading Template parameter(s)]" << std::endl
            << "[table" << std::endl
            << "[[Parameter] [Description]]" << std::endl;

        BOOST_FOREACH(param const& p, cos.template_parameters)
        {
            out << "[[" << p.fulltype << "] [" << p.description << "]]" << std::endl;
        }
        out << "]" << std::endl
            << std::endl;
    }


    std::map<function_type, int> counts;
    BOOST_FOREACH(function const& f, cos.functions)
    {
        counts[f.type]++;
    }

    if (counts[function_constructor] > 0)
    {
        out << "[heading Constructor(s)]" << std::endl
            << "[table" << std::endl
            << "[[Function] [Description] [Parameters] ]" << std::endl;

        BOOST_FOREACH(function const& f, cos.functions)
        {
            if (f.type == function_constructor)
            {
                out << "[[";
                quickbook_synopsis(f, out);
                out << "] [" << f.brief_description << "] [";
                quickbook_short_output(f, out);
                out << "]]" << std::endl;
            }
        }
        out << "]" << std::endl
            << std::endl;
    }

    if (counts[function_member] > 0)
    {
        out << "[heading Member Function(s)]" << std::endl
            << "[table" << std::endl
            << "[[Function] [Description] [Parameters] [Returns] ]" << std::endl;

        BOOST_FOREACH(function const& f, cos.functions)
        {
            if (f.type == function_member)
            {
                out << "[[";
                quickbook_synopsis(f, out);
                out << "] [" << f.brief_description << "] [";
                quickbook_short_output(f, out);
                out << "]]" << std::endl;
            }
        }
        out << "]" << std::endl
            << std::endl;
    }

    quickbook_header(cos.location, config, out);
    //quickbook_examples(cos.examples, out);
    quickbook_includes(cos.includes, out);

    out << "[endsect]" << std::endl
        << std::endl;
}

inline bool equal_ignore_fix(std::string const& a, std::string const& b, std::string const &fix)
{
    return fix + a == b || a == fix + b
        || a + fix == b || a == b + fix;
}


int main(int argc, char** argv)
{
    // TODO: use boost::program_options
    if (argc < 2)
    {
        std::cerr
            << "Usage: doxygen_xml2qbk [XML-filename] {prefix CH} {convenience header (CH) 1} {CH 2} ..." << std::endl
            << "   where the XML refers to an XML written by Doxygen" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string xml_string = file_to_string(filename);

    xml_doc xml(xml_string.c_str());

    documentation doc;
    parse(xml.first_node(), doc);

    for (std::size_t i = 0; i < doc.functions.size(); i++)
    {
        function& f1 = doc.functions[i];
        for (std::size_t j = i + 1; j < doc.functions.size(); j++)
        {
            function& f2 = doc.functions[j];

            if (f1.name == f2.name)
            {
                // It is not a unique function, so e.g. an overload,
                // so a description must distinguish them.
                // Difference is either the number of parameters, or a const / non-const version
                // Use the "\qbk{distinguish,with strategy}" in the source code to distinguish
                f1.unique = false;
                f2.unique = false;
            }
        }
    }

    configuration config;
    if (argc > 2)
    {
        config.prefix = argv[2];
        for (int i = 3; i < argc; i++)
        {
            config.headerfiles.push_back(argv[i]);
        }
    }

    // Keep inspect silent. TODO: read this block from a file.
    std::cout
        << "[/============================================================================" << std::endl
        << "  Boost.Geometry (aka GGL, Generic Geometry Library)" << std::endl
        << std::endl
        << "  Copyright (c) 2009-2011 Barend Gehrels, Geodan, Amsterdam, the Netherlands." << std::endl
        << "  Copyright (c) 2009-2011 Mateusz Loskot (mateusz@loskot.net)" << std::endl
        << "  Copyright (c) 2009-2011 Bruno Lalande, Paris, France." << std::endl
        << std::endl
        << "  Use, modification and distribution is subject to the Boost Software License," << std::endl
        << "  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at" << std::endl
        << "  http://www.boost.org/LICENSE_1_0.txt)" << std::endl
        << "=============================================================================/]" << std::endl
        << std::endl
        << std::endl;

    std::cout
        << "[/ Generated by doxygen_xml2qbk, don't change, it will be overwritten automatically]" << std::endl
        << "[/ Generated from " << filename << "]" << std::endl;

    BOOST_FOREACH(function const& f, doc.functions)
    {
        quickbook_output(f, config, std::cout);
    }
    BOOST_FOREACH(function const& f, doc.defines)
    {
        quickbook_output(f, config, std::cout);
    }

    if (! doc.cos.name.empty())
    {
        std::sort(doc.cos.functions.begin(), doc.cos.functions.end(), sort_on_line<function>());
        quickbook_output(doc.cos, config, std::cout);
    }

    return 0;
}

