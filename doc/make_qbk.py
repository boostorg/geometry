#! /usr/bin/env python
# -*- coding: utf-8 -*-
# ===========================================================================
#  Copyright (c) 1995-2010 Barend Gehrels, Geodan, Amsterdam, the Netherlands.
#  Copyright (c) 2008-2010 Bruno Lalande, Paris, France.
#  Copyright (c) 2009-2010 Mateusz Loskot (mateusz@loskot.net), London, UK
# 
#  Use, modification and distribution is subject to the Boost Software License,
#  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)
# ============================================================================

import os, sys

cmd = "doxygen_xml2qbk"
cmd = cmd + " --xml doxy/doxygen_output/xml/%s.xml"
cmd = cmd + " --start_include boost/geometry/"
cmd = cmd + " --convenience_header_path ../../../boost/geometry/"
cmd = cmd + " --convenience_headers geometry.hpp,geometries/geometries.hpp,multi/multi.hpp"
cmd = cmd + " --skip_namespace boost::geometry::"
cmd = cmd + " --copyright src/copyright_block.qbk"
cmd = cmd + " > reference/%s.qbk"

def call_doxygen():
    os.chdir("doxy");
    os.system("doxygen")
    os.chdir("..")

def group_to_quickbook(section):
    os.system(cmd % ("group__" + section.replace("_", "__"), section))

def model_to_quickbook(section):
    os.system(cmd % ("classboost_1_1geometry_1_1model_1_1" + section.replace("_", "__"), section))

def model_to_quickbook2(classname, section):
    os.system(cmd % ("classboost_1_1geometry_1_1model_1_1" + classname, section))

def struct_to_quickbook(section):
    os.system(cmd % ("structboost_1_1geometry_1_1" + section.replace("_", "__"), section))

def class_to_quickbook(section):
    os.system(cmd % ("classboost_1_1geometry_1_1" + section.replace("_", "__"), section))

def strategy_to_quickbook(section):
    p = section.find("::")
    ns = section[:p]
    strategy = section[p+2:]
    os.system(cmd % ("classboost_1_1geometry_1_1strategy_1_1" 
        + ns.replace("_", "__") + "_1_1" + strategy.replace("_", "__"), 
        ns + "_" + strategy))
        
def cs_to_quickbook(section):
    os.system(cmd % ("structboost_1_1geometry_1_1cs_1_1" + section.replace("_", "__"), section))
        

call_doxygen()

algorithms = ["append", "assign", "make", "clear"
    , "area", "buffer", "centroid", "combine", "convert", "correct"
    , "convex_hull", "difference", "disjoint", "dissolve", "distance" 
    , "envelope", "equals", "for_each", "intersection", "intersects" 
    , "length", "num_geometries", "num_interior_rings", "num_points" 
    , "overlaps", "perimeter", "reverse", "simplify", "sym_difference" 
    , "transform", "union", "unique", "within"]

access_functions = ["get", "set", "exterior_ring", "interior_rings"
    , "num_points", "num_interior_rings", "num_geometries"]
    
coordinate_systems = ["cartesian", "geographic", "polar", "spherical"]

core = ["closure", "coordinate_system", "coordinate_type", "cs_tag"
    , "dimension", "exception", "geometry_id", "interior_type"
    , "is_areal", "is_linear", "is_multi", "is_radian", "point_order"
    , "point_type", "ring_type", "tag", "topological_dimension" ]

exceptions = ["exception", "centroid_exception"];

iterators = ["box_iterator", "circular_iterator", "closing_iterator"
    , "ever_circling_iterator", "segment_range_iterator"]

models = ["point", "linestring", "box"
    , "polygon", "segment", "ring"
    , "multi_linestring", "multi_point", "multi_polygon", "referring_segment"]

ranges = ["box_range", "segment_range"];

strategies = ["distance::pythagoras", "distance::haversine"
    , "distance::cross_track", "distance::projected_point"
    , "within::winding", "within::franklin", "within::crossings_multiply"
    , "area::surveyor", "area::huiller"
    , "centroid::bashein_detmer", "centroid::average"
    , "convex_hull::graham_andrew"
    , "simplify::douglas_peucker"
    , "side::side_by_triangle", "side::side_by_cross_track"
    , "transform::inverse_transformer", "transform::map_transformer"
    , "transform::rotate_transformer", "transform::scale_transformer"
    , "transform::translate_transformer", "transform::ublas_transformer"
    ]
    
views = ["closeable_view", "reversible_view", "identity_view"]



for i in algorithms:
    group_to_quickbook(i)
    
for i in access_functions:
    group_to_quickbook(i)
    
for i in coordinate_systems:
    cs_to_quickbook(i)

for i in core:
    struct_to_quickbook(i)

for i in exceptions:
    class_to_quickbook(i)

for i in iterators:
    struct_to_quickbook(i)

for i in models:
    model_to_quickbook(i)
   
for i in ranges:
    class_to_quickbook(i)

for i in strategies:
    strategy_to_quickbook(i)

for i in views:
    struct_to_quickbook(i)
    

model_to_quickbook2("d2_1_1point__xy", "point_xy")

group_to_quickbook("arithmetic")
group_to_quickbook("register")
group_to_quickbook("enum")

os.system("bjam") 
