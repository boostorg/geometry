// Boost.Geometry (aka GGL, Generic Geometry Library)
// This file is manually converted from PROJ4

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Geometry Library by Barend Gehrels (Geodan, Amsterdam)

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_PJ_PARAM_HPP
#define BOOST_GEOMETRY_PROJECTIONS_PJ_PARAM_HPP


#include <string>
#include <vector>

#include <boost/geometry/srs/projections/impl/dms_parser.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>


namespace boost { namespace geometry { namespace projections {

namespace detail {



/* create pvalue list entry */
template <typename T>
inline pvalue<T> pj_mkparam(std::string const& name, std::string const& value)
{
    pvalue<T> newitem;
    newitem.param = name;
    newitem.s = value;
    //newitem.used = false;
    return newitem;
}

template <typename T>
inline pvalue<T> pj_mkparam(std::string const& str)
{
    std::string name = str;
    std::string value;
    boost::trim_left_if(name, boost::is_any_of("+"));
    std::string::size_type loc = name.find("=");
    if (loc != std::string::npos)
    {
        value = name.substr(loc + 1);
        name.erase(loc);
    }

    return pj_mkparam<T>(name, value);
}

/************************************************************************/
/*                              pj_param_X()                            */
/*                                                                      */
/*      Test for presence and/or get pvalue value.                      */
/*      Where X can be:                                                 */
/*                                                                      */
/*       `e' - test for presence, return true/false                     */
/*       `i' - integral number, returned as int                         */
/*       `f' - floating point number, returned as T                     */
/*       `r' - radians, returned as T                                   */
/*       `s' - string, returned as std::string                          */
/*       `b' - boolean, test for t/T/f/F, returned as bool              */
/*                                                                      */
/************************************************************************/

/* input exists */
template <typename T>
inline bool pj_param_e(std::vector<pvalue<T> > const& pl, std::string const& name)
{
    typedef typename std::vector<pvalue<T> >::const_iterator iterator;
    for (iterator it = pl.begin(); it != pl.end(); it++)
    {
        if (it->param == name)
        {
            return true;
        }
    }

    return false;
}

/* integer input */
template <typename T>
inline bool pj_param_i(std::vector<pvalue<T> > const& pl, std::string const& name, int & par)
{
    typedef typename std::vector<pvalue<T> >::const_iterator iterator;
    for (iterator it = pl.begin(); it != pl.end(); it++)
    {
        if (it->param == name)
        {
            //it->used = true;
            par = atoi(it->s.c_str());
            return true;
        }
    }

    return false;
}

template <typename T>
inline int pj_param_i(std::vector<pvalue<T> > const& pl, std::string const& name)
{
    int res = 0;
    pj_param_i(pl, name, res);
    return res;
}

/* floating point input */
template <typename T>
inline bool pj_param_f(std::vector<pvalue<T> > const& pl, std::string const& name, T & par)
{
    typedef typename std::vector<pvalue<T> >::const_iterator iterator;
    for (iterator it = pl.begin(); it != pl.end(); it++)
    {
        if (it->param == name)
        {
            //it->used = true;
            par = atof(it->s.c_str());
            return true;
        }
    }

    return false;
}

template <typename T>
inline T pj_param_f(std::vector<pvalue<T> > const& pl, std::string const& name)
{
    T res = 0;
    pj_param_f(pl, name, res);
    return res;
}

/* radians input */
template <typename T>
inline bool pj_param_r(std::vector<pvalue<T> > const& pl, std::string const& name, T & par)
{
    typedef typename std::vector<pvalue<T> >::const_iterator iterator;
    for (iterator it = pl.begin(); it != pl.end(); it++)
    {
        if (it->param == name)
        {
            //it->used = true;
            dms_parser<T, true> parser;
            par = parser.apply(it->s.c_str()).angle();
            return true;
        }
    }

    return false;
}

template <typename T>
inline T pj_param_r(std::vector<pvalue<T> > const& pl, std::string const& name)
{
    T res = 0;
    pj_param_r(pl, name, res);
    return res;
}

/* string input */
template <typename T>
inline bool pj_param_s(std::vector<pvalue<T> > const& pl, std::string const& name, std::string & par)
{
    typedef typename std::vector<pvalue<T> >::const_iterator iterator;
    for (iterator it = pl.begin(); it != pl.end(); it++)
    {
        if (it->param == name)
        {
            //it->used = true;
            par = it->s;
            return true;
        }
    }

    return false;
}

template <typename T>
inline std::string pj_param_s(std::vector<pvalue<T> > const& pl, std::string const& name)
{
    std::string res;
    pj_param_s(pl, name, res);
    return res;
}

/* bool input */
template <typename T>
inline bool pj_param_b(std::vector<pvalue<T> > const& pl, std::string const& name)
{
    typedef typename std::vector<pvalue<T> >::const_iterator iterator;
    for (iterator it = pl.begin(); it != pl.end(); it++)
    {
        if (it->param == name)
        {
            //it->used = true;
            switch (it->s[0])
            {
            case '\0': case 'T': case 't':
                return true;
            case 'F': case 'f':
            default:
                return false;
            }
        }
    }

    return false;
}

} // namespace detail
}}} // namespace boost::geometry::projections

#endif
