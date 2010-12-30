===========================================================================
 Copyright (c) 1995-2010 Barend Gehrels, Geodan, Amsterdam, the Netherlands.
 Copyright (c) 2008-2010 Bruno Lalande, Paris, France.
 Copyright (c) 2009-2010 Mateusz Loskot (mateusz@loskot.net), London, UK

 Use, modification and distribution is subject to the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 http://www.boost.org/LICENSE_1_0.txt)
============================================================================

To generate the documentation, simulate as if it is already in Boost.Trunk, 
by adding boostbook.dtd and html/callouts/etc,

So folder structure should be like

.../boost/sandbox/geometry/boost [normal]
.../boost/sandbox/geometry/libs [normal]
.../boost/sandbox/geometry/other [normal]

.../boost/sandbox/geometry/libs/doc/html/images/*  [copy files from boost trunk]
.../boost/sandbox/geometry/libs/doc/html/images/callouts/* [copy files from boost trunk]
.../boost/sandbox/geometry/tools/boostbook/dtd/boostbook.dtd [copy file from boost trunk]


Procedure to create documentation using Doxygen, QuickBook and doxygen_xml2qbk

1) install Doxygen
2) install QuickBook using http://www.boost.org/doc/libs/1_44_0/doc/html/quickbook/install.html#quickbook.install.linux
3) compile doxygen_xml2qbk, in .../boost/sandbox/geometry/other/programs/doxygen_xml2qbk
4) put binary somewhere, e.g. in /usr/local/bin/doxygen_xml2qbk
5) execute python file "make_qbk.py"

- 
