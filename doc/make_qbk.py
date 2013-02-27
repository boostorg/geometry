#! /usr/bin/env python
# -*- coding: utf-8 -*-
# ===========================================================================
#  Copyright (c) 2011-2012 Barend Gehrels, Amsterdam, the Netherlands.
# 
#  Use, modification and distribution is subject to the Boost Software License,
#  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)9
# ============================================================================

import os, sys

cmd = "doxygen_xml2qbk"
cmd = cmd + " --xml xml/%s.xml"
cmd = cmd + " --start_include boost/"
cmd = cmd + " --output_style alt"
cmd = cmd + " > generated/%s.qbk"

os.system("doxygen Doxyfile")
os.system(cmd % ("classboost_1_1geometry_1_1index_1_1rtree", "rtree"))
os.system(cmd % ("group__rtree__functions", "rtree_functions"))

os.system(cmd % ("structboost_1_1geometry_1_1index_1_1linear", "rtree_linear"))
os.system(cmd % ("structboost_1_1geometry_1_1index_1_1quadratic", "rtree_quadratic"))
os.system(cmd % ("structboost_1_1geometry_1_1index_1_1rstar", "rtree_rstar"))
os.system(cmd % ("classboost_1_1geometry_1_1index_1_1dynamic__linear", "rtree_dynamic_linear"))
os.system(cmd % ("classboost_1_1geometry_1_1index_1_1dynamic__quadratic", "rtree_dynamic_quadratic"))
os.system(cmd % ("classboost_1_1geometry_1_1index_1_1dynamic__rstar", "rtree_dynamic_rstar"))

os.system(cmd % ("structboost_1_1geometry_1_1index_1_1translator", "translator"))
#os.system(cmd % ("structboost_1_1geometry_1_1index_1_1translator_3_01std_1_1pair_3_01_indexable_00_01_second_01_4_01_4", "translator_pair"))
#os.system(cmd % ("structboost_1_1geometry_1_1index_1_1translator_3_01boost_1_1tuple_3_01_indexable_00_01_t1_00_01_1c55aa63046ee2c1171cfc2033fb0edd", "translator_tuple"))

os.system(cmd % ("group__predicates", "predicates"))
os.system(cmd % ("group__nearest__relations", "nearest_relations"))
os.system(cmd % ("group__adaptors", "adaptors"))
os.system(cmd % ("group__inserters", "inserters"))

os.system("b2") 
