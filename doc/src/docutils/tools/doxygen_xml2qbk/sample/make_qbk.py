#! /usr/bin/env python
# -*- coding: utf-8 -*-
# ===========================================================================
#  Copyright (c) 2011 Barend Gehrels, Geodan, Amsterdam, the Netherlands.
# 
#  Use, modification and distribution is subject to the Boost Software License,
#  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)9
# ============================================================================

import os, sys

cmd = "doxygen_xml2qbk"
cmd = cmd + " --xml xml/%s.xml"
cmd = cmd + " --start_include sample/"
cmd = cmd + " > %s.qbk"


os.system("doxygen fruit.dox")
os.system(cmd % ("group__eat", "eat"))
os.system(cmd % ("classfruit_1_1apple", "apple"))

os.system("bjam") 
