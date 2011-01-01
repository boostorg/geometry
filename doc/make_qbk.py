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


import os, sys


cmd="doxygen_xml2qbk  doxy/doxygen_output/xml/%s.xml ../../../ boost/geometry/geometry.hpp boost/geometry/geometries/geometries.hpp boost/geometry/multi/multi.hpp > reference/%s.qbk"

def call_doxygen():
	os.chdir("doxy");
	os.system("doxygen")
	os.chdir("..")

def group_to_quickbook(section):
	os.system(cmd % ("group__" + section, section))

def xml_to_quickbook(xml, section):
	os.system(cmd % (xml, section))


call_doxygen()

algorithms = ["area", "buffer", "centroid", "combine", "convert" 
	, "convex_hull", "difference", "disjoint", "dissolve", "distance" 
	, "envelope", "equals", "for_each", "intersection", "intersects" 
	, "length", "num_geometries", "num_interior_rings", "num_points" 
	, "overlaps", "perimeter", "reverse", "simplify", "sym_difference" 
	, "transform", "union", "unique", "within"]

for a in algorithms:
	group_to_quickbook(a)

group_to_quickbook("register")

xml_to_quickbook("classboost_1_1geometry_1_1model_1_1point", "point")
xml_to_quickbook("classboost_1_1geometry_1_1model_1_1d2_1_1point__xy", "point_xy")
xml_to_quickbook("classboost_1_1geometry_1_1model_1_1linestring", "linestring")
xml_to_quickbook("classboost_1_1geometry_1_1model_1_1box", "box")

xml_to_quickbook("structboost_1_1geometry_1_1closing__iterator", "closing_iterator")

os.system("bjam") 
