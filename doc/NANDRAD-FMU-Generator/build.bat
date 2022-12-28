@echo off

set ADOC=NANDRAD-FMU-Generator.adoc

echo *** Generating html ***
python ..\adoc_utils\scripts\adoc-image-prep.py html .
asciidoctor -a lang=de -a webfonts!  %ADOC%.adoc
