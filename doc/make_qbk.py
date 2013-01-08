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
cmd = cmd + " --start_include ."
cmd = cmd + " --document_id_path %s"
cmd = cmd + " > generated/%s.qbk"

os.system("doxygen Doxyfile")
os.system(cmd % ("classboost_1_1geometry_1_1index_1_1rtree", "geometry_index.r_tree.index.boost_geometry_index_rtree", "rtree"))

os.system("b2") 
