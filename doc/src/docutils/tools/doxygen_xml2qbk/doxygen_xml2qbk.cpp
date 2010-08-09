// Boost.Geometry (aka GGL, Generic Geometry Library)
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
//     currently this is the element <qbk.snippet> which will make a reference
//     to an example.
// - earlier generations of QBK was done by XSLT, I'm not so into the XSLT and
//   think this is more flexible. The XSLT only did point-structure, not yet
//   functions as area, so there is not (yet) an overlap
// - currently still in draft


#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _SCL_INSECURE_DEPRECATE

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>


#include <rapidxml.hpp>

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



// -------------------------------------------------------------
std::string file_to_string(std::string const& filename)
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
        }
    }
    return result;
}

struct par
{
    std::string name;
    std::string description;
    std::string type, fulltype;
    std::string concept;
};

struct function
{
    std::string name;
    std::string brief_description, detailed_description;
    std::string argsstring, definition;
    std::string return_type;
    std::string model_of;
    std::string location;
    std::string paragraphs; 
    std::vector<std::string> snippets;
    std::vector<std::string> images;

    std::map<std::string, par> parameters;
    std::vector<std::string> tparams;
};

// -------------------------------------------------------------
static void parse_parameter(rapidxml::xml_node<>* node, par& p)
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
        else if (name == "para") p.description = node->value();

        parse_parameter(node->first_node(), p);
        parse_parameter(node->next_sibling(), p);
    }
}

static void parse_parameter_list(rapidxml::xml_node<>* node, function& f)
{
	if (node != NULL)
	{
        std::string name = node->name();

		if (name == "parameteritem") 
        {
            par p;
            parse_parameter(node->first_node(), p);
            if (! p.name.empty())
            {
                // Add or set description
                typedef std::map<std::string, par>::iterator iterator;
                iterator it = f.parameters.find(p.name);
                if (it == f.parameters.end())
                {
                    f.parameters[p.name] = p;
                }
                else
                {
                    f.parameters[p.name].description = p.description;
                }
            }
        }
		else if (name == "param") 
        {
            // Element of 'templateparamlist.param (.type,.declname,.defname)'
            par p;
            parse_parameter(node->first_node(), p);
            std::string tparam = p.type;
            if (! p.name.empty())
            {
                tparam += " ";
                tparam += p.name;
            }
            f.tparams.push_back(tparam);
        }

        parse_parameter_list(node->first_node(), f);
        parse_parameter_list(node->next_sibling(), f);
    }
}


static void parse_member(rapidxml::xml_node<>* node, std::string const& parent, function& f)
{
	if (node != NULL)
	{
        std::string name = node->name();
        std::string full = parent + "." + name;

		if (full == ".name") f.name = node->value();
        else if (full == ".briefdescription.para") f.brief_description = node->value();
        else if (full == ".detaileddescription.para") f.detailed_description = node->value();
        else if (full == ".argsstring") f.argsstring = node->value();
        else if (full == ".definition") 
        {
            f.definition = node->value();
            boost::replace_all(f.definition, "boost::geometry::", "");
        }
        else if (full == ".location")
        {
            std::string loc = get_attribute(node, "file");
            std::size_t pos = loc.rfind("/boost/geometry/");
            if (pos != std::string::npos)
            {
                loc = loc.substr(pos + 1);
            }
            f.location = loc;
        }
        else if (full == ".param")
        {
            par p;
            parse_parameter(node->first_node(), p);
            f.parameters[p.name] = p;
        }
        else if (full == ".detaileddescription.para.simplesect")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "return")
            {
                get_contents(node->first_node(), f.return_type);
            }
            else if (kind == "par")
            {
                get_contents(node->first_node(), f.paragraphs);
            }
        }
        else if (full == ".detaileddescription.para.qbk.snippet")
        {
            f.snippets.push_back(node->value());
        }
        else if (full == ".detaileddescription.para.image")
        {
            std::string image = get_attribute(node, "name");
            if (! image.empty())
            {
                f.images.push_back(image);
            }
        }
        else if (full == ".templateparamlist") 
        {
            parse_parameter_list(node->first_node(), f);
        }
        else if (full == ".detaileddescription.para.parameterlist")
        {
            std::string kind = get_attribute(node, "kind");
            if (kind == "param")
            {
                parse_parameter_list(node->first_node(), f);
            }
            else if (kind == "templateparam")
            {
                // Get descriptions of template parameters
                function tf;
                parse_parameter_list(node->first_node(), tf);
                // Map them on the found parameters (which are already typed)
                typedef std::map<std::string, par>::iterator iterator;
                for (iterator it = tf.parameters.begin();
                    it != tf.parameters.end(); ++it)
                {
                    // Find it by type (not by name, so sequentially)
                    for (iterator fit = f.parameters.begin();
                        fit != f.parameters.end(); ++fit)
                    {
                        if (fit->second.type == it->second.name)
                        {
                            fit->second.concept = it->second.description;
                        }
                    }
                }
            }
        }
        
        parse_member(node->first_node(), full, f);
        parse_member(node->next_sibling(), parent, f);
    }
}


