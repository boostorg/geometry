# ===========================================================================
#  Copyright (c) 1995-2010 Barend Gehrels, Geodan, Amsterdam, the Netherlands.
#  Copyright (c) 2008-2010 Bruno Lalande, Paris, France.
#  Copyright (c) 2009-2010 Mateusz Loskot (mateusz@loskot.net), London, UK
# 
#  Use, modification and distribution is subject to the Boost Software License,
#  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)
# ============================================================================

cd ..
doxygen
cd qbk

# Assume that doxygen_xml2qbk is in /usr/local/bin
export out=../doxygen_output/xml

# Algorithms
doxygen_xml2qbk $out/group__area.xml > reference/area.qbk
doxygen_xml2qbk $out/group__buffer.xml > reference/buffer.qbk
doxygen_xml2qbk $out/group__centroid.xml > reference/centroid.qbk
doxygen_xml2qbk $out/group__convex__hull.xml > reference/convex_hull.qbk
doxygen_xml2qbk $out/group__dissolve.xml > reference/dissolve.qbk
doxygen_xml2qbk $out/group__envelope.xml > reference/envelope.qbk
doxygen_xml2qbk $out/group__length.xml > reference/length.qbk
doxygen_xml2qbk $out/group__num__geometries.xml > reference/num_geometries.qbk
doxygen_xml2qbk $out/group__num__interior__rings.xml > reference/num_interior_rings.qbk
doxygen_xml2qbk $out/group__num__points.xml > reference/num_points.qbk
doxygen_xml2qbk $out/group__perimeter.xml > reference/perimeter.qbk
doxygen_xml2qbk $out/group__reverse.xml > reference/reverse.qbk
doxygen_xml2qbk $out/group__simplify.xml > reference/simplify.qbk
doxygen_xml2qbk $out/group__unique.xml > reference/unique.qbk
# doxygen_xml2qbk $out/group__access.xml > reference/access.qbk
# doxygen_xml2qbk $out/group__combine.xml > reference/combine.qbk
# doxygen_xml2qbk $out/group__convert.xml > reference/convert.qbk
# doxygen_xml2qbk $out/group__difference.xml > reference/difference.qbk
# doxygen_xml2qbk $out/group__disjoint.xml > reference/disjoint.qbk
doxygen_xml2qbk $out/group__distance.xml > reference/distance.qbk
# doxygen_xml2qbk $out/group__equals.xml > reference/equals.qbk
# doxygen_xml2qbk $out/group__for__each.xml > reference/for_each.qbk
doxygen_xml2qbk $out/group__intersection.xml > reference/intersection.qbk
doxygen_xml2qbk $out/group__intersects.xml > reference/intersects.qbk
# doxygen_xml2qbk $out/group__overlaps.xml > reference/overlaps.qbk
# doxygen_xml2qbk $out/group__sym__difference.xml > reference/sym_difference.qbk
# doxygen_xml2qbk $out/group__transform.xml > reference/transform.qbk
# doxygen_xml2qbk $out/group__union.xml > reference/union.qbk
doxygen_xml2qbk $out/group__within.xml > reference/within.qbk

doxygen_xml2qbk $out/group__register.xml > reference/register.qbk


doxygen_xml2qbk $out/classboost_1_1geometry_1_1point.xml > reference/point.qbk
doxygen_xml2qbk $out/classboost_1_1geometry_1_1point__xy.xml > reference/point_xy.qbk

doxygen_xml2qbk $out/classboost_1_1geometry_1_1concept_1_1_point.xml > reference/concept_point.qbk

doxygen_xml2qbk $out/structboost_1_1geometry_1_1closing__iterator.xml > reference/closing_iterator.qbk

bjam --toolset=gcc --without-python
echo Please ignore for the moment the IDs which are not found

