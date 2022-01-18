#!/bin/bash

ADOC=NANDRAD-FMU-Generator

echo '*** Generating html ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py html . &&
asciidoctor -a lang=de -r asciidoctor-mathematical $ADOC.adoc &&

echo &&
echo '*** Generating pdf ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py pdf . &&
asciidoctor-pdf -a lang=de  -a pdf-theme=../adoc_utils/pdf-theme.yml -r asciidoctor-mathematical  -r ../adoc_utils/rouge_theme.rb -a pdf-fontsdir="../adoc_utils/fonts;GEM_FONTS_DIR" $ADOC.adoc &&

# restore html-type image files
echo &&
echo '*** Restoring html ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py html . &&

echo &&
echo '*** Copying files to ../../docs directory' &&

if [ ! -d "../../docs/$ADOC" ]; then
	mkdir ../../docs/$ADOC
fi &&
mv $ADOC.html ../../docs/$ADOC/index.html &&
#mv $ADOC.pdf ../../docs &&

echo 'Copying images to ../../docs/'$ADOC'/images' &&
cp -r ./images/*.png ../../docs/$ADOC/images &&
cp -r ./images/*.svg ../../docs/$ADOC/images