void quickbook_output(function const& f, std::ostream& out, int index)
{
    // Write the parsed function
    int arity = f.parameters.size();
    out << "[section:" << f.name << "_" << arity << " " << f.name << " (" << arity << ")" "]" << std::endl;
    out << std::endl;

    out << f.brief_description << std::endl;
    out << std::endl;

    out << "[heading Synopsis]" << std::endl;
    out << "``";
    if (!f.tparams.empty())
    {
        out << "template<" ;
        bool first = true;
        BOOST_FOREACH(std::string const& tparam, f.tparams)
        {
            out << (first ? "" : ", ") << tparam;
            first = false;
        }
        out << ">" << std::endl;

    }
    out << f.definition << " " << f.argsstring << "``" << std::endl;
    out << std::endl;

    out << "[heading Parameters]" << std::endl;
    out << std::endl;

    out << "[table" << std::endl;
    out << "[[Type] [Concept] [Name] [Description] ]" << std::endl;

    typedef std::map<std::string, par>::const_iterator iterator;
    for (iterator it = f.parameters.begin();
        it != f.parameters.end(); ++it)
    {
        par const& p = it->second;
        out << "[[" << p.fulltype << "] [" << p.concept << "] [" << p.name << "] [" << p.description << "]]" << std::endl;
    }
    out << "]" << std::endl;
    out << std::endl;
    out << std::endl;

    if (! f.return_type.empty())
    {
        out << "[heading Returns]" << std::endl;
        out << f.return_type << std::endl;
        out << std::endl;
    }

    out << "[heading Description]" << std::endl;
    out << f.detailed_description << std::endl;
    out << std::endl;

    if (! f.model_of.empty())
    {
        out << "[heading Model of]" << std::endl;
        out << f.model_of << std::endl;
        out << std::endl;
    }

    out << "[heading Header]" << std::endl;
    if (true)
    {
        // TODO: get the alternative headerfiles from somewhere
        out << "Either" << std::endl << std::endl;
        out << "`#include <boost/geometry/geometry.hpp>`" << std::endl << std::endl;
        out << "Or" << std::endl << std::endl;
    }
    out << "`#include <" << f.location << ">`" << std::endl;
    out << std::endl;

    if (! f.snippets.empty())
    {
        out << "[heading Snippets]" << std::endl;
        BOOST_FOREACH(std::string const& snippet, f.snippets)
        {
            out << "[" << snippet << "]" << std::endl;
        }
    }
    out << std::endl;

    if (! f.images.empty())
    {
        out << "[heading Image(s)]" << std::endl;
        BOOST_FOREACH(std::string const& image, f.images)
        {
            out << "[$" << image << "]" << std::endl;
        }
    }
    out << std::endl;

    out << "[endsect]" << std::endl;
    out << std::endl;
}

static void parse_function(rapidxml::xml_node<>* node, std::ostream& out, std::vector<function>& functions)
{
	if (node != NULL)
	{
        bool recurse = false;
		if (! strcmp(node->name(), "doxygen")
            || ! strcmp(node->name(), "compounddef")
            || ! strcmp(node->name(), "sectiondef")
            )
		{
            recurse = true;
		}
		else if (! strcmp(node->name(), "memberdef"))
		{
            std::string kind = get_attribute(node, "kind");
            if (kind == "function")
            {
                function f;
                parse_member(node->first_node(), "", f);
                functions.push_back(f);
            }
		}

        if (recurse)
        {
		    // First recurse into childnodes, then handle next siblings
		    parse_function(node->first_node(), out, functions);
        }
        parse_function(node->next_sibling(), out, functions);
	}
}




int main(int argc, char** argv)
{
    std::string filename = 
        argc > 1 
            ? argv[1] 
            : "../../../libs/geometry/doc/qbk/doxygen_output/xml/group__simplify.xml";

    std::string xml = file_to_string(filename);

    xml_doc doc(xml.c_str());
    
    std::vector<function> functions;
    parse_function(doc.first_node(), std::cout, functions);

    int i = 0;
    BOOST_FOREACH(function const& f, functions)
    {
        quickbook_output(f, std::cout, i++);        
    }

    return 0;
}

