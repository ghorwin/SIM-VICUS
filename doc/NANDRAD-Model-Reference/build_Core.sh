#!/bin/bash

ADOC=NANDRAD_solver_core

echo '*** Generating html ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py html . &&
asciidoctor -a lang=en -r asciidoctor-mathematical $ADOC.adoc &&

echo '*** Generating pdf ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py pdf . &&
asciidoctor-pdf -a lang=en  -a pdf-theme=../adoc_utils/pdf-theme.yml -r asciidoctor-mathematical  -r ../adoc_utils/rouge_theme.rb -a pdf-fontsdir="../adoc_utils/fonts;GEM_FONTS_DIR" $ADOC.adoc &&

# restore html-type image files
python ../adoc_utils/scripts/adoc-image-prep.py html . &&

echo '*** Copying files to ../../docs directory' &&

mkdir ../../docs/$ADOC
mv $ADOC.html ../../docs/$ADOC/index.html &&
mv $ADOC.pdf ../../docs &&

cp -r ./images/* ../../docs/$ADOC/images

