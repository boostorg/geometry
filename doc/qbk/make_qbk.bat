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

if a%1 == askip_doxygen goto skip_doxygen

cd ..
doxygen
cd qbk

:skip_doxygen

set xml2qbk=..\..\..\..\other\programs\doxygen_xml2qbk\Release\doxygen_xml2qbk.exe
set out=..\doxygen_output\xml

:: Algorithms
%xml2qbk% %out%\group__area.xml > reference\area.qbk
%xml2qbk% %out%\group__buffer.xml > reference\buffer.qbk
%xml2qbk% %out%\group__centroid.xml > reference\centroid.qbk
%xml2qbk% %out%\group__convex__hull.xml > reference\convex_hull.qbk
%xml2qbk% %out%\group__dissolve.xml > reference\dissolve.qbk
%xml2qbk% %out%\group__envelope.xml > reference\envelope.qbk
%xml2qbk% %out%\group__length.xml > reference\length.qbk
%xml2qbk% %out%\group__num__geometries.xml > reference\num_geometries.qbk
%xml2qbk% %out%\group__num__interior__rings.xml > reference\num_interior_rings.qbk
%xml2qbk% %out%\group__num__points.xml > reference\num_points.qbk
%xml2qbk% %out%\group__perimeter.xml > reference\perimeter.qbk
%xml2qbk% %out%\group__reverse.xml > reference\reverse.qbk
%xml2qbk% %out%\group__simplify.xml > reference\simplify.qbk
%xml2qbk% %out%\group__unique.xml > reference\unique.qbk
:: %xml2qbk% %out%\group__access.xml > reference\access.qbk
:: %xml2qbk% %out%\group__combine.xml > reference\combine.qbk
:: %xml2qbk% %out%\group__convert.xml > reference\convert.qbk
:: %xml2qbk% %out%\group__difference.xml > reference\difference.qbk
:: %xml2qbk% %out%\group__disjoint.xml > reference\disjoint.qbk
%xml2qbk% %out%\group__distance.xml > reference\distance.qbk
:: %xml2qbk% %out%\group__equals.xml > reference\equals.qbk
:: %xml2qbk% %out%\group__for__each.xml > reference\for_each.qbk
%xml2qbk% %out%\group__intersection.xml > reference\intersection.qbk
%xml2qbk% %out%\group__intersects.xml > reference\intersects.qbk
:: %xml2qbk% %out%\group__overlaps.xml > reference\overlaps.qbk
:: %xml2qbk% %out%\group__sym__difference.xml > reference\sym_difference.qbk
:: %xml2qbk% %out%\group__transform.xml > reference\transform.qbk
:: %xml2qbk% %out%\group__union.xml > reference\union.qbk
%xml2qbk% %out%\group__within.xml > reference\within.qbk

%xml2qbk% %out%\group__register.xml > reference\register.qbk


%xml2qbk% %out%\classboost_1_1geometry_1_1point.xml > reference\point.qbk
%xml2qbk% %out%\classboost_1_1geometry_1_1point__xy.xml > reference\point_xy.qbk

%xml2qbk% %out%\classboost_1_1geometry_1_1concept_1_1_point.xml > reference\concept_point.qbk

%xml2qbk% %out%\structboost_1_1geometry_1_1closing__iterator.xml > reference\closing_iterator.qbk

:: set xslt=c:\software\xsltproc\xml\bin\xsltproc.exe
:: %xslt% reference.xsl doxygen_output\xml\classboost_1_1geometry_1_1point.xml > point.qbk
:: %xslt% reference.xsl doxygen_output\xml\group__area.xml > area.qbk
:: %xslt% reference.xsl doxygen_output\xml\area_8hpp.xml > area2.qbk

bjam --toolset=msvc --without-python
