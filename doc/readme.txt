===========================================================================
 Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
 Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
 Copyright (c) 2009-2011 Mateusz Loskot, London, UK

 Use, modification and distribution is subject to the Boost Software License,
 Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 http://www.boost.org/LICENSE_1_0.txt)
============================================================================

Procedure to create documentation using Doxygen, QuickBook and doxygen_xml2qbk

Note that doxygen_xml2qbk is a tool currently (slightly) specific to Boost.Geometry,
and that it translates from Doxygen-XML output to Quickbook (without xslt)

1) install Doxygen
2) install QuickBook using http://www.boost.org/doc/libs/1_45_0/doc/html/quickbook/install.html#quickbook.install.linux
3) compile doxygen_xml2qbk, in src/docutils/tools/doxygen_xml2qbk
4) put binary somewhere, e.g. in /usr/local/bin/doxygen_xml2qbk
5) execute python file "make_qbk.py" (calling doxygen, doxygen_xml2qbk, bjam)

Folders in this folder:
concept: manually written documentation QBK files, on concept
doxy: folders and files needed for doxygen input and output
html: contains generated HTML files
other: older documentation (subject to update or deletion)
ref: manually written documentation QBK files, included from .hpp files
reference: generated documentation QBK files (by doxygen_xml2qbk)
	[note: this book cannot be called "generated" or something like that,
	because it is used in the final URL and we want to have "reference" in it]
src: examples used in documentation and tools (doxygen_xml2qbk)

