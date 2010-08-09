:: ===========================================================================
::  Copyright (c) 1995-2010 Barend Gehrels, Geodan, Amsterdam, the Netherlands.
::  Copyright (c) 2008-2010 Bruno Lalande, Paris, France.
::  Copyright (c) 2009-2010 Mateusz Loskot (mateusz@loskot.net), London, UK
:: 
::  Use, modification and distribution is subject to the Boost Software License,
::  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
::  http://www.boost.org/LICENSE_1_0.txt)
:: ============================================================================

:: Note, all of this is very experimental
:: BSG, Aug 1, 2010


@echo off

doxygen

set xml2qbk=..\..\..\..\other\programs\doxygen_xml2qbk\Release\doxygen_xml2qbk.exe
%xml2qbk% doxygen_output\xml\group__area.xml > area.qbk
%xml2qbk% doxygen_output\xml\group__access.xml > access.qbk
%xml2qbk% doxygen_output\xml\group__distance.xml > distance.qbk
%xml2qbk% doxygen_output\xml\group__simplify.xml > simplify.qbk

set xslt=c:\software\xsltproc\xml\bin\xsltproc.exe

:: %xslt% reference.xsl doxygen_output\xml\classboost_1_1geometry_1_1point.xml > point.qbk
:: %xslt% reference.xsl doxygen_output\xml\group__area.xml > area.qbk
:: %xslt% reference.xsl doxygen_output\xml\area_8hpp.xml > area2.qbk


bjam --toolset=msvc --without-python

