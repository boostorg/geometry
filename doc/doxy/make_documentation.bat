@echo off

doxygen

cd doxygen_output\html

for %%f in (*.html) do ..\..\doxygen_enhance.py %%f 

cd ..\..
