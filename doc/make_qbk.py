#! /usr/bin/env python
# -*- coding: utf-8 -*-
# ===========================================================================
#  Copyright (c) 1995-2010 Barend Gehrels, Geodan, Amsterdam, the Netherlands.
#  Copyright (c) 2008-2010 Bruno Lalande, Paris, France.
#  Copyright (c) 2009-2010 Mateusz Loskot (mateusz@loskot.net), London, UK
# 
#  Use, modification and distribution is subject to the Boost Software License,
#  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)9
# ============================================================================

# Note, all of this is very experimental
# BSG, Aug 1, 2010

import os

# if a%1 == askip_doxygen goto skip_doxygen

os.chdir("doxy");
os.system("doxygen")
os.chdir("..")

#skip_doxygen

cmd="doxygen_xml2qbk  doxy/doxygen_output/xml/%s.xml ../../../../ boost/geometry/geometry.hpp boost/geometry/geometries/geometries.hpp boost/geometry/multi/multi.hpp > reference/%s.qbk"

# Algorithms
os.system(cmd % ("group__area", "area"))
os.system(cmd % ("group__buffer", "buffer"))
os.system(cmd % ("group__centroid", "centroid"));
os.system(cmd % ("group__convex__hull", "convex_hull"))
os.system(cmd % ("group__dissolve", "dissolve"))
os.system(cmd % ("group__envelope", "envelope"))
os.system(cmd % ("group__length", "length"))
os.system(cmd % ("group__num__geometries", "num_geometries"))
os.system(cmd % ("group__num__interior__rings", "num_interior_rings"))
os.system(cmd % ("group__num__points", "num_points"))
os.system(cmd % ("group__perimeter", "perimeter"))
os.system(cmd % ("group__reverse", "reverse"))
os.system(cmd % ("group__simplify", "simplify"))
os.system(cmd % ("group__unique", "unique"))

# os.system(cmd % ("group__access", "access"))
os.system(cmd % ("group__combine", "combine"))
os.system(cmd % ("group__convert", "convert"))
os.system(cmd % ("group__difference", "difference"))
os.system(cmd % ("group__disjoint", "disjoint"))

os.system(cmd % ("group__distance", "distance"))

os.system(cmd % ("group__equals", "equals"))
os.system(cmd % ("group__for__each", "for_each"))

os.system(cmd % ("group__intersection", "intersection"))
os.system(cmd % ("group__intersects", "intersects"))

os.system(cmd % ("group__overlaps", "overlaps"))
os.system(cmd % ("group__sym__difference", "sym_difference"))
os.system(cmd % ("group__transform", "transform"))
os.system(cmd % ("group__union", "union"))

os.system(cmd % ("group__within", "within"))
os.system(cmd % ("group__register", "register"))

os.system(cmd % ("classboost_1_1geometry_1_1point", "point"))
os.system(cmd % ("classboost_1_1geometry_1_1point__xy", "point_xy"))

os.system(cmd % ("classboost_1_1geometry_1_1concept_1_1_point", "concept_point"))

os.system(cmd % ("structboost_1_1geometry_1_1closing__iterator", "closing_iterator"))

os.system("bjam") 
